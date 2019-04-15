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

int main( int argc, char *argv[] ) {

    // default values of Width, Height and Sample rays per pixel (antialiasing)
    int nx = 640;
    int ny = 480;
    int ns = 3;

    float progress = 0.0;
    std::string nameOfOutput="1.ppm";
    //default Camera vectors and parameters
    vec3 lookfrom(15,3,2);
    vec3 lookat(0,0,0);
    float dist_to_focus = 10.0;
    float aperture = 0.05;

    if (argc > 0) { // TODO: remake this ugly code
    /*       for (int i = 1; i < argc; i++){
            std::string temp(argv[i]);
            if ( ( (temp.find("-o")) || (temp.find("-output")) )
                 && (argv[i+1]!=NULL)) {

                std::string nextarg(argv[++i]);
                nameOfOutput=nextarg;
            };
            if ( ( (temp.find("-h")) || (temp.find("-height")) ) 
                 && (argv[i+1]!=NULL)) {

                std::string nextarg(argv[++i]);
                ny=std::stoi(nextarg);
            };
            if ( ( (temp.find("-w")) || (temp.find("-width")) ) 
                 && (argv[i+1]!=NULL)) {

                std::string nextarg(argv[++i]);   
                nx=std::stoi(nextarg);
            };
            if ( ( (temp.find("-s")) || (temp.find("-samples")) )
                 && (argv[i+1]!=NULL)) {

                std::string nextarg(argv[++i]);     
                ns=std::stoi(nextarg);
            };
            if ( ( (temp.find("-lf")) || (temp.find("-lookfrom")) )
                && (argv[i+3]!=NULL)) {

            };
            if ( ( (temp.find("-la")) || (temp.find("-lookat")) )
                && (argv[i+3]!=NULL)) {

            };
            if ( (temp.find("-fd")) || (temp.find("-focus")) 
                 && (argv[i+1]!=NULL)) {
                std::string nextarg(argv[i++]);
                aperture=std::stof(nextarg);
            };
            if ( (temp.find("-ap")) || (temp.find("-aperture"))
                 && (argv[i+1]!=NULL)) {
                std::string nextarg(argv[i++]); 
                aperture=std::stof(nextarg);
            };
        };
    */
    };

    std::cout << "Rendering with the following parameters:\n\nWidth: " << nx << " Height:" << ny << ", Samples per pixel:" << ns
        << ";\n\nCamera position: " << lookfrom << ", Target position: " << lookat 
        << ";\n\nFocus distance: " << dist_to_focus << ", Aperture: " << aperture << std::endl;

    std::ofstream outputFile;
    outputFile.open(nameOfOutput, std::ios::out | std::ios::trunc);

    std::cout << "\nFile" << nameOfOutput << "is successfully opened. Target file size: " << (16 + nx * ny * 13) << " bytes." << std::endl;

    // output the header: P3 means that the colours are represented in ASCII, 
    // nx colums, ny rows ; the maximum possible value for a chanell is 255
    outputFile << "P3\n" << nx << " " << ny << "\n255\n";

    hitable *list[5];
    float R = cos(M_PI/4);
    list[0] = new sphere(vec3(0,0,-1), 0.5, new lambertian(vec3(0.1, 0.2, 0.5)));
    list[1] = new sphere(vec3(0,-100.5,-1), 100, new lambertian(vec3(0.8, 0.8, 0.0)));
    list[2] = new sphere(vec3(1,0,-1), 0.5, new metal(vec3(0.8, 0.6, 0.2), 0.0));
    list[3] = new sphere(vec3(-1,0,-1), 0.5, new dielectric(1.5));
    list[4] = new sphere(vec3(-1,0,-1), -0.45, new dielectric(1.5));
    hitable *world = new hitable_list(list,5);
    world = random_scene();

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

    outputFile.close();

    return 0;
}
