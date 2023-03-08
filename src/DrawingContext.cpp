

// Copyright (c) 2022-2023 Jacob R. Green
// All Rights Reserved.

#include "muchcool/xgdi/DrawingContext.hpp"

#define MAX_DESCRIPTOR_COUNT 1024

const char *const rectVertShaderPath = "Shaders/RectVertShader.spv";
const char *const rectFragShaderPath = "Shaders/RectFragShader.spv";

const char *const roundRectVertShaderPath = "Shaders/RoundRectVertShader.spv";
const char *const roundRectFragShaderPath = "Shaders/RoundRectFragShader.spv";

const char *const glyphVertShaderPath = "Shaders/GlyphVertShader.spv";
const char *const glyphFragShaderPath = "Shaders/GlyphFragShader.spv";

const char *const bitmapVertShaderPath = "Shaders/BitmapVertShader.spv";
const char *const bitmapFragShaderPath = "Shaders/BitmapFragShader.spv";

namespace xgdi {

rndr::GraphicsPipeline *CreatePipeline(rndr::RenderSurface *renderSurface,
                                       rndr::PipelineLayout *layout,
                                       const char *vertexShaderPath,
                                       const char *fragmentShaderPath) {
  auto &graphicsContext = renderSurface->GetGraphicsContext();
  auto &renderPass = renderSurface->GetRenderPass();

  auto vertexShader =
      rndr::Shader::LoadFromFile(graphicsContext, vertexShaderPath);

  auto fragmentShader =
      rndr::Shader::LoadFromFile(graphicsContext, fragmentShaderPath);

  auto pipeline =
      new rndr::GraphicsPipeline(graphicsContext, renderPass, layout,
                                 vertexShader, fragmentShader, {}, {});

  return pipeline;
}

rndr::GraphicsPipeline *
CreateRectanglePipeline(rndr::RenderSurface *renderSurface,
                        rndr::PipelineLayout *layout) {
  return CreatePipeline(renderSurface, layout, rectVertShaderPath,
                        rectFragShaderPath);
}

rndr::GraphicsPipeline *
CreateRoundRectPipeline(rndr::RenderSurface *renderSurface,
                        rndr::PipelineLayout *layout) {
  return CreatePipeline(renderSurface, layout, roundRectVertShaderPath,
                        roundRectFragShaderPath);
}

rndr::GraphicsPipeline *CreateGlyphPipeline(rndr::RenderSurface *renderSurface,
                                            rndr::PipelineLayout *layout) {
  return CreatePipeline(renderSurface, layout, glyphVertShaderPath,
                        glyphFragShaderPath);
}

rndr::GraphicsPipeline *CreateBitmapPipeline(rndr::RenderSurface *renderSurface,
                                             rndr::PipelineLayout *layout) {
  return CreatePipeline(renderSurface, layout, bitmapVertShaderPath,
                        bitmapFragShaderPath);
}

DrawingContext::DrawingContext(rndr::RenderSurface *surface)
    : _renderSurface(surface), _renderInfo() {
  auto &graphicsContext = _renderSurface->GetGraphicsContext();
  auto &device = graphicsContext->GetDevice();
  auto &frameBuffers = _renderSurface->GetFrameBuffers();

  auto &viewport = _renderSurface->GetViewport();
  _renderInfo.Projection =
      glm::ortho(0.0f, viewport.width, 0.0f, viewport.height);

  _renderInfo.View = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, -1.0f),
                                 glm::vec3(0.0f, -1.0f, 0.0f));

  _renderInfoSetLayout = new rndr::DescriptorSetLayout(
      graphicsContext,
      {rndr::DecriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1,
                                       vk::ShaderStageFlagBits::eAll)});

  _modelInfoSetLayout = new rndr::DescriptorSetLayout(
      graphicsContext,
      {rndr::DecriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1,
                                       vk::ShaderStageFlagBits::eAll)});

  _glyphSetLayout = new rndr::DescriptorSetLayout(
      graphicsContext, {rndr::DecriptorSetLayoutBinding(
                           0, vk::DescriptorType::eCombinedImageSampler, 1,
                           vk::ShaderStageFlagBits::eFragment)});

  _pipelineLayout = new rndr::PipelineLayout(
      graphicsContext, {_renderInfoSetLayout, _modelInfoSetLayout});

  _sampledPipelineLayout = new rndr::PipelineLayout(
      graphicsContext,
      {_renderInfoSetLayout, _modelInfoSetLayout, _glyphSetLayout});

  _rectanglePipeline = CreateRectanglePipeline(_renderSurface, _pipelineLayout);
  _roundRectPipeline = CreateRoundRectPipeline(_renderSurface, _pipelineLayout);
  _glyphPipeline = CreateGlyphPipeline(_renderSurface, _sampledPipelineLayout);
  _bitmapPipeline =
      CreateBitmapPipeline(_renderSurface, _sampledPipelineLayout);

  _commandPool = new rndr::CommandPool(graphicsContext);

  _commandBuffers = _commandPool->AllocateBuffers(frameBuffers.size());
  _imageTransitionCommands = _commandPool->AllocateBuffer();

  _renderInfoUniformBuffer =
      new RenderUniformBuffer(graphicsContext, _renderInfo);

  _descriptorPool = new rndr::DescriptorPool(
      graphicsContext, MAX_DESCRIPTOR_COUNT,
      {rndr::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer,
                                MAX_DESCRIPTOR_COUNT),
       rndr::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler,
                                MAX_DESCRIPTOR_COUNT)});

  _renderDescriptor = _descriptorPool->Allocate(_renderInfoSetLayout);
  _renderDescriptor->UpdateUniform(0, _renderInfoUniformBuffer);

  auto semaphoreCreateInfo = vk::SemaphoreCreateInfo();
  _imageAvailableSemaphore = device.createSemaphore(semaphoreCreateInfo);
  _renderFinishedSemaphore = device.createSemaphore(semaphoreCreateInfo);
}

DrawingContext::~DrawingContext() {}

void DrawingContext::Reset() {
  for (auto commandBuffer : _commandBuffers)
    commandBuffer.reset();

  _imageTransitionCommands->operator vk::CommandBuffer().reset();
  _rectUniforms.clear();
  _roundRectUniforms.clear();
}

void DrawingContext::StartRecording() {
  auto &renderSurface = *_renderSurface;
  auto &frameBuffers = renderSurface.GetFrameBuffers();
  auto &renderPass = renderSurface.GetRenderPass();
  auto framebufferSize = renderSurface.GetCurrentExtent();

  auto commandxBeginInfo = vk::CommandBufferBeginInfo();
  _imageTransitionCommands->operator vk::CommandBuffer().begin(
      commandxBeginInfo);

  for (int i = 0; i < _commandBuffers.size(); ++i) {
    auto &commandBuffer = _commandBuffers[i];

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

void DrawingContext::EndRecording() {
  _imageTransitionCommands->operator vk::CommandBuffer().end();
  for (auto commandBuffer : _commandBuffers) {
    commandBuffer.endRenderPass();
    commandBuffer.end();
  }
}

void DrawingContext::Submit() const {
  auto &renderSurface = *_renderSurface;
  auto &graphicsContext = *renderSurface.GetGraphicsContext();
  auto &device = graphicsContext.GetDevice();
  auto &queue = graphicsContext.GetQueue();

  auto &swapchain = renderSurface.GetSwapchain();

  auto &inFlightFence = renderSurface.GetInFlightFence();

  auto renderLock = renderSurface.LockRenderMutex();

  uint32_t nextImageIndex = 0;
  auto result = device.acquireNextImageKHR(
      swapchain, UINT64_MAX, _imageAvailableSemaphore, null, &nextImageIndex);
  if (result != vk::Result::eSuccess) {
    vk::throwResultException(result, "failed to acquire next frame.");
  }

  auto &commandBuffer = _commandBuffers[nextImageIndex];

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

glm::mat4 ModelProjection(const Rect &rect, float rotation = 0.0f) {
  return glm::translate(glm::vec3(rect.Offset, 0.0f)) *
         glm::rotate(rotation, glm::vec3(0.0f, 0.0f, 1.0f)) *
         glm::scale(glm::vec3(rect.Size, 0.0f));
}

void DrawingContext::DrawRectangle(const Rect &rect, const Color &color) {
  auto &graphicsContext = _renderSurface->GetGraphicsContext();
  auto &viewport = _renderSurface->GetViewport();

  auto transformInfo =
      _RectangleInfo{.Model = ModelProjection(rect), .Color = color};

  auto uniformBuffer = new RectUniformBuffer(graphicsContext, transformInfo);
  _rectUniforms.emplace_back(uniformBuffer);

  auto descriptorSet = _descriptorPool->Allocate(_modelInfoSetLayout);
  descriptorSet->UpdateUniform(0, uniformBuffer);
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

void DrawingContext::DrawLine(const Point &start, const Point &end,
                              const Color &color, float thickness) {
  auto width = glm::length(end - start);
  auto rect = Rect{start, {width, thickness}};

  auto roundRectInfo = _RoundRectInfo{.Model = ModelProjection(rect),
                                      .FillColor = color,
                                      .ModelSize = rect.Size};

  DrawRectangle(roundRectInfo);
}

void DrawingContext::DrawRectangle(const Rect &rect, const Size &radius,
                                   const Color &fill, const Color &stroke,
                                   float strokeThickness) {
  auto roundRectInfo =
      _RoundRectInfo{.Model = ModelProjection(rect),
                     .FillColor = fill,
                     .StrokeColor = stroke,
                     .ModelSize = rect.Size,
                     .CornerRadius = radius,
                     .StrokeThickness = glm::vec1(strokeThickness)};

  DrawRectangle(roundRectInfo);
}

void DrawingContext::DrawRectangle(const _RoundRectInfo &roundRectInfo) {
  auto &graphicsContext = _renderSurface->GetGraphicsContext();
  auto &viewport = _renderSurface->GetViewport();

  auto uniformBuffer =
      new RoundRectUniformBuffer(graphicsContext, roundRectInfo);
  _roundRectUniforms.emplace_back(uniformBuffer);

  auto descriptorSet = _descriptorPool->Allocate(_modelInfoSetLayout);
  descriptorSet->UpdateUniform(0, uniformBuffer);
  _transformDescriptorSets.emplace_back(descriptorSet);

  auto transformDescriptorSets =
      std::array<vk::DescriptorSet, 1>{*descriptorSet};

  for (auto &commandBuffer : _commandBuffers) {
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                               *_roundRectPipeline);

    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     *_pipelineLayout, 1,
                                     transformDescriptorSets, {});

    commandBuffer.draw(6, 1, 0, 0);
  }
}

void DrawingContext::DrawCustom(DrawCustomCallback callback) {
  for (auto &commandBuffer : _commandBuffers)
    callback(commandBuffer);
}

bool isControlChar(char16_t c) { return c <= 0x1F; }

void DrawingContext::DrawFormattedText(const Point &point, FormattedText *text,
                                       const Color &color) {
  auto &font = text->GetFont();

  auto p = glm::round(point); // Keeps text pixel aligned

  for (auto c : text->GetText()) {
    if (!isControlChar(c)) {
      auto &glyph = font->GetGlyph(c);
      if (glyph.GetTexture())
        DrawGlyph(p, glyph, color);
      p.x += glyph.GetAdvance();
    } else {
      if (c == '\n') {
        p.x = point.x; // TODO : Fix text pixel alignment on newline bug
        p.y += font->GetLineHeight();
      }
    }
  }
}

void DrawingContext::DrawGlyph(const Point &point, const Glyph &glyph,
                               const Color &color) {
  auto &graphicsContext = _renderSurface->GetGraphicsContext();
  auto &viewport = _renderSurface->GetViewport();

  auto p = glm::round(point);

  auto rect =
      Rect{.Offset = Point{p.x + glyph.bearing.x, p.y - glyph.bearing.y},
           .Size = {glyph.size.x, glyph.size.y}};

  auto glyphInfo = _GlyphInfo{.Model = ModelProjection(rect), .Color = color};

  auto uniformBuffer = new GlyphUniformBuffer(graphicsContext, glyphInfo);
  _glyphUniforms.emplace_back(uniformBuffer);

  auto descriptorSet = _descriptorPool->Allocate(_modelInfoSetLayout);
  descriptorSet->UpdateUniform(0, uniformBuffer);
  _transformDescriptorSets.emplace_back(descriptorSet);

  auto samplerSet = _descriptorPool->Allocate(_glyphSetLayout);
  samplerSet->UpdateSampler(0, glyph.GetTexture());
  _glyphDescriptorSets.emplace_back(samplerSet);

  auto descriptorSets =
      std::array<vk::DescriptorSet, 2>{*descriptorSet, *samplerSet};

  for (auto &commandBuffer : _commandBuffers) {

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                               *_glyphPipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     *_sampledPipelineLayout, 1, descriptorSets,
                                     {});

    commandBuffer.draw(6, 1, 0, 0);
  }
}

void DrawingContext::DrawBitmap(const Rect &rect, const Bitmap *bitmap) {
  auto &graphicsContext = _renderSurface->GetGraphicsContext();

  auto rectInfo =
      _RectangleInfo{.Model = ModelProjection(rect), .Color = Color::White};

  auto uniformBuffer = new RectUniformBuffer(graphicsContext, rectInfo);
  _rectUniforms.emplace_back(uniformBuffer);

  auto descriptorSet = _descriptorPool->Allocate(_modelInfoSetLayout);
  descriptorSet->UpdateUniform(0, uniformBuffer);
  _transformDescriptorSets.emplace_back(descriptorSet);

  auto samplerSet = _descriptorPool->Allocate(_glyphSetLayout);
  samplerSet->UpdateSampler(0, bitmap->GetTexture());
  _glyphDescriptorSets.emplace_back(samplerSet);

  auto descriptorSets =
      std::array<vk::DescriptorSet, 2>{*descriptorSet, *samplerSet};

  for (auto &commandBuffer : _commandBuffers) {

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics,
                               *_bitmapPipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     *_sampledPipelineLayout, 1, descriptorSets,
                                     {});

    commandBuffer.draw(6, 1, 0, 0);
  }
}

} // namespace xgdi
