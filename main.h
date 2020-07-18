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

#ifdef __linux__ 
	#define APP_DEFAULT_WIDTH 800
	#define APP_DEFAULT_HEIGHT 600	
#elif _WIN32
	#define APP_DEFAULT_WIDTH 1280
	#define APP_DEFAULT_HEIGHT 720	
#else
	#define APP_DEFAULT_WIDTH 400
	#define APP_DEFAULT_HEIGHT 200		
#endif

#ifndef MAIN_H
#define MAIN_H
class window_properties {
public:
	int width = APP_DEFAULT_WIDTH, height = APP_DEFAULT_HEIGHT;
	int ratio_GCD = std::__gcd(width, height);
	bool fullscreen = false;
	int framerate_limiter = 120;
};
#endif