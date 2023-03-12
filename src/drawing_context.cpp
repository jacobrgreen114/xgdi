

// Copyright (c) 2023 Jacob R. Green
// All Rights Reserved.

#include "muchcool/xgdi/drawing_context.hpp"

#include "src/shader/rect.vert.spv.hpp"
#include "src/shader/rect.frag.spv.hpp"

#include "src/shader/roundrect.vert.spv.hpp"
#include "src/shader/roundrect.frag.spv.hpp"

#include "src/shader/bitmap.vert.spv.hpp"
#include "src/shader/bitmap.frag.spv.hpp"

#include "src/shader/glyph.vert.spv.hpp"
#include "src/shader/glyph.frag.spv.hpp"

#include "src/shader/glyph_sdf.vert.spv.hpp"
#include "src/shader/glyph_sdf.frag.spv.hpp"

#define MAX_DESCRIPTOR_COUNT 4096

#define XGDI_DRAW_GLYPH_BOUNDING_BOX false

namespace muchcool::xgdi {

Shared<rndr::GraphicsPipeline> CreatePipeline(
    Shared<rndr::RenderSurface> renderSurface,
    Shared<rndr::PipelineLayout> layout,
    ArrayProxy<const uint8> vertexShaderData,
    ArrayProxy<const uint8> fragmentShaderData) {
  auto& context = renderSurface->context();
  auto& renderPass = renderSurface->GetRenderPass();

  auto vertexShader = rndr::Shader::FromData(context, vertexShaderData);

  auto fragmentShader = rndr::Shader::FromData(context, fragmentShaderData);

  return Shared{new rndr::GraphicsPipeline(
      context, renderPass, layout, vertexShader, fragmentShader, {}, {})};
}

// rndr::GraphicsPipeline* CreatePipeline(rndr::RenderSurface* renderSurface,
//                                        rndr::PipelineLayout* layout,
//                                        const char* vertexShaderPath,
//                                        const char* fragmentShaderPath) {
//   auto& graphicsContext = renderSurface->GetGraphicsContext();
//   auto& renderPass = renderSurface->GetRenderPass();
//
//   auto vertexShader =
//       rndr::Shader::LoadFromFile(graphicsContext, vertexShaderPath);
//
//   auto fragmentShader =
//       rndr::Shader::LoadFromFile(graphicsContext, fragmentShaderPath);
//
//   auto pipeline =
//       new rndr::GraphicsPipeline(graphicsContext, renderPass, layout,
//                                  vertexShader, fragmentShader, {}, {});
//
//   return pipeline;
// }

Shared<rndr::GraphicsPipeline> CreateRectanglePipeline(
    Shared<rndr::RenderSurface> renderSurface,
    Shared<rndr::PipelineLayout> layout) {
  return CreatePipeline(renderSurface, layout, rect_vert_spv, rect_frag_spv);
}

Shared<rndr::GraphicsPipeline> CreateRoundRectPipeline(
    Shared<rndr::RenderSurface> renderSurface,
    Shared<rndr::PipelineLayout> layout) {
  return CreatePipeline(renderSurface, layout, roundrect_vert_spv,
                        roundrect_frag_spv);
}

Shared<rndr::GraphicsPipeline> CreateGlyphPipeline(
    Shared<rndr::RenderSurface> renderSurface,
    Shared<rndr::PipelineLayout> layout) {
  return CreatePipeline(renderSurface, layout, glyph_sdf_vert_spv,
                        glyph_sdf_frag_spv);
}

Shared<rndr::GraphicsPipeline> CreateBitmapPipeline(
    Shared<rndr::RenderSurface> renderSurface,
    Shared<rndr::PipelineLayout> layout) {
  return CreatePipeline(renderSurface, layout, bitmap_vert_spv,
                        bitmap_frag_spv);
}

DrawingContext::DrawingContext(Shared<rndr::RenderSurface> surface_)
    : _renderSurface(std::move(surface_)), _renderInfo() {
  auto& context = _renderSurface->context();
  auto& device = context->device();
  auto& frameBuffers = _renderSurface->GetFrameBuffers();

  auto& viewport = _renderSurface->GetViewport();
  _renderInfo.Projection =
      glm::ortho(0.0f, viewport.width, 0.0f, viewport.height);

  _renderInfo.View = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                                 glm::vec3(0.0f, -1.0f, 0.0f));

  _renderInfoSetLayout = new rndr::DescriptorSetLayout(
      context,
      {rndr::DecriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1,
                                       vk::ShaderStageFlagBits::eAll)});

  _modelInfoSetLayout = new rndr::DescriptorSetLayout(
      context,
      {rndr::DecriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1,
                                       vk::ShaderStageFlagBits::eAll)});

  _glyphSetLayout = new rndr::DescriptorSetLayout(
      context, {rndr::DecriptorSetLayoutBinding(
                   0, vk::DescriptorType::eCombinedImageSampler, 1,
                   vk::ShaderStageFlagBits::eFragment)});

  _pipelineLayout = new rndr::PipelineLayout(
      context, {_renderInfoSetLayout, _modelInfoSetLayout});

  _sampledPipelineLayout = new rndr::PipelineLayout(
      context, {_renderInfoSetLayout, _modelInfoSetLayout, _glyphSetLayout});

  _rectanglePipeline = CreateRectanglePipeline(_renderSurface, _pipelineLayout);
  _roundRectPipeline = CreateRoundRectPipeline(_renderSurface, _pipelineLayout);
  _glyphPipeline = CreateGlyphPipeline(_renderSurface, _sampledPipelineLayout);
  _bitmapPipeline =
      CreateBitmapPipeline(_renderSurface, _sampledPipelineLayout);

  _commandPool = new rndr::CommandPool(context);

  _commandBuffers = _commandPool->AllocateBuffers(frameBuffers.size());
  _imageTransitionCommands = _commandPool->AllocateBuffer();

  _renderInfoUniformBuffer = new RenderUniformBuffer(context, _renderInfo);

  _descriptorPool = new rndr::DescriptorPool(
      context, MAX_DESCRIPTOR_COUNT,
      {rndr::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer,
                                MAX_DESCRIPTOR_COUNT),
       rndr::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler,
                                MAX_DESCRIPTOR_COUNT)});

  _renderDescriptor = _descriptorPool->allocate(*_renderInfoSetLayout);
  _renderDescriptor->update_uniform(0, *_renderInfoUniformBuffer);

  auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();
  _imageAvailableSemaphore = device.createSemaphore(semaphoreCreateInfo);
  _renderFinishedSemaphore = device.createSemaphore(semaphoreCreateInfo);
}

DrawingContext::~DrawingContext() {}

void DrawingContext::reset() {
  for (auto commandBuffer : _commandBuffers) commandBuffer.reset();

  _imageTransitionCommands->operator vk::CommandBuffer().reset();
  _rectUniforms.clear();
  _roundRectUniforms.clear();
}

void DrawingContext::start_recording() {
  auto& renderSurface = *_renderSurface;
  auto& frameBuffers = renderSurface.GetFrameBuffers();
  auto& renderPass = renderSurface.GetRenderPass();
  auto framebufferSize = renderSurface.GetCurrentExtent();

  auto commandxBeginInfo = vk::CommandBufferBeginInfo();
  _imageTransitionCommands->operator vk::CommandBuffer().begin(
      commandxBeginInfo);

  for (int i = 0; i < _commandBuffers.size(); ++i) {
    auto& commandBuffer = _commandBuffers[i];

    auto commandBeginInfo = vk::CommandBufferBeginInfo();
    commandBuffer.begin(commandBeginInfo);

    auto clearColor = vk::ClearValue(
        vk::ClearColorValue(std::array<float, 4>({0.0f, 0.0f, 0.0f, 0.0f})));

    auto renderPassBeginInfo = vk::RenderPassBeginInfo(
        *renderPass, frameBuffers[i], vk::Rect2D({0, 0}, framebufferSize), 1,
        &clearColor);
    commandBuffer.beginRenderPass(renderPassBeginInfo,
                                  vk::SubpassContents::eInline);

    commandBuffer.setViewport(0, renderSurface.GetViewport());
    commandBuffer.setScissor(0, renderSurface.GetScissor());

    auto renderDescriptorSets =
        std::array<vk::DescriptorSet, 1>{*_renderDescriptor};
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     *_pipelineLayout, 0, renderDescriptorSets,
                                     {});
  }
}

void DrawingContext::end_recording() {
  _imageTransitionCommands->operator vk::CommandBuffer().end();
  for (auto commandBuffer : _commandBuffers) {
    commandBuffer.endRenderPass();
    commandBuffer.end();
  }
}

void DrawingContext::submit() const {
  auto& renderSurface = *_renderSurface;
  auto& context = *renderSurface.context();
  auto& device = context.device();
  auto& queue = context.queue();

  auto& swapchain = renderSurface.GetSwapchain();

  auto& inFlightFence = renderSurface.GetInFlightFence();

  auto renderLock = renderSurface.LockRenderMutex();

  uint32_t nextImageIndex = 0;
  auto result = device.acquireNextImageKHR(
      swapchain, UINT64_MAX, _imageAvailableSemaphore, null, &nextImageIndex);
  if (result != vk::Result::eSuccess) {
    vk::throwResultException(result, "failed to acquire next frame.");
  }

  auto& commandBuffer = _commandBuffers[nextImageIndex];

  auto commandBuffers = std::array<vk::CommandBuffer, 2>{
      *_imageTransitionCommands, commandBuffer};

  auto submitSemaphore = std::array<vk::Semaphore, 1>{_imageAvailableSemaphore};

  auto submitStage = std::array<vk::PipelineStageFlags, 1>{
      vk::PipelineStageFlagBits::eColorAttachmentOutput};

  static_assert(submitSemaphore.size() == submitStage.size());
  auto submitInfo = vk::SubmitInfo(submitSemaphore, submitStage, commandBuffers,
                                   _renderFinishedSemaphore);

  result = queue.submit(1, &submitInfo, inFlightFence);
  if (result != vk::Result::eSuccess)
    vk::throwResultException(result,
                             "failed to submit command buffers to queue.");

  auto presentInfo =
      vk::PresentInfoKHR(_renderFinishedSemaphore, swapchain, nextImageIndex);

  result = queue.presentKHR(&presentInfo);
  if (result != vk::Result::eSuccess) {
    vk::throwResultException(result, "failed to present.");
  }

  result = device.waitForFences(inFlightFence, VK_TRUE, UINT64_MAX);
  if (result != vk::Result::eSuccess)
    vk::throwResultException(result, "failed to wait for fence.");
  device.resetFences(inFlightFence);
}

glm::mat4 model_projection(const Rect& rect, float rotation = 0.0f) {
  return glm::translate(glm::vec3(rect.Offset, 0.0f)) *
         glm::rotate(rotation, glm::vec3(0.0f, 0.0f, 1.0f)) *
         glm::scale(glm::vec3(rect.Size, 0.0f));
}

void DrawingContext::draw_rectangle(const Rect& rect, const Color& color) {
  auto& graphicsContext = _renderSurface->context();
  auto& viewport = _renderSurface->GetViewport();

  auto transformInfo =
      _RectangleInfo{.Model = model_projection(rect), .Color = color};

  auto uniformBuffer =
      Shared{new RectUniformBuffer(graphicsContext, transformInfo)};
  _rectUniforms.emplace_back(uniformBuffer);

  auto descriptorSet = _descriptorPool->allocate(*_modelInfoSetLayout);
  descriptorSet->update_uniform(0, *uniformBuffer);
  _transformDescriptorSets.emplace_back(descriptorSet);

  auto transformDescriptorSets =
      std::array<vk::DescriptorSet, 1>{*descriptorSet};

  for (auto commandBuffer : _commandBuffers) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                               *_rectanglePipeline);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     *_pipelineLayout, 1,
                                     transformDescriptorSets, {});

    commandBuffer.draw(6, 1, 0, 0);
  }
}

void DrawingContext::draw_line(const Point& start, const Point& end,
                               const Color& color, float thickness) {
  auto width = glm::length(end - start);
  auto rect = Rect{start, {width, thickness}};

  auto roundRectInfo = _RoundRectInfo{.Model = model_projection(rect),
                                      .FillColor = color,
                                      .ModelSize = rect.Size};

  draw_rectangle(roundRectInfo);
}

void DrawingContext::draw_rectangle(const Rect& rect, const Size& radius,
                                    const Color& fill, const Color& stroke,
                                    float strokeThickness) {
  auto roundRectInfo =
      _RoundRectInfo{.Model = model_projection(rect),
                     .FillColor = fill,
                     .StrokeColor = stroke,
                     .ModelSize = rect.Size,
                     .CornerRadius = radius,
                     .StrokeThickness = glm::vec1(strokeThickness)};

  draw_rectangle(roundRectInfo);
}

void DrawingContext::draw_rectangle(const _RoundRectInfo& roundRectInfo) {
  auto& graphicsContext = _renderSurface->context();
  auto& viewport = _renderSurface->GetViewport();

  auto uniformBuffer =
      Shared{new RoundRectUniformBuffer(graphicsContext, roundRectInfo)};
  _roundRectUniforms.emplace_back(uniformBuffer);

  auto descriptorSet = _descriptorPool->allocate(*_modelInfoSetLayout);
  descriptorSet->update_uniform(0, *uniformBuffer);
  _transformDescriptorSets.emplace_back(descriptorSet);

  auto transformDescriptorSets =
      std::array<vk::DescriptorSet, 1>{*descriptorSet};

  for (auto& commandBuffer : _commandBuffers) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                               *_roundRectPipeline);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     *_pipelineLayout, 1,
                                     transformDescriptorSets, {});

    commandBuffer.draw(6, 1, 0, 0);
  }
}

void DrawingContext::draw_custom(DrawCustomCallback callback) {
  for (auto& commandBuffer : _commandBuffers) callback(commandBuffer);
}

bool isControlChar(char16_t c) { return c <= 0x1F; }

void DrawingContext::draw_formatted_text(const Point& point,
                                         const FormattedText& text,
                                         const Color& color) {
  auto& font = text.font();

  auto p = glm::round(point);  // Keeps text pixel aligned

  for (auto c : text.text()) {
    if (!isControlChar(c)) {
      auto& glyph = font->glyph(c);
      if (glyph.texture()) draw_glyph(p, glyph, color);
      p.x += glyph.advance();
    } else {
      if (c == '\n') {
        p.x = point.x;  // TODO : Fix text pixel alignment on newline bug
        p.y += font->line_height();
      }
    }
  }
}

void DrawingContext::draw_glyph(const Point& point, const Glyph& glyph,
                                const Color& color) {
  auto& graphicsContext = _renderSurface->context();
  auto& viewport = _renderSurface->GetViewport();

  auto bitmap_scale = glyph.bitmap_size() / glyph.size();

  // todo : fix bitmap offset
  auto bitmap_offset =
      glyph.bitmap_baseline() * glyph.size() / glyph.bitmap_size();

  auto p = point;
  auto off = glyph.bearing() * glm::vec2{1.0f, -1.0f};

#if XGDI_DRAW_GLYPH_BOUNDING_BOX
  auto bgrect = Rect{.Offset = p + off, .Size = glyph.size()};
  draw_rectangle(bgrect, Color::Red);
#endif

  off.x += glyph.bitmap_baseline().x - off.x;
  off.y -= glyph.bitmap_baseline().y + off.y;
  auto rect = Rect{.Offset = p + off, .Size = glyph.size() * bitmap_scale};

  auto glyphInfo = _GlyphInfo{.Model = model_projection(rect), .Color = color};

  auto uniformBuffer =
      Shared{new GlyphUniformBuffer(graphicsContext, glyphInfo)};
  _glyphUniforms.emplace_back(uniformBuffer);

  auto descriptorSet = _descriptorPool->allocate(*_modelInfoSetLayout);
  descriptorSet->update_uniform(0, *uniformBuffer);
  _transformDescriptorSets.emplace_back(descriptorSet);

  auto samplerSet = _descriptorPool->allocate(*_glyphSetLayout);
  samplerSet->update_sampler(0, *glyph.texture());
  _glyphDescriptorSets.emplace_back(samplerSet);

  auto descriptorSets =
      std::array<vk::DescriptorSet, 2>{*descriptorSet, *samplerSet};

  for (auto& commandBuffer : _commandBuffers) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                               *_glyphPipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     *_sampledPipelineLayout, 1, descriptorSets,
                                     {});

    commandBuffer.draw(6, 1, 0, 0);
  }
}

void DrawingContext::draw_bitmap(const Rect& rect, const Bitmap& bitmap) {
  auto& graphicsContext = _renderSurface->context();

  auto rectInfo =
      _RectangleInfo{.Model = model_projection(rect), .Color = Color::White};

  auto uniformBuffer = Shared{new RectUniformBuffer(graphicsContext, rectInfo)};
  _rectUniforms.emplace_back(uniformBuffer);

  auto descriptorSet = _descriptorPool->allocate(*_modelInfoSetLayout);
  descriptorSet->update_uniform(0, *uniformBuffer);
  _transformDescriptorSets.emplace_back(descriptorSet);

  auto samplerSet = _descriptorPool->allocate(*_glyphSetLayout);
  samplerSet->update_sampler(0, *bitmap.GetTexture());
  _glyphDescriptorSets.emplace_back(samplerSet);

  auto descriptorSets =
      std::array<vk::DescriptorSet, 2>{*descriptorSet, *samplerSet};

  for (auto& commandBuffer : _commandBuffers) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                               *_bitmapPipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     *_sampledPipelineLayout, 1, descriptorSets,
                                     {});

    commandBuffer.draw(6, 1, 0, 0);
  }
}

}  // namespace muchcool::xgdi
