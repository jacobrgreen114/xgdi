
// Copyright (c) 2023 Jacob R. Green
// All Rights Reserved.

#pragma once

#include "datatypes.hpp"

namespace muchcool::xgdi {

class Bitmap : public rndr::GraphicsObject {
  Shared<rndr::Texture> _texture;

  uint32 _width;
  uint32 _height;

 public:
  Bitmap(Shared<rndr::GraphicsContext> context, const char* path);

  auto GetWidth() const { return _width; }
  auto GetHeight() const { return _height; }

  auto& GetTexture() const { return _texture; }
};

}  // namespace muchcool::xgdi
