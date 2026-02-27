#pragma once
#include "hittable.h"
#include "infinite_cylinder.h"
#include "sphere.h"

class capsule : public hittable
{
public:
	point3 center;
	vec3 dir;
	double radius;
	double length;
	shared_ptr<material> mat;

	capsule() : center(), dir(), radius(1), length(1), mat(nullptr) {}
	capsule(point3 c, vec3 d, double r, double l, shared_ptr<material> m) :
		center(c), dir(d), radius(r), length(l), mat(std::move(m)) {
	}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override
	{
		infinite_cylinder ic(center, dir, radius, mat);

		if (ic.hit(r, ray_t, rec))
		{
			if (project(rec.p - center, dir).length() <= length)
				return true;

			point3 tempPoint = center + dir * length;
			sphere s(tempPoint, radius, mat);
			bool result = s.hit(r, ray_t, rec);
			if (result && rec.front_face)
				return true;

			tempPoint = center - dir * length;
			s = sphere(tempPoint, radius, mat);
			result = s.hit(r, ray_t, rec);
			if (result && rec.front_face)
				return true;
		}

		return false;
	}
};
