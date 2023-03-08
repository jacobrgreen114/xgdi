
// Copyright (c) 2022. Jacob R. Green
// All Rights Reserved.

#pragma once

#include "Datatypes.hpp"

namespace xgdi {

class Bitmap : public rndr::GraphicsObject {

  Pointer<rndr::Texture> _texture;

  uint32 _width;
  uint32 _height;

public:
  Bitmap(rndr::GraphicsContext* context, const char* path);

  inline auto GetWidth() const { return _width; }
  inline auto GetHeight() const { return _height; }

  inline auto& GetTexture() const { return _texture; }
};

} // namespace xgdi
