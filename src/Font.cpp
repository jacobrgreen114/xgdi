// Copyright (c) 2022. Jacob R. Green
// All Rights Reserved.

#include "muchcool/xgdi/Font.hpp"

#include <exception>

namespace xgdi {

Glyph::Glyph(rndr::GraphicsContext* context, FT_GlyphSlot glyph)
    : rndr::GraphicsObject(context), metrics(glyph->metrics),
      size(metrics.width / 64.0f, metrics.height / 64.0f),
      bearing(metrics.horiBearingX / 64.0f, metrics.horiBearingY / 64.0f),
      advance(metrics.horiAdvance / 64.0f) {

  if (glyph->bitmap.width > 0 && glyph->bitmap.rows > 0)
    _texture = new rndr::Texture(
        context, glyph->bitmap.width, glyph->bitmap.rows, vk::Format::eR8Unorm,
        glyph->bitmap.width * glyph->bitmap.rows, glyph->bitmap.buffer,
        vk::Filter::eNearest, vk::SamplerAddressMode::eClampToBorder);
}

#define FONT_DPI 96

FT_Library ftLibrary;

Font::Font(rndr::GraphicsContext* context, const char* fontPath, float size)
    : rndr::GraphicsObject(context), _font(fontPath), _size(size) {
  FT_Error error;

  if (ftLibrary == nullptr) {
    error = FT_Init_FreeType(&ftLibrary);
    if (error)
      throw std::exception(FT_Error_String(error));
  }

  error = FT_New_Face(ftLibrary, _font.c_str(), 0, &_face);
  if (error)
    throw std::exception(FT_Error_String(error));

  error = FT_Set_Char_Size(_face, 0, _size * 64, 0, FONT_DPI);
  if (error)
    throw std::exception(FT_Error_String(error));
}

const Glyph& Font::GetGlyph(CharCode code) {
  if (_characterCache.contains(code))
    return _characterCache.at(code);

  auto glyphIndex = FT_Get_Char_Index(_face, code);
  if (glyphIndex == 0)
    throw std::exception();

  auto error = FT_Load_Glyph(_face, glyphIndex, FT_LOAD_DEFAULT);
  if (error)
    throw std::exception(FT_Error_String(error));

  error = FT_Render_Glyph(_face->glyph, FT_RENDER_MODE_NORMAL);
  if (error)
    throw std::exception(FT_Error_String(error));

  _characterCache.try_emplace(code, GetGraphicsContext(), _face->glyph);

  return _characterCache.at(code);
}

Font::~Font() { FT_Done_Face(_face); }

std::vector<Pointer<Font>> fontCache;

Font* Font::Load(rndr::GraphicsContext* context, const char* fontPath,
                 float size) {

  for (auto& font : fontCache) {
    if (&(*font->GetGraphicsContext()) == context && font->_font == fontPath &&
        font->_size == size)
      return font;
  }

  auto font = new Font(context, fontPath, size);
  fontCache.emplace_back(font);

  return font;
}

} // namespace xgdi