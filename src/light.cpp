//light.cpp
#include "light.h"

bool Light::operator == ( const Light& b ) const
{
  //!!!дописать условия, когда будут использоваться новые параметры
  if ( ( this->pt == b.pt ) && ( this->size == b.size ) && ( this->class_id == b.class_id )
    && ( this->m_time == b.m_time ) )
  {
    return true;
  }
  return false;
}

double Light::getTime()
{
  return this->m_time;
}

void Light::setClassID( const int new_class_id )
{
  this->class_id = new_class_id;
}