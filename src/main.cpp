//main.cpp
//14.04.14

#include "cover_net.h"
#include "euclid_2d.h"
#include "light.h"
#include "lights_detector.h"
#include "lights_tracker.h"

int main( int argc, char* argv[] )
{
  //std::string file_name = "";
  std::string file_name = "/../../../testdata/roadvideo/input/roadvideo.05/roadvideo.05.0030.avi.960.avi";
  try
  {
    cv::VideoCapture cap;
    cap.open( file_name );

    /*if ( argc == 2 )
    {
      cap.open(argv[1]);
    }*/
    if ( !cap.isOpened() )
    {
      throw std::runtime_error( "Unable to open VideoCapture" );
    }
 
    //cap.set( CV_CAP_PROP_POS_MSEC, 197280 );//  промотка видео

    const int n_frames = 40000;//максимальное количестов кадров, которое можно просмотреть
    int i_frame = 0;//номер текущего кадра

    LightsDetector detector;//частота с которой делается полный поиск
    LightsTracker tracker;

    while( i_frame < n_frames )
    {
      cv::Mat img;
      if ( !cap.read( img ) )
        break;

      if ( img.empty() )
      {
        std::cout << "Can't load image" << file_name << std::endl;
        return -1;
      }

      detector.setTime( cap.get( CV_CAP_PROP_POS_MSEC ) );// Current position of the video file in milliseconds or video capture timestamp.

      //std::cout << " FPS " << cap.get(CV_CAP_PROP_FPS);
      //std::cout << cap.get(CV_CAP_PROP_POS_MSEC) << "\n";

      cv::resize( img, img, cv::Size(), 0.5, 0.5 );
      //detector.detect( img, i_frame, 60 );//изображение, номер кадра, частота полного поиска
      //detector.printLights( img );
      detector.detect_red( img );
     
     
      //tracker.update( detector.getLights(), detector.getTime() );//прямоугольники огоньков, время кадра в видеопотоке
      //tracker.print( img, 4 );

      //отрисовка огоньков на кадре

      //cv::imshow( "lights", img );
      //cv::waitKey( 0 );

      std::cout << "frame number " << i_frame << ":\n";  
      i_frame++;
    }
  }
  catch ( std::exception const& e )
  {
    fprintf( stderr, "Exception: %s\n", e.what() );
    return 1;
  }
  return 0;
}
