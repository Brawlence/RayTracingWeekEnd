#ifndef RENDERER_H
#define RENDERER_H

#include "main.h"
#include "RayTracer/camera.h"
#include "RayTracer/hitable.h"
#include <SFML/Graphics.hpp>

//renders chunk of image
void render_chunk(hitable *world, camera cam, sf::Uint8 *target_image, window_properties win_prop, int lower_bound, int upper_bound, int left_bound, int right_bound, int samples);

//starts threads
void spin_threads(sf::Uint8 *image, window_properties win_prop);

//ends threads
void join_threads();

#endif