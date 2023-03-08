// Copyright (c) 2022-2023 Jacob R. Green
// All Rights Reserved.

#pragma once

#include "muchcool/rndr.hpp"

namespace xgdi {

using Point = glm::vec2;
using Size = glm::vec2;

/*
struct Size : public glm::vec2 {
  inline constexpr Size(float x, float y) : glm::vec2(x, y) {}
};
*/

struct Rect {
  Point Offset;
  Size Size;
};

template <typename T> constexpr float Normalize(T x) {
  return x / (float)std::numeric_limits<T>::max();
}

// inline constexpr float Normalize(uint8 x) { return x / (float)UINT8_MAX; }

class Color : public glm::vec4 {

public:
  constexpr Color() = default;
  constexpr Color(Color &&) = default;
  constexpr Color(const Color &) = default;
  constexpr Color &operator=(Color &&) = default;
  constexpr Color &operator=(const Color &) = default;

  inline constexpr Color(float r, float g, float b, float a = 1.0f)
      : glm::vec4(r, g, b, a) {}
  inline constexpr Color(float i) : Color(i, i, i) {}

  inline constexpr Color FromBytes(uint8 r, uint8 g, uint8 b) {
    return {Normalize(r), Normalize(g), Normalize(b), 1.0f};
  }

  // Primative Colors

  static const Color Transparent;

  static const Color White;
  static const Color Black;

  static const Color GrayDark;
  static const Color Gray;
  static const Color GrayLight;
  static const Color GrayLightExtra;

  static const Color Red;
  static const Color RedOrange;
  static const Color Orange;
  static const Color Amber;
  static const Color Yellow;
  static const Color Lime;
  static const Color Green;
  static const Color Cyan;
  static const Color Blue;
  static const Color Violet;
  static const Color Purple;
  static const Color Magenta;
  static const Color Pink;
  static const Color Salmon;

  // Artsy Colors

  static const Color SpaceGray;
  static const Color RoseGold;
  static const Color MidnightGreen;
};

inline const Color Color::Transparent(0.0f, 0.0f, 0.0f, 0.0f);

inline const Color Color::White(1.0f);
inline const Color Color::Black(0.0f);

inline const Color Color::GrayDark(0.125f);
inline const Color Color::Gray(0.25f);
inline const Color Color::GrayLight(0.5f);
inline const Color Color::GrayLightExtra(0.75f);

inline const Color Color::Red(1.0f, 0.0f, 0.0f);
inline const Color Color::RedOrange(1.0f, 0.25f, 0.0f);
inline const Color Color::Orange(1.0f, 0.5f, 0.0f);
inline const Color Color::Amber(1.0f, 0.75f, 0.0f);
inline const Color Color::Yellow(1.0f, 1.0f, 0.0f);
inline const Color Color::Lime(0.5f, 1.0f, 0.0f);
inline const Color Color::Green(0.0f, 1.0f, 0.0f);
inline const Color Color::Cyan(0.0f, 1.0f, 1.0f);
inline const Color Color::Blue(0.0f, 0.0f, 1.0f);
inline const Color Color::Violet(1.0f / 3, 0.0f, 1.0f);
inline const Color Color::Purple(2.0f / 3, 0.0f, 1.0f);
inline const Color Color::Magenta(1.0f, 0.0f, 1.0f);
inline const Color Color::Pink(1.0f, 0.0f, 0.5f);
inline const Color Color::Salmon(1.0f, 0.0f, 1.0f / 3);

inline const Color Color::SpaceGray(0.25f);
inline const Color Color::RoseGold(1.0f, 0.75f, 0.75f);
inline const Color Color::MidnightGreen(0.125f, 0.2f, 0.125f);

} // namespace xgdi