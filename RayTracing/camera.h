#pragma once
#include "rtweekend.h"
#include "hittable.h"
#include "material.h"
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <iomanip>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

struct RenderConfig {
    int image_width = 500;
    double aspect_ratio = 16.0 / 9.0;
    int samples_per_pixel = 50;
    int max_depth = 10;

    bool multithreaded = true;
    unsigned thread_count = 0; 

    bool benchmark_both = false;

    std::string tag = "output"; 
};

class camera {
public:
    double aspect_ratio = 16.0 / 9.0;
    int image_width = 500;
    int samples_per_pixel = 50;
    int max_depth = 10;

    double vfov = 20;
    point3 lookfrom = point3(13, 2, 3);
    point3 lookat = point3(0, 0, 0);
    vec3 vup = vec3(0, 1, 0);

    double defocus_angle = 0.0; 
    double focus_dist = 10.0;

    void render(const hittable& world) 
    {
        initialize();

        namespace fs = std::filesystem;

        fs::path outDir = fs::current_path() / "Outputs";
        fs::create_directories(outDir);

        const unsigned hw = std::max(1u, std::thread::hardware_concurrency());
        const unsigned thread_count = hw;

        std::ostringstream name;
        name << "out_"
            << image_width << "x" << image_height
            << "_spp" << samples_per_pixel
            << "_fd" << focus_dist
            << "_thr" << hw
            << ".ppm";

        fs::path outPath = outDir / name.str();

        auto t_start = std::chrono::steady_clock::now();

        std::cerr << "========== Render Settings =========\n";
        std::cerr << "Output: " << outPath.string() << "\n";
        std::cerr << "Resolution: " << image_width << " x " << image_height << "\n";
        std::cerr << "Samples/Pixel: " << samples_per_pixel << "\n";
        std::cerr << "Threads: " << hw << "\n";
        std::cerr << "====================================\n";

        std::vector<color> framebuffer(image_width * image_height, color(0, 0, 0));

        std::atomic<int> next_row{ 0 };
        std::atomic<int> rows_done{ 0 };

        auto t_render_start = std::chrono::steady_clock::now();

        auto worker = [&]() 
            {
            while (true) 
            {
                int j = next_row.fetch_add(1);
                if (j >= image_height) break;

                for (int i = 0; i < image_width; ++i) 
                {
                    color pixel_color(0, 0, 0);
                    for (int s = 0; s < samples_per_pixel; ++s) 
                    {
                        ray r = get_ray(i, j);
                        pixel_color += ray_color(r, max_depth, world);
                    }
                    framebuffer[j * image_width + i] = pixel_color;
                }

                rows_done.fetch_add(1);
            }
            };

        std::vector<std::thread> threads;
        threads.reserve(thread_count);
        for (unsigned t = 0; t < thread_count; ++t)
            threads.emplace_back(worker);

        auto last_print = std::chrono::steady_clock::now();

        double ema_rows_per_sec = 0.0;
        bool ema_initialized = false;

        const int ETA_MIN_ROWS = 25; 
        const double ETA_MIN_SECS = 2.0; 
        const double EMA_ALPHA = 0.15;

        while (rows_done.load() < image_height) {
            auto now = std::chrono::steady_clock::now();
            if (now - last_print >= std::chrono::milliseconds(150)) 
            {
                last_print = now;

                int done = rows_done.load();
                double pct = 100.0 * (double)done / (double)image_height;

                double elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - t_render_start).count() / 1000.0;

                double inst_rows_per_sec = (elapsed > 0.0) ? (done / elapsed) : 0.0;

                if (!ema_initialized) 
                {
                    ema_rows_per_sec = inst_rows_per_sec;
                    ema_initialized = true;
                }
                else 
                {
                    ema_rows_per_sec = EMA_ALPHA * inst_rows_per_sec + (1.0 - EMA_ALPHA) * ema_rows_per_sec;
                }

                bool show_eta = (done >= ETA_MIN_ROWS) && (elapsed >= ETA_MIN_SECS) && (ema_rows_per_sec > 0.0);

                std::cerr << "\rRender: " << std::fixed << std::setprecision(1)
                    << pct << "% (" << done << "/" << image_height << " rows)";

                if (show_eta) 
                {
                    int remaining_rows = image_height - done;
                    double eta_sec = remaining_rows / ema_rows_per_sec;

                    int eta_int = (int)(eta_sec + 0.5);
                    int eta_min = eta_int / 60;
                    int eta_rem = eta_int % 60;

                    std::cerr << "  ETA: " << format_time_seconds(eta_sec) << " ";
                }
                else 
                {
                    std::cerr << "  ETA: -- ";
                }
                std::cerr << "Elapsed: " << format_time_seconds(elapsed);
                std::cerr << "   " << std::flush;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }

        for (auto& th : threads) th.join();

        auto t_render_end = std::chrono::steady_clock::now();
        std::cerr << "\rRender: 100.0% (" << image_height << "/" << image_height
            << " rows)  ETA: 0m 0s   \n";

        std::ofstream out(outPath, std::ios::out | std::ios::trunc);
        if (!out) 
        {
            std::cerr << "ERROR: Failed to open output file: " << outPath.string() << "\n";
            return;
        }

        std::cerr << "Writing file...\n";
        auto t_write_start = std::chrono::steady_clock::now();

        out << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        for (int j = 0; j < image_height; ++j) 
        {
            for (int i = 0; i < image_width; ++i) 
            {
                write_color(out, framebuffer[j * image_width + i]);
            }

            if ((j % 10) == 0 || j == image_height - 1)
            {
                double pct = 100.0 * (double)(j + 1) / (double)image_height;
                std::cerr << "\rWrite:  " << std::fixed << std::setprecision(1)
                    << pct << "% (" << (j + 1) << "/" << image_height << " rows)   "
                    << std::flush;
            }
        }

        auto t_write_end = std::chrono::steady_clock::now();
        out.close();

        std::cerr << "\rWrite:  100.0% (" << image_height << "/" << image_height << " rows)   \n";

        auto render_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_render_end - t_render_start).count();
        auto write_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_write_end - t_write_start).count();
        auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t_write_end - t_start).count();

        std::cerr << "============= Timing ===============\n";
        std::cerr << "Render: " << format_time_ms(render_ms) << "\n";
        std::cerr << "Write:  " << format_time_ms(write_ms) << "\n";
        std::cerr << "Total:  " << format_time_ms(total_ms) << "\n";
        std::cerr << "====================================\n";
        std::cerr << "Done. Wrote: " << outPath.string() << "\n";
    }


private:
    int image_height{};
    point3 center;
    point3 pixel00_loc;


    vec3 pixel_delta_u, pixel_delta_v;
    vec3 u, v, w;
    vec3 defocus_disk_u, defocus_disk_v;

    void initialize() 
    {
        image_height = static_cast<int>(image_width / aspect_ratio);
        if (image_height < 1) image_height = 1;

        center = lookfrom;

        auto theta = degrees_to_radians(vfov);
        auto h = std::tan(theta / 2);
        auto viewport_height = 2.0 * h * focus_dist;
        auto viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);

        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        vec3 viewport_u = viewport_width * u;
        vec3 viewport_v = viewport_height * -v;

        pixel_delta_u = viewport_u / image_width;
        pixel_delta_v = viewport_v / image_height;

        auto viewport_upper_left = center - (focus_dist * w) - viewport_u / 2 - viewport_v / 2;
        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        auto defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    ray get_ray(int i, int j) const 
    {
        auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
        auto pixel_sample = pixel_center + pixel_sample_square();

        auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
        auto ray_direction = pixel_sample - ray_origin;
        return ray(ray_origin, ray_direction);
    }

    vec3 pixel_sample_square() const 
    {
        auto px = -0.5 + random_double();
        auto py = -0.5 + random_double();
        return (px * pixel_delta_u) + (py * pixel_delta_v);
    }

    point3 defocus_disk_sample() const 
    {
        auto p = random_in_unit_disk();
        return center + (p.x() * defocus_disk_u) + (p.y() * defocus_disk_v);
    }

    color ray_color(const ray& r, int depth, const hittable& world) const 
    {
        if (depth <= 0) return color(0, 0, 0);

        hit_record rec;
        if (world.hit(r, interval(0.001, interval::universe.max), rec)) 
        {
            ray scattered;
            color attenuation;
            if (rec.mat && rec.mat->scatter(r, rec, attenuation, scattered))
                return attenuation * ray_color(scattered, depth - 1, world);
            return color(0, 0, 0);
        }

        vec3 unit_dir = unit_vector(r.direction());
        auto t = 0.5 * (unit_dir.y() + 1.0);
        return (1.0 - t) * color(1.0, 1.0, 1.0) + t * color(0.5, 0.7, 1.0);
    }

    static double clamp01(double x) 
    {
        if (x < 0.0) return 0.0;
        if (x > 0.999) return 0.999;
        return x;
    }

    void write_color(std::ostream& out, const color& pixel_color) const 
    {
        const double scale = 1.0 / samples_per_pixel;

        double r = std::sqrt(pixel_color.x() * scale);

        double g = std::sqrt(pixel_color.y() * scale);
        double b = std::sqrt(pixel_color.z() * scale);

        int ir = static_cast<int>(256 * clamp01(r));
        int ig = static_cast<int>(256 * clamp01(g));
        int ib = static_cast<int>(256 * clamp01(b));

        out << ir << ' ' << ig << ' ' << ib << '\n';
    }
    static std::string format_time_seconds(double seconds)
    {
        if (seconds < 0.0) seconds = 0.0;

        int total = static_cast<int>(seconds + 0.5);
        if (total < 60) {
            return std::to_string(total) + "s";
        }

        int mins = total / 60;
        int secs = total % 60;

        return std::to_string(mins) + "m " + std::to_string(secs) + "s";
    }

    static std::string format_time_ms(long long ms)
    {
        double seconds = ms / 1000.0;
        return format_time_seconds(seconds);
    }
};
