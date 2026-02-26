#pragma once
#include "hittable.h"
#include "texture.h"   
#include <utility> 
// Surface/material behavior for ray hits (diffuse, metal, glass, light)
class material 
{
public:
    virtual ~material() = default;
    virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const = 0;
    virtual color emitted() const { return color(0, 0, 0); }
};
class diffuse_light : public material {
public:
    diffuse_light(const color& emit_color) : emit(emit_color) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override {
        return false; 
    }

    color emitted() const override {
        return emit;
    }

private:
    color emit;
};
class lambertian : public material {
public:
    explicit lambertian(const color& a)
        : albedo(make_shared<solid_color>(a)) {
    }

    explicit lambertian(shared_ptr<texture> tex)
        : albedo(std::move(tex)) {
    }

    bool scatter(const ray&, const hit_record& rec, color& attenuation, ray& scattered) const override {
        vec3 scatter_direction = rec.normal + random_unit_vector();

        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;

        scattered = ray(rec.p, scatter_direction);

        // texture sample
        attenuation = albedo->value(rec.u, rec.v, rec.p);
        return true;
    }

private:
    shared_ptr<texture> albedo;
};

class metal : public material 
{
public:
    color albedo;
    double fuzz;

    metal(const color& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override 
    {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        scattered = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        attenuation = albedo;
        return (dot(scattered.direction(), rec.normal) > 0);
    }
};

class dielectric : public material 
{
public:
    double ir; // index of refraction

    explicit dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    static double reflectance(double cosine, double ref_idx) 
    {
        // schlick approx
        double r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1 - cosine), 5);
    }

    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override 
    {
        attenuation = color(1.0, 1.0, 1.0);
        double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

        vec3 unit_dir = unit_vector(r_in.direction());
        double cos_theta = std::fmin(dot(-unit_dir, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
            direction = reflect(unit_dir, rec.normal);
        else
            direction = refract(unit_dir, rec.normal, refraction_ratio);

        scattered = ray(rec.p, direction);
        return true;
    }
};
