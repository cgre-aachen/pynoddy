/*

This include file describes the available wrapping options.

Wrapping is the extension of each layer of the model so that it simulates a cyclic
function. The fill values in this extended model can influence the geophysical
spectral calculations and therefore, are best being user defined. The users's choice
is passed to the program in the WRAPPING structure.

Wrapping types (illustrated in 1-dimension only):

RAMP:

       _/\__       /|\__
   /\_/     \  ___/ |   \__
  /          \/     |      \__
 /                  |         \_  <----- wrap linearly ramps from RHS to LHS value
|      model        |   wrap    |
|___________________|___________|


FENCE:

       _/\__       /|\_
   /\_/     \  ___/ |  \
  /          \/     |   \ 
 /                  |    \     _
|      model        |wrap \ __/ |
|___________________|______|____|

                          ^
                          |_______ wrap linearly ramps from RHS to LHS value
                                   but passes through given 'Fence' value or mean


SET:

       _/\__       /|
   /\_/     \  ___/ |
  /          \/     |___________
 /                  |           |  <-- wrap 'set' to given value or mean (NOT cyclic!)
|      model        |   wrap    |
|___________________|___________|



REFLECTION:

       _/\__       /|\       _/\_
   /\_/     \  ___/ | \___  /    \_/\
  /          \/     |     \/         \
 /                  |                 \  <---- wrap is 'opposite' of model
|      model        |       wrap       |
|___________________|__________________|


*/
#define M_PI 3.14159265358979323846

#define WRAP_TYPES      6
#define WRAP_TYPE_RAMP     0
#define WRAP_TYPE_FENCE_MEAN  1
#define WRAP_TYPE_FENCE_VALUE 2
#define WRAP_TYPE_SET_MEAN 3
#define WRAP_TYPE_SET_VALUE   4
#define WRAP_TYPE_REFLECTION  5

typedef struct s_wrapping {

    int type; /* 0-5 as per WRAP_TYPE_* defines above */

    int percentage; /* 10-100% for non-REFLECTION wrapping */
          /* REFLECTION wrapping 0=scalar 1=vector */

/* following is used for wrap_types WRAP_TYPE_FENCE_VALUE & WRAP_TYPE_FENCE_MEAN */
    int fence; /* 1-50% for 'fence width' of wrap_percentage */

/* following are used for wrap_types WRAP_TYPE_FENCE_VALUE & WRAP_TYPE_SET_VALUE */
    double density; /* density */
    double susX;    /* isotropic or anisotropic X susceptibility */
    double susY;    /* anisotropic Y susceptibility */
    double susZ;    /* anisotropic Z susceptibility */

} WRAPPING;


