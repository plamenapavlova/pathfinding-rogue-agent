#include"vec2.hpp"

Vec2::Vec2()
    : x(0), y(0)
{}

Vec2::Vec2(int x, int y)
    : x(x), y(y)
{}

bool Vec2::operator==(const Vec2 & o) const {
    return o.x==x && o.y==y;
}

bool Vec2::operator!=(const Vec2 & o) const {
    return !(*this==o);
}

Vec2 Vec2::operator*(const Vec2 & o) const {
    return Vec2(x*o.x, y*o.y);
}

Vec2 Vec2::operator*(int i) const {
    return Vec2(x*i, y*i);
}

Vec2 Vec2::operator+(const Vec2 & o) const {
    return Vec2(x+o.x, y+o.y);
}

Vec2 Vec2::operator-(const Vec2 & o) const {
    return Vec2(x-o.x, y-o.y);
}
double Vec2::Distance(const Vec2& o) const {
    return std::sqrt(
        std::pow(x - o.x, 2.0) +
        std::pow(y - o.y, 2.0)
    );
}
int Vec2::ManhattanDistance(const Vec2& o) const {
  return std::abs(o.x-x) + std::abs(o.y-y);
}
void Vec2::SwapXY() {
    int temp = x;
    x = y;
    y = temp;
}
std::string Vec2::to_string() {
    return std::to_string(x) + "," + std::to_string(y);
}
