#pragma once
#include "hittable.h"
#include "infinite_cylinder.h"
#include "infinite_plane.h"

class cone : public hittable
{
public:
	point3 point;
	vec3 dir;
	double radius;
	double length;
	shared_ptr<material> mat;

	cone() : point(), dir(), radius(1), length(1), mat(nullptr) {}
	cone(point3 p, vec3 d, double r, double l, shared_ptr<material> m) :
		point(p), dir(unit_vector(d)), radius(r), length(l), mat(std::move(m)) {
	}

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override
	{
		
		//check the colision as if its 2d
		point3 pointOnPlane = r.origin() - project(r.origin() - point, dir);
		vec3 dirOnPlane = r.direction() - project(r.direction(), dir);
		ray rayOnPlane(pointOnPlane, dirOnPlane);
		
		sphere s(point, radius, mat);

		//this hits the infinite cone
		if (s.hit(rayOnPlane, ray_t, rec))
		{
			rec.p = r.origin() + r.direction() * rec.t;
			if (project(rec.p - point, dir).length() <= length && rec.t > 0)
			{
				vec3 tempVector = project(rec.p - point, dir);
				if (dot(tempVector, dir) < 0) return false;
				s = sphere(point, radius * tempVector.length() / length, mat);
				if(s.hit(rayOnPlane, ray_t, rec))
					return true;
				
				point3 tempPoint = point + dir * length;
				infinite_plane plane(tempPoint, dir, mat);
				bool result = plane.hit(r, ray_t, rec);
				if (result && rec.front_face && (rec.p - tempPoint).length() <= radius)
					return true;
			}
		}

		return false;
	}
};
