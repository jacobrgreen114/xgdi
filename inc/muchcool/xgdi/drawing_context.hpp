
// Copyright (c) 2023 Jacob R. Green
// All Rights Reserved.

#pragma once

#include "bitmap.hpp"
#include "datatypes.hpp"
#include "formatted_text.hpp"
#include "muchcool/rndr.hpp"

namespace muchcool::xgdi {

struct _RenderInfo {
  glm::mat4 Projection;
  glm::mat4 View;
};

struct _RectangleInfo {
  glm::mat4 Model;
  glm::vec4 Color;
};

struct _RoundRectInfo {
  glm::mat4 Model;
  glm::vec4 FillColor;
  glm::vec4 StrokeColor;
  glm::vec2 ModelSize;
  glm::vec2 CornerRadius;
  glm::vec1 StrokeThickness;
};

struct _GlyphInfo {
  glm::mat4 Model;
  glm::vec4 Color;
};

using DrawCustomCallback = void (*)(vk::CommandBuffer& commandBuffer);

class DrawingContext : public virtual Object {
  using RenderUniformBuffer = rndr::UniformBuffer<_RenderInfo>;
  using RectUniformBuffer = rndr::UniformBuffer<_RectangleInfo>;
  using RoundRectUniformBuffer = rndr::UniformBuffer<_RoundRectInfo>;
  using GlyphUniformBuffer = rndr::UniformBuffer<_GlyphInfo>;

  Shared<rndr::RenderSurface> _renderSurface;

  Shared<rndr::DescriptorSetLayout> _renderInfoSetLayout;
  Shared<rndr::DescriptorSetLayout> _modelInfoSetLayout;
  Shared<rndr::DescriptorSetLayout> _glyphSetLayout;
  Shared<rndr::PipelineLayout> _pipelineLayout;
  Shared<rndr::PipelineLayout> _sampledPipelineLayout;

  Shared<rndr::GraphicsPipeline> _rectanglePipeline;
  Shared<rndr::GraphicsPipeline> _roundRectPipeline;
  Shared<rndr::GraphicsPipeline> _glyphPipeline;
  Shared<rndr::GraphicsPipeline> _bitmapPipeline;

  Shared<rndr::CommandPool> _commandPool;
  std::vector<vk::CommandBuffer> _commandBuffers;
  Shared<rndr::CommandBuffer> _imageTransitionCommands;

  _RenderInfo _renderInfo;
  Shared<RenderUniformBuffer> _renderInfoUniformBuffer;

  std::vector<Shared<RectUniformBuffer>> _rectUniforms;
  std::vector<Shared<RoundRectUniformBuffer>> _roundRectUniforms;
  std::vector<Shared<GlyphUniformBuffer>> _glyphUniforms;

  Shared<rndr::DescriptorPool> _descriptorPool;
  Shared<rndr::DescriptorSet> _renderDescriptor;

  std::vector<Shared<rndr::DescriptorSet>> _transformDescriptorSets;
  std::vector<Shared<rndr::DescriptorSet>> _glyphDescriptorSets;

  vk::Semaphore _imageAvailableSemaphore;
  vk::Semaphore _renderFinishedSemaphore;

 public:
  DrawingContext(Shared<rndr::RenderSurface> surface_);
  DrawingContext(DrawingContext&&) = delete;
  DrawingContext(const DrawingContext&) = delete;
  ~DrawingContext() override;

  void reset();

  void start_recording();
  void end_recording();

  void submit() const;

  void draw_rectangle(const Rect& rect, const Color& color);

  void draw_line(const Point& start, const Point& end, const Color& color,
                 float thickness = 1.0f);

  void draw_rectangle(const Rect& rect, const Size& radius, const Color& fill,
                      const Color& stroke = {}, float strokeThickness = 0);

  void draw_formatted_text(const Point& point, const FormattedText& text,
                           const Color& color = Color::Black);

  void draw_bitmap(const Rect& rect, const Bitmap& bitmap);

  void draw_custom(DrawCustomCallback callback);

 private:
  void draw_rectangle(const _RoundRectInfo& roundRectInfo);
  void draw_glyph(const Point& point, const Glyph& glyph, const Color& color);
};

}  // namespace muchcool::xgdi