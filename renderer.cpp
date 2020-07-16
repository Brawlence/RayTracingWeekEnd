#include <thread>				// std::thread - concurrency
#include <vector>				// needed for array of threads
#include <float.h>

#include <SFML/Graphics.hpp>
#include <fmt/format.h>			
#include <fmt/ostream.h>		// needed for serialization of std::thread::thread_id via overloaded <<

#include "main.h"
#include "renderer.h"
#include "RayTracer/ray.h"
#include "RayTracer/vec3.h"

static int amount_of_threads = std::thread::hardware_concurrency() - 1;
std::vector<std::thread> threads(amount_of_threads);

float hit_sphere(const vec3& center, float radius, const ray& r) {
	vec3 A_minus_center = r.origin() - center;
	float quadratic_a = dot(r.direction(), r.direction());
	float quadratic_b = 2.0 * dot(r.direction(), r.origin() - center);
	float quadratic_c = dot(A_minus_center,A_minus_center) - radius*radius;
	float discriminant = quadratic_b*quadratic_b - 4 * quadratic_a * quadratic_c;
	if (discriminant < 0) {
		return -1.0;
	} else {
		return (-quadratic_b - sqrt(discriminant)) / (2.0*quadratic_a);
	};
}

vec3 color(const ray& r) {
	vec3 sphere_center(0,0,-1);
	float t = hit_sphere(sphere_center, 0.5, r);
	if (t > 0.0) {
		vec3 Normal = unit_vector(r.point_at_parameter(t) - sphere_center);
		return 0.5 * vec3(Normal.x()+1, Normal.y()+1,Normal.z()+1);
	}
	vec3 unit_direction = unit_vector(r.direction());
	t = 0.5 * (unit_direction.y() + 1.0f);
	return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5,0.7,1.0);
}

//renders chunk of image
void render_chunk(sf::Uint8 *target_image, window_properties win_prop, int lower_bound, int upper_bound, int left_bound, int right_bound) {
	vec3 origin(0.0, 0.0, 0.0);
	vec3 lower_left_corner(-2.0, -1.0, -1.0);
	vec3 horizon(4.0, 0.0, 0.0);
	vec3 vertical(0.0, 2.0, 0.0);
	for (int row = upper_bound-1; row >= lower_bound; row--) {
		for (int column = left_bound; column < right_bound; column++) {
			float u = float(column) / float(win_prop.width); // camera FoV
			float v = float(row) / float(win_prop.height);  // camera vertical
			ray r(origin, lower_left_corner + u*horizon + v*vertical);			
			vec3 col = color(r);

			int shift = 4*((win_prop.height-row-1)*win_prop.width+column);
			target_image[shift+0] = int(255.99*col[0]);
			target_image[shift+1] = int(255.99*col[1]);
			target_image[shift+2] = int(255.99*col[2]);
			target_image[shift+3] = int(255.99);
		} 
	}
	fmt::print("Thread with id '{}' has finished execution.\n", std::this_thread::get_id());
};

//starts threads 
void spin_threads(sf::Uint8 *image, window_properties win_prop) {
	for (int t = 0; t < threads.size(); ++t) { 
		int lower_bound = t*win_prop.height/amount_of_threads;
		int upper_bound = (t+1)==amount_of_threads ? win_prop.height : (t+1)*win_prop.height/amount_of_threads;
		int left_bound = 0;
		int right_bound = win_prop.width;
		threads[t] = std::thread(render_chunk, image, win_prop, lower_bound, upper_bound, left_bound, right_bound);
		fmt::print("Thread {} has started execution with id '{}'.\n", t, threads[t].get_id());
	};
};

//ends threads
void join_threads() {
	for (int t = 0; t < amount_of_threads; ++t) {
		threads[t].join();
	};
	fmt::print("Threads syncronized.\n");
};