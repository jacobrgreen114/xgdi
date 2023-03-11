// Copyright (c) 2023 Jacob R. Green
// All Rights Reserved.

#pragma once

#include "muchcool/rndr.hpp"

namespace muchcool::xgdi {

using Point = glm::vec2;
using Size = glm::vec2;

/*
struct Size : public glm::vec2 {
  constexpr Size(float x, float y) : glm::vec2(x, y) {}
};
*/

struct Rect {
  Point Offset;
  Size Size;
};

template <typename T>
constexpr float Normalize(T x) {
  return x / (float)std::numeric_limits<T>::max();
}

// constexpr float Normalize(uint8 x) { return x / (float)UINT8_MAX; }

class Color : public glm::vec4 {
 public:
  constexpr Color() = default;
  constexpr Color(Color&&) = default;
  constexpr Color(const Color&) = default;
  constexpr Color& operator=(Color&&) = default;
  constexpr Color& operator=(const Color&) = default;

  constexpr Color(float r, float g, float b, float a = 1.0f)
      : glm::vec4(r, g, b, a) {}
  constexpr Color(float i) : Color(i, i, i) {}

  constexpr Color FromBytes(uint8 r, uint8 g, uint8 b) {
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

}  // namespace muchcool::xgdi