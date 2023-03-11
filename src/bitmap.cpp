
// Copyright (c) 2023 Jacob R. Green
// All Rights Reserved.

#include "muchcool/xgdi/bitmap.hpp"

#include "IL/il.h"
#include "IL/ilu.h"

namespace muchcool::xgdi {

bool ilInitialized = false;

Bitmap::Bitmap(Shared<rndr::GraphicsContext> context_, const char* filePath)
    : GraphicsObject(std::move(context_)) {
  if (!ilInitialized) {
    ilInit();
    iluInit();
    ilInitialized = true;
  }

  ILuint image;
  ilGenImages(1, &image);
  ilBindImage(image);

  auto imageLoaded = ilLoadImage(filePath);
  if (imageLoaded == IL_FALSE) {
    const char* msg = iluErrorString(ilGetError());
    throw std::exception(msg);
  }

  imageLoaded = ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
  if (imageLoaded == IL_FALSE) throw std::exception();

  ILinfo info;
  iluGetImageInfo(&info);

  _width = info.Width;
  _height = info.Height;

  auto pixelData = std::vector<uint32>(_width * _height);
  auto copyResult = ilCopyPixels(0, 0, 0, _width, _height, 1, IL_RGBA,
                                 IL_UNSIGNED_BYTE, pixelData.data());

  _texture = Shared{
      new rndr::Texture(context(), _width, _height, vk::Format::eR8G8B8A8Unorm,
                        sizeof(uint32) * _width * _height, ilGetData())};

  ilDeleteImages(1, &image);
}

}  // namespace muchcool::xgdi