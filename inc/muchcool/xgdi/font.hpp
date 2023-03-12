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

  const FT_Glyph_Metrics metrics;

 public:
  const glm::vec2 size;
  const glm::vec2 bearing;
  const glm::i32vec2 bitmap_baseline;
  const float advance_;


  Glyph(Shared<rndr::GraphicsContext> context, ft::Glyph glyph);
  Glyph(Glyph&&) = default;
  Glyph(const Glyph&) = delete;

  auto& texture() const { return _texture; }

  auto advance() const { return advance_; }
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