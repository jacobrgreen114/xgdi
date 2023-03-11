// Copyright (c) 2023 Jacob R. Green
// All Rights Reserved.

#include "muchcool/xgdi/datatypes.hpp"

namespace muchcool::xgdi {

constexpr Color Color::Transparent(0.0f, 0.0f, 0.0f, 0.0f);

constexpr Color Color::White(1.0f);
constexpr Color Color::Black(0.0f);

constexpr Color Color::GrayDark(0.125f);
constexpr Color Color::Gray(0.25f);
constexpr Color Color::GrayLight(0.5f);
constexpr Color Color::GrayLightExtra(0.75f);

constexpr Color Color::Red(1.0f, 0.0f, 0.0f);
constexpr Color Color::RedOrange(1.0f, 0.25f, 0.0f);
constexpr Color Color::Orange(1.0f, 0.5f, 0.0f);
constexpr Color Color::Amber(1.0f, 0.75f, 0.0f);
constexpr Color Color::Yellow(1.0f, 1.0f, 0.0f);
constexpr Color Color::Lime(0.5f, 1.0f, 0.0f);
constexpr Color Color::Green(0.0f, 1.0f, 0.0f);
constexpr Color Color::Cyan(0.0f, 1.0f, 1.0f);
constexpr Color Color::Blue(0.0f, 0.0f, 1.0f);
constexpr Color Color::Violet(1.0f / 3, 0.0f, 1.0f);
constexpr Color Color::Purple(2.0f / 3, 0.0f, 1.0f);
constexpr Color Color::Magenta(1.0f, 0.0f, 1.0f);
constexpr Color Color::Pink(1.0f, 0.0f, 0.5f);
constexpr Color Color::Salmon(1.0f, 0.0f, 1.0f / 3);

constexpr Color Color::SpaceGray(0.25f);
constexpr Color Color::RoseGold(1.0f, 0.75f, 0.75f);
constexpr Color Color::MidnightGreen(0.125f, 0.2f, 0.125f);

}  // namespace muchcool::xgdi