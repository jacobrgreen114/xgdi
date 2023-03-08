
// Copyright (c) 2022-2023 Jacob R. Green
// All Rights Reserved.

#pragma once

#include "Bitmap.hpp"
#include "Datatypes.hpp"
#include "FormattedText.hpp"
#include "muchcool/rndr.hpp"

namespace xgdi {

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

using DrawCustomCallback = void (*)(vk::CommandBuffer &commandBuffer);

class DrawingContext : public virtual Object {
  using RenderUniformBuffer = rndr::UniformBuffer<_RenderInfo>;
  using RectUniformBuffer = rndr::UniformBuffer<_RectangleInfo>;
  using RoundRectUniformBuffer = rndr::UniformBuffer<_RoundRectInfo>;
  using GlyphUniformBuffer = rndr::UniformBuffer<_GlyphInfo>;

  Pointer<rndr::RenderSurface> _renderSurface;

  Pointer<rndr::DescriptorSetLayout> _renderInfoSetLayout;
  Pointer<rndr::DescriptorSetLayout> _modelInfoSetLayout;
  Pointer<rndr::DescriptorSetLayout> _glyphSetLayout;
  Pointer<rndr::PipelineLayout> _pipelineLayout;
  Pointer<rndr::PipelineLayout> _sampledPipelineLayout;

  Pointer<rndr::GraphicsPipeline> _rectanglePipeline;
  Pointer<rndr::GraphicsPipeline> _roundRectPipeline;
  Pointer<rndr::GraphicsPipeline> _glyphPipeline;
  Pointer<rndr::GraphicsPipeline> _bitmapPipeline;

  Pointer<rndr::CommandPool> _commandPool;
  std::vector<vk::CommandBuffer> _commandBuffers;
  Pointer<rndr::CommandBuffer> _imageTransitionCommands;

  _RenderInfo _renderInfo;
  Pointer<RenderUniformBuffer> _renderInfoUniformBuffer;

  std::vector<Pointer<RectUniformBuffer>> _rectUniforms;
  std::vector<Pointer<RoundRectUniformBuffer>> _roundRectUniforms;
  std::vector<Pointer<GlyphUniformBuffer>> _glyphUniforms;

  Pointer<rndr::DescriptorPool> _descriptorPool;
  Pointer<rndr::DescriptorSet> _renderDescriptor;

  std::vector<Pointer<rndr::DescriptorSet>> _transformDescriptorSets;
  std::vector<Pointer<rndr::DescriptorSet>> _glyphDescriptorSets;

  vk::Semaphore _imageAvailableSemaphore;
  vk::Semaphore _renderFinishedSemaphore;

public:
  DrawingContext(rndr::RenderSurface *surface);
  DrawingContext(DrawingContext &&) = delete;
  DrawingContext(const DrawingContext &) = delete;
  ~DrawingContext() override;

  void Reset();

  void StartRecording();
  void EndRecording();

  void Submit() const;

  void DrawRectangle(const Rect &rect, const Color &color);

  void DrawLine(const Point &start, const Point &end, const Color &color,
                float thickness = 1.0f);

  void DrawRectangle(const Rect &rect, const Size &radius, const Color &fill,
                     const Color &stroke = {}, float strokeThickness = 0);

  void DrawFormattedText(const Point &point, FormattedText *text,
                         const Color &color = Color::Black);

  void DrawBitmap(const Rect &rect, const Bitmap *bitmap);

  void DrawCustom(DrawCustomCallback callback);

private:
  void DrawRectangle(const _RoundRectInfo &roundRectInfo);
  void DrawGlyph(const Point &point, const Glyph &glyph, const Color &color);
};

} // namespace xgdi