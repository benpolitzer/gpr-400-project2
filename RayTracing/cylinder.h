#pragma once
#include "hittable.h"
#include "infinite_cylinder.h"
#include "infinite_plane.h"

class cylinder : public hittable
{
public:
	point3 center;
	vec3 dir;
	double radius;
	double length;
	shared_ptr<material> mat;

	cylinder() : center(), dir(), radius(1), length(1), mat(nullptr) {}
	cylinder(point3 c, vec3 d, double r, double l, shared_ptr<material> m) :
		center(c), dir(d), radius(r), length(l), mat(std::move(m)){}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override
	{
		// check for infinite by checking for colision on the plane then checking 
		// where it is along the infinite cylindar
		// if its on the end treat it like a plane if not treat it like a sphere

		infinite_cylinder ic(center, dir, radius, mat);

		if (ic.hit(r, ray_t, rec))
		{
			if(project(rec.p - center, dir).length() <= length)
				return true;

			point3 tempPoint = center + dir * length;
			infinite_plane plane(tempPoint, dir, mat);
			bool result = plane.hit(r, ray_t, rec);
			if (result && rec.front_face && (rec.p - tempPoint).length() <= radius)
				return true;

			tempPoint = center - dir * length;
			plane = infinite_plane(tempPoint, -dir, mat);
			result = plane.hit(r, ray_t, rec);
			if (result && rec.front_face && (rec.p - tempPoint).length() <= radius)
				return true;
		}

		return false;
	}
};
