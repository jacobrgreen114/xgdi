// Copyright (c) 2022. Jacob R. Green
// All Rights Reserved.

#pragma once

#include "Font.hpp"

namespace xgdi {

class FormattedText : public virtual Object {
  Pointer<Font> _font;
  std::string _text;

public:
  FormattedText(Font* font, const char* text);

  inline auto& GetFont() const { return _font; }
  inline auto& GetText() { return _text; }
};

} // namespace xgdi