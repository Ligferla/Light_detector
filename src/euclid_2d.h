//euclid_2d.h
#ifndef EUCLID_2D_H
#define EUCLID_2D_H

#include "light.h"

class Euclid2D
{
public:
  double computeDistance( const Light* a, const Light* b )
  {
    double c_x_a = a->pt.x + a->size/2;
    double c_y_a = a->pt.y + a->size/2;
    double c_x_b = b->pt.x + b->size/2;
    double c_y_b = b->pt.y + b->size/2;

    return sqrt ( double( c_x_a - c_x_b )*( c_x_a - c_x_b ) + ( c_y_a - c_y_b )*( c_y_a - c_y_b ) );
  }
};

#endif //EUCLID_2D_H
