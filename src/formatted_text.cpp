// Copyright (c) 2023 Jacob R. Green
// All Rights Reserved.

#include "muchcool/xgdi/formatted_text.hpp"

namespace muchcool::xgdi {

FormattedText::FormattedText(Shared<Font> font, const char* text)
    : _font(std::move(font)), _text(text) {}

}  // namespace muchcool::xgdi