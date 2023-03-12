// Copyright (c) 2023 Jacob R. Green
// All Rights Reserved.

#pragma once

#include <freetype/freetype.hpp>

#include <muchcool/rndr.hpp>

#include <unordered_map>

namespace muchcool::xgdi {

using CharCode = ft::CharCode;

class Glyph : public rndr::GraphicsObject {
  Shared<rndr::Texture> _texture;

  FT_Glyph_Metrics _metrics;

  glm::vec2 _size;
  glm::vec2 _bearing;
  glm::vec2 _bitmap_size;
  glm::vec2 _bitmap_baseline;
  float _advance;

 public:
  Glyph(Shared<rndr::GraphicsContext> context, ft::Glyph glyph);
  Glyph(Glyph&&) = delete;
  Glyph(const Glyph&) = delete;

  auto& texture() const { return _texture; }

  auto& size() const { return _size; }
  auto& bearing() const { return _bearing; }
  auto& bitmap_size() const { return _bitmap_size; }
  auto& bitmap_baseline() const { return _bitmap_baseline; }
  auto advance() const { return _advance; }
};

class Font : public rndr::GraphicsObject {
 public:
#ifdef OS_WINDOWS
  static constexpr std::string_view Arial = "C:\\Windows\\Fonts\\Arial.ttf";
  static constexpr std::string_view SegoeUI = "C:\\Windows\\Fonts\\segoeui.ttf";
  static constexpr std::string_view Consolas =
      "C:\\Windows\\Fonts\\consola.ttf";
  static constexpr std::string_view ComicSans = "C:\\Windows\\Fonts\\Comic.ttf";
  static constexpr std::string_view CourierNew = "C:\\Windows\\Fonts\\cour.ttf";
#endif

 private:
  ft::Face _face;

  fs::path _font;
  float _size;

  std::unordered_map<CharCode, Glyph> _characterCache;

  Font(Shared<rndr::GraphicsContext> context, fs::path fontPath = SegoeUI,
       float size = 12.0f);

  Font(Font&&) = default;
  Font(const Font&) = delete;

 public:
  ~Font() override;

  static Shared<Font> Load(Shared<rndr::GraphicsContext> context,
                           fs::path fontPath = SegoeUI, float size = 12.0f);

  const Glyph& glyph(CharCode code);

  auto line_height() const { return _face.metrics().height / 64.0f; }

  auto ascender() const { return _face.metrics().ascender / 64.0f; }

  auto descender() const { return _face.metrics().descender / 64.0f; }

  // auto baseline_y() const { return _face.metrics().ascender / 64.0f; }
};

}  // namespace muchcool::xgdi