// Copyright (c) 2023 Jacob R. Green
// All Rights Reserved.

#pragma once

#include "font.hpp"

namespace muchcool::xgdi {

class FormattedText final : public Object {
  Shared<Font> _font;
  std::string _text;

 public:
  FormattedText(Shared<Font> font, const char* text);

  constexpr auto& font() const { return _font; }
  constexpr auto& text() const { return _text; }
};

}  // namespace muchcool::xgdi