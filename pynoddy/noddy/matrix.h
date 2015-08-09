/*
 * =======================
 * NAME:
 *                      matrix.h
 *
 * DESCRIPTION:
 *                      Include file for using the <matrix> module of matrix manipulation
 *                      routines.  The routines are designed to assist in the construction
 *                      of transformation matricies for later application to the points
 *                      from a DXF file.
 *
 * AUTHOR:
 *                      Ewen Christopher
 *                      (c) Ewen Christopher 1994
 *                      (c) Monash University 1994
 * =======================
 */

void identity( float I[4][4] );
void multiplyMatrix( float result[4][4], const float a[4][4],
                                                                                 const float b[4][4] );

void rotateX( float result[4][4], float d );
void rotateY( float result[4][4], float d );
void rotateZ( float result[4][4], float d );

void translate( float result[4][4], float x, float y, float z );
void scale( float result[4][4], float x, float y, float z );
