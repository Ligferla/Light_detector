//lights_tracker.h
#ifndef LIGHTS_TRACKER_H
#define LIGHTS_TRACKER_H

#include "cover_net.h"
#include "euclid_2d.h"
#include "light.h"

#include <map>
#include <queue>
#include <iostream>

class LightsTracker
{
public:

  LightsTracker(){};
  void update( const std::vector< cv::Rect >& frame_lights, const double time );
  void print( cv::Mat& img, const int i );

private:

  void classify( Light& l );//������� ������ � �������� ���������� ����������
  void clear( const int i );//������� i-�� �������� � mTreesOfFrames 
  std::map< int, std::deque< Light > > mStorage;//< ����� ����������, ���������� >
  std::deque< CoverNet< Light*, Euclid2D > > mTreesOfFrames;//������� ��������, ���������� �� ������ 
  std::queue < int > free_tr_numbers;//��������� ������ ����������
  
};

#endif //LIGHTS_TRACKER_H 