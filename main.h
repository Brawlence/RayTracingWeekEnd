// These definitions were added in 2019 by Brawlence to make it work under Win7 x64 on VSC + minGW
//what the HECK where are my constants and random functions at - those had to be re-defined for windows
#ifndef M_PI
    #define M_PI 3.14159265358979323846264338327950288
#endif

#ifndef srand48
    #define srand48(x) srand((int)(x))
#endif

#ifndef drand48
    #define drand48() ((double)rand()/RAND_MAX)
#endif

#ifndef MAXFLOAT
    #define MAXFLOAT (FLT_MAX)
#endif

int spin_threads(sf::Uint8 *image);
int join_threads();
int render_chunk (sf::Uint8 *target_image, int lower_bound, int upper_bound, int left_bound, int right_bound);