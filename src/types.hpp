#pragma once
#include <cmath>

// Minimal 2D vector used throughout the decoder for position/velocity.
// TODO(you): fill in each body - see README.md Step 0.
struct Vec2 {
    double x = 0.0;
    double y = 0.0;

    Vec2() = default;
    Vec2(double x_, double y_) : x(x_), y(y_) {}

    Vec2 operator+(const Vec2& o) const {
        return {x + o.x, y + o.y};
    }
    Vec2 operator-(const Vec2& o) const {
        return {x - o.x, y - o.y};
    }
    Vec2 operator*(double s) const {
        return {x * s, y * s};
    }
    Vec2& operator+=(const Vec2& o) {
        x += o.x;
        y += o.y;
        return *this;
    }

    double norm() const { return std::sqrt(x * x + y * y); }
    double dot(const Vec2& o) const {
        return{x * o.x + y * o.y};
        
    }
};
