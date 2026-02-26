#pragma once
#include "rtweekend.h"
// Defines texture interfaces and texture types used by materials
class texture {
public:
    virtual ~texture() = default;
    virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color : public texture {
public:
    solid_color() = default;
    explicit solid_color(const color& c) : color_value(c) {}

    color value(double, double, const point3&) const override {
        return color_value;
    }

private:
    color color_value{ 0,0,0 };
};