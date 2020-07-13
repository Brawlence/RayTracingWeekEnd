#include <thread>				// std::thread - concurrency
#include <vector>				// needed for array of threads
#include <float.h>

#include <SFML/Graphics.hpp>
#include <fmt/format.h>			
#include <fmt/ostream.h>		// needed for serialization of std::thread::thread_id via overloaded <<

#include "vec3.h"
#include "../main.h"

static int amount_of_threads = std::thread::hardware_concurrency() - 1;
std::vector<std::thread> threads(amount_of_threads);

//renders chunk of image
int render_chunk (sf::Uint8 *target_image, window_properties win_prop, int lower_bound, int upper_bound, int left_bound, int right_bound) {
	for (int row = upper_bound-1; row >= lower_bound; row--) {
		for (int column = left_bound; column < right_bound; column++) {
			float r = float(column) / float(win_prop.width);
			float g = float(row) / float(win_prop.height);
			float b = (((column%win_prop.ratio_GCD < 2)||(row%win_prop.ratio_GCD < 2))&&(win_prop.ratio_GCD > 15)) ? 0.3f : 0.6f;
			int shift = 4*((win_prop.height-row-1)*win_prop.width+column);
			target_image[shift+0] = int(255.99*r);
			target_image[shift+1] = int(255.99*g);
			target_image[shift+2] = int(255.99*b);
			target_image[shift+3] = int(255.99);
		} 
	}
	fmt::print("Thread with id '{}' has finished execution.\n", std::this_thread::get_id());
	return EXIT_SUCCESS;
};

//starts threads 
int spin_threads(sf::Uint8 *image, window_properties win_prop) {
	for (int t = 0; t < threads.size(); ++t) { 
		int lower_bound = t*win_prop.height/amount_of_threads;
		int upper_bound = (t+1)==amount_of_threads ? win_prop.height : (t+1)*win_prop.height/amount_of_threads;
		int left_bound = 0;
		int right_bound = win_prop.width;
		threads[t] = std::thread(render_chunk, image, win_prop, lower_bound, upper_bound, left_bound, right_bound);
		fmt::print("Thread {} has started execution with id '{}'.\n", t, threads[t].get_id());
	};
	return EXIT_SUCCESS;
};

//ends threads
inline int join_threads() {
	for (int t = 0; t < amount_of_threads; ++t) {
		threads[t].join();
	};
	fmt::print("Threads syncronized.\n");
	return EXIT_SUCCESS;
};