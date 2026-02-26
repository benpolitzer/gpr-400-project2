#pragma once
#include "hittable.h"
#include "hittable_list.h"
#include "finite_plane.h"
#include "material.h"
#include "rtweekend.h"

// Construct a box from set of finite_planes
class box : public hittable {
public:
    box() = default;

    box(const point3& pmin, const point3& pmax, shared_ptr<material> mat, bool include_front_face = true)
        : box_min(pmin), box_max(pmax)
    {
        const double dx = box_max.x() - box_min.x();
        const double dy = box_max.y() - box_min.y();
        const double dz = box_max.z() - box_min.z();

        // Sanity: degenerate box => no sides
        if (dx <= 0 || dy <= 0 || dz <= 0) return;

        // left
        sides.add(make_shared<finite_plane>(
            point3(box_min.x(), box_min.y(), box_min.z()),
            vec3(0, dy, 0),
            vec3(0, 0, dz),
            mat));

        // right
        sides.add(make_shared<finite_plane>(
            point3(box_max.x(), box_min.y(), box_min.z()),
            vec3(0, 0, dz),
            vec3(0, dy, 0),
            mat));

        // floor
        sides.add(make_shared<finite_plane>(
            point3(box_min.x(), box_min.y(), box_min.z()),
            vec3(dx, 0, 0),
            vec3(0, 0, dz),
            mat));

        // ceiling
        sides.add(make_shared<finite_plane>(
            point3(box_min.x(), box_max.y(), box_min.z()),
            vec3(dx, 0, 0),
            vec3(0, 0, dz),
            mat));

        // back
        sides.add(make_shared<finite_plane>(
            point3(box_min.x(), box_min.y(), box_max.z()),
            vec3(dx, 0, 0),
            vec3(0, dy, 0),
            mat));

        // front
        if (include_front_face) {
            sides.add(make_shared<finite_plane>(
                point3(box_min.x(), box_min.y(), box_min.z()),
                vec3(dx, 0, 0),
                vec3(0, dy, 0),
                mat));
        }
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        return sides.hit(r, ray_t, rec);
    }

private:
    point3 box_min, box_max;
    hittable_list sides;
};