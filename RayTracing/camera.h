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
#include <functional>
// Generates camera rays, performs path tracing, and writes the final image to PPM
    // also handles multithreaded rendering, progress display, and timing output

// Settomgs/Config struct to be passed before rendering (not in use atm)
struct RenderConfig {
    int image_width = 500;
    double aspect_ratio = 16.0 / 9.0;
    int samples_per_pixel = 50;
    int max_depth = 10;

    bool multithreaded = true;
    unsigned thread_count = 0;

    bool benchmark_both = false;
    std::string tag = "Output";
};

class camera {
public:
    // Image/render settings
    double aspect_ratio = 16.0 / 9.0;
    int image_width = 500;
    int samples_per_pixel = 50;
    int max_depth = 10;

    // Camera transform/view settings
    double vfov = 20.0;
    point3 lookfrom = point3(13.0, 2.0, 3.0);
    point3 lookat = point3(0.0, 0.0, 0.0);
    vec3 vup = vec3(0.0, 1.0, 0.0);

    // Depth of field controls
    double defocus_angle = 0.0;
    double focus_dist = 10.0;

    // Render the entire image
        // initialize camera geometry
        // multithreaded render into framebuffer
        // write framebuffer to PPM
    void render(const hittable& world)
    {
        initialize();

        namespace fs = std::filesystem;

        // Output folder
        fs::path outDir = fs::current_path() / "Outputs";
        fs::create_directories(outDir);

        // Number of worker threads (equal to number of threads on CPU)
        const unsigned hw = std::max(1u, std::thread::hardware_concurrency());
        const unsigned thread_count = hw;

        // Filename "constructor"
        std::ostringstream name;
        name << "out_"
            << image_width << "x" << image_height
            << "_spp" << samples_per_pixel
            << "_fd" << focus_dist
            << "_thr" << hw
            << ".ppm";

        fs::path outPath = outDir / name.str();

        // Render and write timer combined
        std::chrono::steady_clock::time_point t_start = std::chrono::steady_clock::now();

        std::cerr << "========== Render Settings =========\n";
        std::cerr << "Output: " << outPath.string() << "\n";
        std::cerr << "Resolution: " << image_width << " x " << image_height << "\n";
        std::cerr << "Samples/Pixel: " << samples_per_pixel << "\n";
        std::cerr << "Threads: " << hw << "\n";
        std::cerr << "====================================\n";

        // Framebuffer stores the accumulated linear color result for each pixel
        // We write to file later in one pass through a single thread
        std::vector<color> framebuffer(
            static_cast<size_t>(image_width * image_height),
            color(0.0, 0.0, 0.0)
        );

        std::atomic<int> next_row{ 0 }; // row index workers claim
        std::atomic<int> rows_done{ 0 }; // progress display

        std::chrono::steady_clock::time_point t_render_start = std::chrono::steady_clock::now();

        // Each thread repeatedly claims one row and renders all pixels in that row in an attempt to balance uneven scenes
        std::function<void()> worker = [&]() 
            {
                while (true)
                {
                    int j = next_row.fetch_add(1);
                    if (j >= image_height) {
                        break;
                    }

                    for (int i = 0; i < image_width; ++i)
                    {
                        color pixel_color(0.0, 0.0, 0.0);

                        // Monte Carlo sampling per pixel
                            // random repeated sampling to estimate values that are too expensive to calculate exactly
                        for (int s = 0; s < samples_per_pixel; ++s)
                        {
                            ray r = get_ray(i, j);
                            pixel_color += ray_color(r, max_depth, world);
                        }

                        framebuffer[static_cast<size_t>(j * image_width + i)] = pixel_color;
                    }

                    rows_done.fetch_add(1);
                }
            };

        // Launch worker threads
        std::vector<std::thread> threads;
        threads.reserve(thread_count);

        for (unsigned t = 0; t < thread_count; ++t)
        {
            threads.emplace_back(worker);
        }

        // Progress printing state
        std::chrono::steady_clock::time_point last_print = std::chrono::steady_clock::now();

        // ETA smoothing
        double ema_rows_per_sec = 0.0;
        bool ema_initialized = false;

        const int ETA_MIN_ROWS = 25; // wait until enough work is done to start displaying ETA
        const double ETA_MIN_SECS = 2.0; // enough time has elapsed to start displaying ETA
        const double EMA_ALPHA = 0.15; // smoothing factor

        // Main thread monitors progress while workers render
        while (rows_done.load() < image_height)
        {
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();

            if (now - last_print >= std::chrono::milliseconds(150))
            {
                last_print = now;

                int done = rows_done.load();
                double pct = 100.0 * static_cast<double>(done) / static_cast<double>(image_height);

                double elapsed =
                    std::chrono::duration_cast<std::chrono::milliseconds>(now - t_render_start).count() / 1000.0;

                double inst_rows_per_sec = (elapsed > 0.0)
                    ? (static_cast<double>(done) / elapsed)
                    : 0.0;

                if (!ema_initialized)
                {
                    ema_rows_per_sec = inst_rows_per_sec;
                    ema_initialized = true;
                }
                else
                {
                    ema_rows_per_sec = EMA_ALPHA * inst_rows_per_sec
                        + (1.0 - EMA_ALPHA) * ema_rows_per_sec;
                }

                bool show_eta = (done >= ETA_MIN_ROWS)
                    && (elapsed >= ETA_MIN_SECS)
                    && (ema_rows_per_sec > 0.0);

                std::cerr << "\rRender: "
                    << std::fixed << std::setprecision(1)
                    << pct << "% (" << done << "/" << image_height << " rows) |";

                if (show_eta)
                {
                    int remaining_rows = image_height - done;
                    double eta_sec = static_cast<double>(remaining_rows) / ema_rows_per_sec;

                    std::cerr << " Estimated Time Remaining: "
                        << format_time_seconds(eta_sec) << " ";
                }
                else
                {
                    std::cerr << " Estimated Time Remaining: -- ";
                }

                std::cerr << "| Elapsed: " << format_time_seconds(elapsed);
                std::cerr << "   " << std::flush;
            }

            // Quick sleep to reduce amount we write to console
            std::this_thread::sleep_for(std::chrono::milliseconds(25));
        }

        // Wait for all worker threads to finish before writing output
        for (std::thread& th : threads)
        {
            th.join();
        }

        std::chrono::steady_clock::time_point t_render_end = std::chrono::steady_clock::now();

        std::cerr << "\rRender: 100.0% (" << image_height << "/" << image_height
            << " rows) | Estimated Time Remaining: 0m 0s   \n";

        // Open output file after rendering
        std::ofstream out(outPath, std::ios::out | std::ios::trunc);
        if (!out)
        {
            std::cerr << "ERROR: Failed to open output file: " << outPath.string() << "\n";
            return;
        }

        std::cerr << "Writing file...\n";
        std::chrono::steady_clock::time_point t_write_start = std::chrono::steady_clock::now();

        // PPM header stuff
        out << "P3\n" << image_width << ' ' << image_height << "\n255\n";

        // Write framebuffer row by row
        for (int j = 0; j < image_height; ++j)
        {
            for (int i = 0; i < image_width; ++i)
            {
                write_color(out, framebuffer[static_cast<size_t>(j * image_width + i)]);
            }

            // Update write progress every so often
            if ((j % 10) == 0 || j == image_height - 1)
            {
                double pct = 100.0 * static_cast<double>(j + 1) / static_cast<double>(image_height);
                std::cerr << "\rWrite:  "
                    << std::fixed << std::setprecision(1)
                    << pct << "% (" << (j + 1) << "/" << image_height << " rows)   "
                    << std::flush;
            }
        }

        std::chrono::steady_clock::time_point t_write_end = std::chrono::steady_clock::now();
        out.close();

        std::cerr << "\rWrite:  100.0% (" << image_height << "/" << image_height << " rows)   \n";

        long long render_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(t_render_end - t_render_start).count();

        long long write_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(t_write_end - t_write_start).count();

        long long total_ms =
            std::chrono::duration_cast<std::chrono::milliseconds>(t_write_end - t_start).count();

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

    vec3 pixel_delta_u;
    vec3 pixel_delta_v;

    vec3 u, v, w;

    vec3 defocus_disk_u;
    vec3 defocus_disk_v;

    // Precompute camera geometry
        // image height, viewport size, pixel spacing, and camera basis
    void initialize()
    {
        image_height = static_cast<int>(static_cast<double>(image_width) / aspect_ratio);
        if (image_height < 1) 
        {
            image_height = 1;
        }

        center = lookfrom;

        double theta = degrees_to_radians(vfov);
        double h = std::tan(theta / 2.0);

        double viewport_height = 2.0 * h * focus_dist;
        double viewport_width = viewport_height *
            (static_cast<double>(image_width) / static_cast<double>(image_height));

        // Camera basis (right-handed):
            // w = backward, u = right, v = up in cam space
        w = unit_vector(lookfrom - lookat);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        vec3 viewport_u = viewport_width * u;
        vec3 viewport_v = viewport_height * -v; // negative because image rows go downward

        pixel_delta_u = viewport_u / static_cast<double>(image_width);
        pixel_delta_v = viewport_v / static_cast<double>(image_height);

        // Upper-left corner of viewport, then offset by half a pixel to get pixel center
        point3 viewport_upper_left =
            center - (focus_dist * w) - viewport_u / 2.0 - viewport_v / 2.0;

        pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);

        // Defocus disk basis (for depth of field)
        double defocus_radius = focus_dist * std::tan(degrees_to_radians(defocus_angle / 2.0));
        defocus_disk_u = u * defocus_radius;
        defocus_disk_v = v * defocus_radius;
    }

    // Generate one ray through a pixel (with some anti-aliasing and optional DOF)
    ray get_ray(int i, int j) const
    {
        point3 pixel_center = pixel00_loc
            + (static_cast<double>(i) * pixel_delta_u)
            + (static_cast<double>(j) * pixel_delta_v);

        point3 pixel_sample = pixel_center + pixel_sample_square();

        point3 ray_origin = (defocus_angle <= 0.0) ? center : defocus_disk_sample();
        vec3 ray_direction = pixel_sample - ray_origin;

        return ray(ray_origin, ray_direction);
    }

    // Random jitter within one pixel square for anti-aliasing
    vec3 pixel_sample_square() const
    {
        double px = -0.5 + random_double();
        double py = -0.5 + random_double();

        return (px * pixel_delta_u) + (py * pixel_delta_v);
    }

    // Random point on defocus disk for depth of field
    point3 defocus_disk_sample() const
    {
        vec3 p = random_in_unit_disk();
        return center + (p.x() * defocus_disk_u) + (p.y() * defocus_disk_v);
    }

    // Recursive path tracing:
        // intersect scene
        // add emitted light
        // scatter and recurse
        // return background if miss
    color ray_color(const ray& r, int depth, const hittable& world) const
    {
        if (depth <= 0) {
            return color(0.0, 0.0, 0.0);
        }

        hit_record rec;
        if (world.hit(r, interval(0.001, interval::universe.max), rec))
        {
            color emitted = rec.mat ? rec.mat->emitted() : color(0.0, 0.0, 0.0);

            ray scattered;
            color attenuation;

            if (rec.mat && rec.mat->scatter(r, rec, attenuation, scattered))
            {
                return emitted + attenuation * ray_color(scattered, depth - 1, world);
            }

            // Non-scattering mat
            return emitted;
        }

        // Background/Skybox
        vec3 unit_dir = unit_vector(r.direction());
        double t = 0.5 * (unit_dir.y() + 1.0);
        return (1.0 - t) * color(1.0, 1.0, 1.0)
            + t * color(0.5, 0.7, 1.0);
    }

    // Clamp to stay within 0-255 range
    static double clamp01(double x)
    {
        if (x < 0.0) return 0.0;
        if (x > 0.999) return 0.999;
        return x;
    }

    // Convert color sample to 8-bit RGB
        // average by spp
        // gamma correct 
        // clamp and write as integers
    void write_color(std::ostream& out, const color& pixel_color) const
    {
        double scale = 1.0 / static_cast<double>(samples_per_pixel);

        double r = std::sqrt(pixel_color.x() * scale);
        double g = std::sqrt(pixel_color.y() * scale);
        double b = std::sqrt(pixel_color.z() * scale);

        int ir = static_cast<int>(256.0 * clamp01(r));
        int ig = static_cast<int>(256.0 * clamp01(g));
        int ib = static_cast<int>(256.0 * clamp01(b));

        out << ir << ' ' << ig << ' ' << ib << '\n';
    }

    // Format seconds
    static std::string format_time_seconds(double seconds)
    {
        if (seconds < 0.0) {
            seconds = 0.0;
        }

        int total = static_cast<int>(seconds + 0.5); // round to nearest second

        if (total < 60) {
            return std::to_string(total) + "s";
        }

        int mins = total / 60;
        int secs = total % 60;

        return std::to_string(mins) + "m " + std::to_string(secs) + "s";
    }

    // Helper for millisecond durations
    static std::string format_time_ms(long long ms)
    {
        double seconds = static_cast<double>(ms) / 1000.0;
        return format_time_seconds(seconds);
    }
};