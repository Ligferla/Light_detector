//lights_detector.h
#ifndef LIGHTS_DETECTOR_H
#define LIGHTS_DETECTOR_H

#include <cv.h>
#include <highgui.h>
#include <opencv2/opencv.hpp>

#include <vector>
#include <queue>
#include <string>
#include <fstream>
#include <math.h>
#include <algorithm>

#include "cover_net.h"
#include "ticker.h"
#include "light.h"

const int brightness_threshold = 60;//����������� ������� �������� ��� localLightsSearch

class LightsDetector
{
public:

  LightsDetector(){};

  void detect(
    const cv::Mat& frame,         //������� �����������
    const long i_frame,           //����� �����, ������� � 0
    const int search_frequency );  //�������, � ������� ����������� ������ �����
  
  void detect_red(
    const cv::Mat& frame );

  void printLights( cv::Mat& img );  //�����������, �� ������� �������� ����
  void setTime( const double time );
  double getTime();
  const std::vector< cv::Rect >& getLights();
  
private:

  double m_time;
  double m_start_time;
  //��������� ��� �������� ����������� �����
  std::vector< cv::Rect > m_lights_exact_boundary;//������ �������� � ����������� ���������
};

#endif //LIGHTS_DETECTOR_H
