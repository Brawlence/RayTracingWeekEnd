//==================================================================================================
// Originally written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// Modified in 2019 by Brawlence
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is distributed
// without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication along
// with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==================================================================================================

#include <iostream> // std::cout
#include <fstream>  //TODO: eventually write into a JPG / PNG, not PPM
#include <string>   // argv parsing - std::string
#include <sstream> //  string stream
//#include "stdlib.h" //TODO: for clearing the screen while progressbar is typed
#include "main.h"
#include "sphere.h"
#include "hitable_list.h"
#include "float.h"
#include "camera.h"
#include "material.h"

#include "CLI11.hpp"


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
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
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

// this will be used for each frame of an animation animation
void render (hitable *world,std::string nameOfOutput, int nx, int ny, int ns, vec3 lookfrom, vec3 lookat, float dist_to_focus, float aperture){
    float progress = 0.0;

    std::cout << "Rendering with the following parameters:\n\nWidth: " << nx << " Height: " << ny << ", Samples per pixel: " << ns
        << "\n\nCamera position: " << lookfrom << ", Target position: " << lookat 
        << "\n\nFocus distance: " << dist_to_focus << ", Aperture: " << aperture << std::endl;

    std::ofstream outputFile;
    outputFile.open(nameOfOutput, std::ios::out | std::ios::trunc);

    std::cout << "\nFile " << nameOfOutput << " opened. Target size: " << (16 + nx * ny * 13) << " bytes." << std::endl;

    // output the header: P3 means that the colours are represented in ASCII, 
    // nx colums, ny rows ; the maximum possible value for a chanell is 255
    outputFile << "P3\n" << nx << " " << ny << "\n255\n";

    // the camera properties
    camera cam(lookfrom, lookat, vec3(0,1,0), 20, float(nx)/float(ny), aperture, dist_to_focus);

    for (int j = ny-1; j >= 0; j--) {
        for (int i = 0; i < nx; i++) {
            vec3 col(0, 0, 0);
            for (int s=0; s < ns; s++) {
                float u = float(i + drand48()) / float(nx);
                float v = float(j + drand48()) / float(ny);
                ray r = cam.get_ray(u, v);
                vec3 p = r.point_at_parameter(2.0);
                col += color(r, world,0);
            }
            col /= float(ns);
            col = vec3( sqrt(col[0]), sqrt(col[1]), sqrt(col[2]) );
            int ir = int(255.99*col[0]); 
            int ig = int(255.99*col[1]); 
            int ib = int(255.99*col[2]); 
            
            // triplet writing
            outputFile << ir << " " << ig << " " << ib << "\n";
        }
        progress = float(ny-j) / float(ny);
        if ( int(progress*1000)%100 < 1 ) { 
            //TODO: make an additional *.h file for this
            //std::cout << "[__________________________________________________]\n";
            //std::cout <<"\b\b+]";
            std::cout << int(progress*100) << "% ... ";
        };    
    };
    std::cout << std::endl;
    outputFile.close();
}


int main( int argc, char *argv[] ) {

    CLI::App app("Ray-tracer renderer");

    // default values of Width, Height and Sample rays per pixel (antialiasing)
    int nx = 640;
    int ny = 480;
    int ns = 3;
    
    std::string nameOfOutput="1.ppm";

    //default Camera vectors and parameters
    vec3 lookfrom(15,3,2);
    vec3 lookat(0,0,0);
    float dist_to_focus = 10.0;
    float aperture = 0.05;

    //for use in generating animations
    bool animation = false;
    vec3 anim_position_start;
    vec3 anim_position_end;
    float anim_focus_start;
    float anim_focus_end;
    float anim_aperture_start;
    float anim_aperture_end;
    int anim_steps;

    app.add_option("-o,--output", nameOfOutput, "The output file name");
    app.add_option("-h,--height", ny, "The height of the output picture");
    app.add_option("-w,--width", nx, "The width of the output picture");
    app.add_option("-s,--samples", ns, "The amount of samples to take for a given pixel");
    app.add_option("-C,--lookfrom", lookfrom, "Camera position relative to the world");
    app.add_option("-T,--lookat", lookat, "Target which the camera is pointed at");
    app.add_option("-F,--focus", dist_to_focus, "Focus distance of the camera");
    app.add_option("-A,--aperture", aperture, "Aperture size of the camera");
    
    app.add_option("--animate", animation, "Well, if you need to make a clip");
    app.add_option("--anim_position_start", anim_position_start, "Camera position at the start of animation");    
    app.add_option("--anim_position_end", anim_position_end, "Camera position at the end of animation");
    app.add_option("--anim_focus_start", anim_focus_start, "Camera focus at the start of animation");    
    app.add_option("--anim_focus_end", anim_focus_end, "Camera focus at the end of animation");
    app.add_option("--anim_aperture_start", anim_aperture_start, "Camera focus at the start of animation");    
    app.add_option("--anim_aperture_end", anim_aperture_end, "Camera focus at the end of animation");
    
    app.add_option("--anim_steps", anim_steps, "The amount of steps taken");
    
    CLI11_PARSE(app, argc, argv);
    
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

    

    if (!animation) { render(world,nameOfOutput,nx,ny,ns,lookfrom,lookat,dist_to_focus,aperture); } else {
        for (int i = 0; i <= anim_steps; i++){
            std::cout << "Frame " << i << "... " << std::endl; 
            nameOfOutput = "animation\\out_" + std::to_string(i) + ".ppm";
            float coeff = float(i)/float(anim_steps);
            vec3 current_post = anim_position_start * (1 - coeff) + anim_position_end * coeff;
            float current_focus = anim_focus_start * (1 - coeff) + anim_focus_end * coeff;
            float current_aperture = anim_aperture_start * (1 - coeff) + anim_aperture_end * coeff;
            render(world,nameOfOutput,nx,ny,ns,current_post,lookat,current_focus,current_aperture);
        } 
    }


    return 0;
}
