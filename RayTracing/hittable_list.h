#pragma once
#include <vector>
#include "hittable.h"

class hittable_list : public hittable {
public:
    std::vector<shared_ptr<hittable>> objects;

    hittable_list() = default;
    explicit hittable_list(shared_ptr<hittable> object) { add(std::move(object)); }

    void clear() { objects.clear(); }
    void add(shared_ptr<hittable> object) { objects.push_back(std::move(object)); }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override 
    {
        hit_record temp_rec;
        bool hit_anything = false;
        auto closest = ray_t.max;

        for (const auto& object : objects) 
        {
            if (object->hit(r, interval(ray_t.min, closest), temp_rec)) 
            {
                hit_anything = true;
                closest = temp_rec.t;
                rec = temp_rec;
            }
        }
        return hit_anything;
    }
};
