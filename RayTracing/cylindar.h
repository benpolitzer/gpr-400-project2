#pragma once
#include "hittable.h"

class cylindar : public hittable
{
public:
	point3 center;
	vec3 dir;
	double radius;
	double length;
	shared_ptr<material> mat;

	cylindar() : center(), dir(), radius(1), length(1), mat(nullptr) {}
	cylindar(point3 c, vec3 d, double r, double l, shared_ptr<material> m) :
		center(c), dir(d), radius(r), length(l), mat(std::move(m)){}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override
	{
		// check for infinite by checking for colision on the plane then checking 
		// where it is along the infinite cylindar
		// if its on the end treat it like a plane if not treat it like a sphere
		return false;
	}
};
