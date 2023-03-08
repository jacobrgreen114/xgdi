// Copyright (c) 2022-2023 Jacob R. Green
// All Rights Reserved.

#pragma once

#include <ft2build.h>
#include FT_FREETYPE_H

#include <muchcool/rndr.hpp>

#include <unordered_map>

namespace xgdi {

#define SEGOE_UI "C:\\Windows\\Fonts\\segoeui.ttf"

using CharCode = char;

class Glyph : public rndr::GraphicsObject {
  Pointer<rndr::Texture> _texture;

  const FT_Glyph_Metrics metrics;

public:
  const glm::vec2 size;
  const glm::vec2 bearing;
  const float advance;

  Glyph(rndr::GraphicsContext *context, FT_GlyphSlot glyph);
  Glyph(Glyph &&) = default;
  Glyph(const Glyph &) = delete;

  inline auto &GetTexture() const { return _texture; }

  inline auto GetAdvance() const { return advance; }
};

class Font : public rndr::GraphicsObject {
private:
  FT_Face _face;

  std::string _font;
  float _size;

  std::unordered_map<CharCode, Glyph> _characterCache;

  Font(rndr::GraphicsContext *context, const char *fontPath = SEGOE_UI,
       float size = 12.0f);

  Font(Font &&) = default;
  Font(const Font &) = delete;

public:
  ~Font() override;

  static Font *Load(rndr::GraphicsContext *context,
                    const char *fontPath = SEGOE_UI, float size = 12.0f);

  const Glyph &GetGlyph(CharCode code);

  auto GetLineHeight() const { return _face->size->metrics.height / 64.0f; }

  auto GetMaxAscent() const { return _face->size->metrics.ascender / 64.0f; }

  auto GetMaxDescent() const { return _face->size->metrics.descender / 64.0f; }

  [[deprecated]] auto GetBaselineY() const {
    return _face->size->metrics.ascender / 64.0f;
  }

public:
  static inline const char *const Arial = "C:\\Windows\\Fonts\\Arial.ttf";
  static inline const char *const SegoeUI = "C:\\Windows\\Fonts\\segoeui.ttf";
  static inline const char *const Consolas = "C:\\Windows\\Fonts\\consola.ttf";
  static inline const char *const ComicSans = "C:\\Windows\\Fonts\\Comic.ttf";
  static inline const char *const CourierNew = "C:\\Windows\\Fonts\\cour.ttf";
};

} // namespace xgdi