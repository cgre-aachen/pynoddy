

#ifndef _ddd_h
#define _ddd_h
              

              
#define Fixed     double
#define FPType    double

#define  QDGLOBALS   qd.

#define HITHER       64.0
#define YON       32000.0

typedef struct
{
   Fixed x, y;
} Point2d;

typedef struct
{
   Fixed x, y, z;
} Point3d;

typedef struct
{
   unsigned char
      above  :1,
      below  :1,
      right  :1,
      left   :1,
      behind :1,
      beyond :1,
      b7     :1,
      b8     :1;
} ClipBits;
                    
typedef struct
{
   float hue,        /* Fraction of circle, red at 0 */
         saturation, /* 0 - 1, 0 for gray, 1 for pure color */
         value;      /* 0 - 1, 0 for block, 1 for max intensity */
} HSV_COLOR;                                                       

typedef struct
{
   Fixed v[4];
} Vector;

typedef struct
{
   Fixed v[4][4];
} Matrix;

enum visibility   { isnt_visible, is_visible };

class Point3d
{
private:
  float x, y, z;

public:
   Point3d();  // default constructor
   Point3d( double x, double y, double z );
   Point3d operator+( const Point3d &second );

   Point3d operator+( const Point3d &second );

};

#endif
