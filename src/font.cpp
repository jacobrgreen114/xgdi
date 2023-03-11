// Copyright (c) 2023 Jacob R. Green
// All Rights Reserved.

#include "muchcool/xgdi/font.hpp"

#include <exception>

namespace muchcool::xgdi {

Glyph::Glyph(Shared<rndr::GraphicsContext> context_, ft::Glyph glyph)
    : rndr::GraphicsObject(std::move(context_)),
      metrics(glyph.metrics()),
      size(metrics.width / 64.0f, metrics.height / 64.0f),
      bearing(metrics.horiBearingX / 64.0f, metrics.horiBearingY / 64.0f),
      advance_(metrics.horiAdvance / 64.0f) {
  const auto& bitmap = glyph.bitmap();

  if (bitmap.width > 0 && bitmap.rows > 0) {
    _texture = new rndr::Texture(
        context(), bitmap.width, bitmap.rows, vk::Format::eR8Unorm,
        bitmap.width * bitmap.rows, bitmap.buffer, vk::Filter::eNearest,
        vk::SamplerAddressMode::eClampToBorder);
  }
}

auto& get_freetype() {
  static auto freetype = ft::Library{};
  return freetype;
}

Font::Font(Shared<rndr::GraphicsContext> context_, fs::path fontPath,
           float size)
    : rndr::GraphicsObject(std::move(context_)),
      _face(get_freetype().new_face(fontPath)),
      _font(fontPath),
      _size(size) {
  _face.set_char_size(_size);
}

const Glyph& Font::glyph(CharCode code) {
  if (auto it = _characterCache.find(code); it != _characterCache.end()) {
    return it->second;
  }

  const auto glyphIndex = _face.get_char_index(code);

  auto glyph = _face.load_glyph(glyphIndex);
  // todo : switch font rendering to sdf
  glyph.render(FT_Render_Mode::FT_RENDER_MODE_NORMAL);

  auto pair = _characterCache.try_emplace(code, context(), glyph);
  if (!pair.second) {
    throw std::runtime_error{"Failed to cache glyph."};
  }

  return pair.first->second;
}

Font::~Font() {}

std::vector<Shared<Font>> fontCache;

Shared<Font> Font::Load(Shared<rndr::GraphicsContext> context,
                        fs::path fontPath, float size) {
  for (auto& font : fontCache) {
    if (font->context() == context && font->_font == fontPath &&
        font->_size == size)
      return font;
  }

  auto font = Shared{new Font(std::move(context), fontPath, size)};
  fontCache.emplace_back(font);

  return font;
}

}  // namespace muchcool::xgdi