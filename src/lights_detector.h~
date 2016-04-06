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

const int brightness_threshold = 60;//минимальная разница яркостей для localLightsSearch

class LightsDetector
{
public:

  LightsDetector(){};

  void detect(
    const cv::Mat& frame,         //входное изображение
    const long i_frame,           //номер кадра, начиная с 0
    const int search_frequency );  //частота, с которой повторяется полный поиск
  
  void detect_red(
    const cv::Mat& frame );

  void printLights( cv::Mat& img );  //изображение, на котором рисуются огни
  void setTime( const double time );
  double getTime();
  const std::vector< cv::Rect >& getLights();
  
private:

  double m_time;
  double m_start_time;
  //использую для хранения предыдущего кадра
  std::vector< cv::Rect > m_lights_exact_boundary;//вектор огоньков с уточненными границами
};

#endif //LIGHTS_DETECTOR_H
