#pragma once
#include "hittable.h"
#include "sphere.h"

class infiniteCylindar : hittable
{
public:
	point3 center;
	vec3 dir;
	double radius;
	shared_ptr<material> mat;

	infiniteCylindar() : center(), dir(), radius(1), mat(nullptr) {}
	infiniteCylindar(point3 c, vec3 d, double r, double l, shared_ptr<material> m) :
		center(c), dir(d), radius(r), mat(std::move(m)) {
	}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override
	{
		//check the colision as if its 2d
		point3 pointOnPlane = r.origin() - dir * (dot(center - r.origin(), dir) / dot(dir, dir));
		vec3 dirOnPlane = r.direction() - dir * (dot(r.direction(), dir) / dot(dir, dir));
		ray rayOnPlane(pointOnPlane, dirOnPlane);
		sphere s(center, radius, mat);

		//this hits the infinite plane
		if (s.hit(rayOnPlane, ray_t, rec))
		{
			rec.p += dir * (dot(r.direction(), dir) / dot(dir, dir));
			return true;
		}

		return false;
	}
};