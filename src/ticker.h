#ifndef __TICKER_H
#define __TICKER_H

// opencv:
#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

typedef long long msec; // время в целочисленных миллисекундах 
typedef double dsec; // время в double-секундах 

class Ticker // замерщик времени
{
  int64 start;
public:
  Ticker() { reset(); }
  int64 reset() { start = cv::getTickCount(); return start; } // старт отсчета 
  int64 ticks() { return cv::getTickCount()-start; } // время после старта отсчета в тиках
  dsec  dsecs() { return dsec(ticks()) / cv::getTickFrequency(); } // время после старта отсчета в секундах
  msec  msecs() { return msec( (1000*ticks()) / cv::getTickFrequency() ); } // время после старта отсчета в миллисекундах
};

#endif // __TICKER_H


//getTickFrequency() сколько тактов в секунду
