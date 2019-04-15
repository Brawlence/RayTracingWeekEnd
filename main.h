//==================================================================================================
// The other source code was written in 2016 by Peter Shirley <ptrshrl@gmail.com>
//
// These definitions were added in 2019 by Brawlence to make it work under Win7 x64 on VSC + minGW
//
// To the extent possible under law, the author(s) have dedicated all copyright and related and
// neighboring rights to this software to the public domain worldwide. This software is distributed
// without any warranty.
//
// You should have received a copy (see file COPYING.txt) of the CC0 Public Domain Dedication along
// with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
//==================================================================================================

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
