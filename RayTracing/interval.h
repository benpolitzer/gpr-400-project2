#pragma once
#include <limits>
// Defines a numeric interval (min, max) used for valid ray hit ranges, clamping, and avoiding invalid intersections
class interval {
public:
    double min, max;

    interval() : min(+std::numeric_limits<double>::infinity()),
        max(-std::numeric_limits<double>::infinity()) {}

    interval(double _min, double _max) : min(_min), max(_max) {}

    bool contains(double x) const { return min <= x && x <= max; }
    bool surrounds(double x) const { return min < x && x < max; }

    static const interval empty, universe;
};

inline const interval interval::empty(+std::numeric_limits<double>::infinity(),
    -std::numeric_limits<double>::infinity());

inline const interval interval::universe(-std::numeric_limits<double>::infinity(),
    +std::numeric_limits<double>::infinity());
