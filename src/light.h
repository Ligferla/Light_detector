//light.h
#ifndef LIGHTS_H
#define LIGHTS_H


#include "opencv2/nonfree/features2d.hpp"


class Light:public cv::KeyPoint

{
public:

  Light( float x, float y, float _size = 0 )
    :cv::KeyPoint( x, y, _size )
  {
    this->class_id = -1;
  };

  Light( float x, float y, float _size, int _class_id, const double time )
    :cv::KeyPoint( x,y, _size )
  {
    this->class_id = _class_id;
    this->m_time = time;
  };

  bool operator == ( const Light& a ) const;
  double getTime();
  void setClassID( const int new_class_id );

private:

  double m_time;//время получения точки

};


#endif //LIGHTS_H
