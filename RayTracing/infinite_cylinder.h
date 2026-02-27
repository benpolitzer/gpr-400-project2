#pragma once
#include "sphere.h"

class infinite_cylinder : public hittable
{
public:
	point3 center;
	vec3 dir;
	double radius;
	shared_ptr<material> mat;

	infinite_cylinder() : center(), dir(), radius(1), mat(nullptr) {}
	infinite_cylinder(point3 c, vec3 d, double r, shared_ptr<material> m) :
		center(c), dir(unit_vector(d)), radius(r), mat(std::move(m)) {
	}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override
	{
		//check the colision as if its 2d
		point3 pointOnPlane = r.origin() - project(r.origin() - center, dir);
		vec3 dirOnPlane = r.direction() - project(r.direction(), dir);
		ray rayOnPlane(pointOnPlane, dirOnPlane);
		sphere s(center, radius, mat);

		//this hits the infinite plane
		if (s.hit(rayOnPlane, ray_t, rec))
		{
			rec.p = r.origin() + r.direction() * rec.t;
			return true;
		}

		return false;
	}
};