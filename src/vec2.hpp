#pragma once

#include<string>
#include<cmath>

struct Vec2 {
  int x;
  int y;
  Vec2();
  Vec2(int x, int y);
  bool operator==(const Vec2 & o) const;
  bool operator!=(const Vec2 & o) const;
  Vec2 operator*(const Vec2 & o) const;
  Vec2 operator*(int i) const;
  Vec2 operator+(const Vec2 & o) const;
  Vec2 operator-(const Vec2 & o) const;
  double Distance(const Vec2& o) const;
  int ManhattanDistance(const Vec2& o) const;
  void SwapXY();
  std::string to_string() const;
};
