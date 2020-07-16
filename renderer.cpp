#include <thread>				// std::thread - concurrency
#include <vector>				// needed for array of threads
#include <float.h>

#include <SFML/Graphics.hpp>
#include <fmt/format.h>			
#include <fmt/ostream.h>		// needed for serialization of std::thread::thread_id via overloaded <<

#include "renderer.h"
#include "RayTracer/sphere.h"
#include "RayTracer/hitable_list.h"
#include "RayTracer/material.h"
#include "RayTracer/camera.h"

static int amount_of_threads = std::thread::hardware_concurrency() - 1;
std::vector<std::thread> threads(amount_of_threads);

vec3 color(const ray& r, hitable *world, int depth) {
	hit_record rec;
	if (world->hit(r, 0.001, MAXFLOAT, rec)) {
		ray scattered;
		vec3 attenuation;
		if (depth < 50 && rec.mat_ptr->scatter(r, rec, attenuation, scattered)) {
			 return attenuation*color(scattered, world, depth+1);
		}
		else {
			return vec3(0,0,0);
		}
	}
	else {
		vec3 unit_direction = unit_vector(r.direction());
		float t = 0.5*(unit_direction.y() + 1.0);
		return (1.0-t)*vec3(1.0, 1.0, 1.0) + t*vec3(0.5, 0.7, 1.0);
	}
}


hitable *random_scene() {
	int n = 500;
	hitable **list = new hitable*[n+1];
	list[0] =  new sphere(vec3(0,-1000,0), 1000, new lambertian(vec3(0.5, 0.5, 0.5)));
	int i = 1;
	for (int a = -7; a < 7; a++) {
		for (int b = -7; b < 7; b++) {
			float choose_mat = drand48();
			vec3 center(a+0.9*drand48(),0.2,b+0.9*drand48()); 
			if ((center-vec3(4,0.2,0)).length() > 0.9) { 
				if (choose_mat < 0.8) {  // diffuse
					list[i++] = new sphere(center, 0.2, new lambertian(vec3(drand48()*drand48(), drand48()*drand48(), drand48()*drand48())));
				}
				else if (choose_mat < 0.95) { // metal
					list[i++] = new sphere(center, 0.2,
							new metal(vec3(0.5*(1 + drand48()), 0.5*(1 + drand48()), 0.5*(1 + drand48())),  0.5*drand48()));
				}
				else {  // glass
					list[i++] = new sphere(center, 0.2, new dielectric(1.5));
				}
			}
		}
	}

	list[i++] = new sphere(vec3(0, 1, 0), 1.0, new dielectric(1.5));
	list[i++] = new sphere(vec3(-4, 1, 0), 1.0, new lambertian(vec3(0.4, 0.2, 0.1)));
	list[i++] = new sphere(vec3(4, 1, 0), 1.0, new metal(vec3(0.7, 0.6, 0.5), 0.0));

	return new hitable_list(list,i);
}

//renders chunk of image
void render_chunk(hitable *world, camera cam, sf::Uint8 *target_image, window_properties win_prop, int lower_bound, int upper_bound, int left_bound, int right_bound, int samples) {
	for (int row = upper_bound-1; row >= lower_bound; row--) {
		for (int column = left_bound; column < right_bound; column++) {
			vec3 col(0, 0, 0);
			for (int s=0; s < samples; s++) {
				float u = float(column + drand48()) / float(win_prop.width);
				float v = float(row + drand48()) / float(win_prop.height);
				ray r = cam.get_ray(u, v);
				col += color(r, world,0);
			}
			col /= float(samples);
			col = vec3( sqrt(col[0]), sqrt(col[1]), sqrt(col[2]) );
			int shift = 4*((win_prop.height-row-1)*win_prop.width+column);
			target_image[shift+0] = int(255.99*col[0]);
			target_image[shift+1] = int(255.99*col[1]);
			target_image[shift+2] = int(255.99*col[2]);
			target_image[shift+3] = int(255.99);
		} 
	}
	fmt::print("Thread {} has finished execution.\n", std::this_thread::get_id());
};

//starts threads 
void spin_threads(sf::Uint8 *image, window_properties win_prop) {

	int ns = 5;										//Sample rays per pixel (antialiasing)
	//default Camera vectors and parameters
	vec3 lookfrom(11,4,5);
	vec3 lookat(0,0,0);
	float dist_to_focus = 10.0;
	float aperture = 0.1;

	//create world
    hitable *list[5];
    float R = cos(M_PI/4);
    list[0] = new sphere(vec3(0,0,-1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
    list[1] = new sphere(vec3(0,-100.5,-1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
    list[2] = new sphere(vec3(1,0,-1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.0));
    list[3] = new sphere(vec3(-1,0,-1), 0.5, new dielectric(1.5));
    list[4] = new sphere(vec3(-1,0,-1), -0.45, new dielectric(1.5));
    
    hitable *world = new hitable_list(list,5);

    world = random_scene();

	camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(win_prop.width)/float(win_prop.height), aperture, dist_to_focus);

	for (int t = 0; t < threads.size(); ++t) { 
		int lower_bound = t*win_prop.height/amount_of_threads;
		int upper_bound = (t+1)==amount_of_threads ? win_prop.height : (t+1)*win_prop.height/amount_of_threads;
		int left_bound = 0;
		int right_bound = win_prop.width;
		threads[t] = std::thread(render_chunk, world, cam, image, win_prop, lower_bound, upper_bound, left_bound, right_bound, ns);
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