//lights_detector.cpp
#include "lights_detector.h"

int imgSum( const cv::Mat& sum, const cv::Rect& rc );

void mergeObjects( const std::vector<cv::Rect>& objects, 
                  const int delta,
                  std::vector< cv::Rect >& merged_objects_pairs );

void saveTrajectoryRect( std::queue< cv::Rect >& trajectory_rect,
                        const std::vector< cv::Rect >& lights,
                        std::queue< int >& count_frame_lights );

void localLightsSearch( const cv::Mat& sum,              //������� �����������
                       const cv::Rect& search_window,   //�������������, � ������� ���������� �����
                       std::vector< cv::Rect >& lights, //������ �������� (�� ���������� �������) ���� ������ ������� � ���
                       const int min_size,              //����������� ������ �������
                       const int max_size );              //������������ ������ �������

void lazyLightsSearch( const cv::Mat& sum,                     //������� �����������
                      const std::vector< cv::Rect >& prev_lights,
                      std::vector< cv::Rect >& lights,        //������ �������� (�� ���������� �������)
                      const int min_size,
                      const int max_size );

///////////////////////////////////////////////////////////////////////////////////


void LightsDetector::detect( const cv::Mat& frame,       //�����������
                            const long i_frame,         //����� �������� �����(��� �������/���������� ������)
                            const int search_frequency ) //������� ������ (� ����� �������� ������ �����)
{
  cv::Mat gray;
  cv::cvtColor( frame, gray, CV_BGR2GRAY );//�������������� ��������� ����������� � �����

  cv::Mat sum;
  cv::integral( gray, sum );// ������� sum �� gray

  std::vector< cv::Rect > lights;//������ �������������� ��������, �� ������� � ���������� ����� ������������� ��������

  if( 0 == i_frame % search_frequency )//������ m_search_frequency ���� ��������� ��������� �� ������� ��������
  {
    //�������������, � ������� ���������� �����
    cv::Rect search_window( 0, 0, gray.cols, gray.rows );//������ �� x, ������ �� �, ������, ������

    Ticker t;
    t.reset();

    //(�����������, ������������� ������,
    //������, ��� �������� "����������" �� ����,
    //����������� ������ ����, ������������ ������ ����)
    localLightsSearch( sum, search_window, lights, 2, 6 );//�� t

    /*std::cout << "ticks = " << t.ticks() 
      << " sec = " << t.dsecs()
      << " msec = " << t.msecs() << "\n";*/
  }
  else//������� ����� � ���������� ������ ��������� ��������
  {
    Ticker t;
    t.reset();
    //(�����������, ������� � ����������� ����, ���, ��� �������)
    lazyLightsSearch( sum, m_lights_exact_boundary, lights, 3, 30 );//
    /*std::cout << "local " << "ticks = " << t.ticks() 
      << " sec = " << t.dsecs()
      << " msec = " << t.msecs() << "\n";*/
  }

  m_lights_exact_boundary.clear();
  mergeObjects( lights, 20, m_lights_exact_boundary );//20 - delta,� m_lights_exact_boundary ���� �������� �����

  //���������� ���������� ��������(��� ���������� ����������)
  //(����������, ������� � �������� �����, ���������� �������� � ������ ����)
  //saveTrajectoryRect(m_trajectory_rect, m_lights_exact_boundary, m_frame_lights_count);
}

void LightsDetector::detect_red( const cv::Mat& frame )
{

  imshow( "Orginal", frame );
  cv::waitKey( 0 );

  std::vector<cv::Mat> rgb;
  split( frame, rgb );


  cv::imshow( "b", rgb[0] );
  cv::waitKey( 0 );

  cv::imshow( "g", rgb[1] );
  cv::waitKey( 0 );

  cv::imshow( "r", rgb[2] );
  cv::waitKey( 0 );

  cv::imshow( "diff", 2* rgb[2] - rgb[0] );
  cv::waitKey( 0 );

  //�������� 2 �������
  //�����������
  //???����������� �� ���������(������ ������ �� ������, ����� ������ ����)
  //���������������� ������
  //�������� �� ��������(�.�. � ������ ������������)
  //���������, ����������� � ����
  //��������� � ���������
}

void LightsDetector::printLights( cv::Mat& img )
{
  for( size_t i = 0; i < m_lights_exact_boundary.size(); i++ )
  {
    cv::Rect current_rect = m_lights_exact_boundary[i]; 
    /*cv::rectangle(img,
      cv::Point(current_rect.x + current_rect.width/2, current_rect.y + current_rect.height/2),
      cv::Point(current_rect.x + current_rect.width/2, current_rect.y + current_rect.height/2),
      cv::Scalar(0, 255, 255), 3, 8, 0);//������ ������*/
    cv::rectangle( img,
      cv::Point( current_rect.x, current_rect.y ),
      cv::Point( current_rect.x + current_rect.width, current_rect.y + current_rect.height ),
      cv::Scalar( 0, 255, 0 ), 2, 8, 0 );//������� ������
  }
}
/**
��������� ��������� ��������������� �������� �� img, ���� ������� �� ������ �������
*/
/*void LightsDetector::printLights(cv::Mat& img)
{
  std::queue< cv::Rect > trajectory_rect(m_trajectory_rect);//������� �������� � ����������� ��������� �� ���� ������
  std::queue< int > frame_lights_count(m_frame_lights_count);//������� �������� ��������� ������(�������� ���� ��������)

  cv::Rect current_rect;
  int original_frame_count = frame_lights_count.size();
  for(int j = 0; j < original_frame_count - 1; j++)//��� ���� ��� ����������
  {
    for(int i = 0; i < frame_lights_count.front(); i++)//���������� �������� � ������ ����
    {
      current_rect = trajectory_rect.front();
      trajectory_rect.pop();
      //����� ����������
      cv::rectangle(img,
        cv::Point(current_rect.x + current_rect.width/2, current_rect.y + current_rect.height/2),
        cv::Point(current_rect.x + current_rect.width/2, current_rect.y + current_rect.height/2),
        cv::Scalar(0, 255, 255), 3, 8, 0);//������ ������
    }
    frame_lights_count.pop();
  }

  for(int i = 0; i < frame_lights_count.front(); i++)
  {
    current_rect = trajectory_rect.front();
    trajectory_rect.pop();
    //�������������� �������� �� ���������� ���� 
    cv::rectangle(img,
      cv::Point(current_rect.x, current_rect.y),
      cv::Point(current_rect.x + current_rect.width, current_rect.y + current_rect.height),
      cv::Scalar(0, 255, 0), 2, 8, 0);//������� ������
  }
}*/

void LightsDetector::setTime( const double time )
{
  if(m_time == 0)
  {
    m_start_time = time;
  }
  m_time = time;
}

double LightsDetector::getTime()
{
  return m_time;
}

const std::vector< cv::Rect >& LightsDetector::getLights()
{
  return m_lights_exact_boundary;
}

//////////////////////////////////////////////////////////////////////////
//��������������� �������
/////////////////////////////////////////////////////////////////////////

/**
������� ����������� �������� �����
*/
double imgSqrSum( const cv::Mat& sum, const cv::Rect& rc )
{
  double value = sum.at< double >( rc.y, rc.x )
    + sum.at< double >( rc.y + rc.height, rc.x + rc.width )
    - sum.at< double >( rc.y + rc.height, rc.x )
    - sum.at< double >( rc.y, rc.x + rc.height ); //y, x
  value /= rc.height * rc.width; //����������
  return value;
}

/**
������� ����������� �����
*/
int imgSum( const cv::Mat& sum, const cv::Rect& rc )
{
  int value = sum.at< int >( rc.y, rc.x )
    + sum.at< int >( rc.y + rc.height, rc.x + rc.width )
    - sum.at< int >( rc.y + rc.height, rc.x )
    - sum.at< int >( rc.y, rc.x + rc.height ); //y, x
  value /= rc.height * rc.width; //����������
  return value;
}

/**
��������� ���������� ����� � ������ ���������������
��� ���������� � ������� mergeObjects
*/
static void addRectangleInSum( std::pair<cv::Point, cv::Point>& sum, const cv::Rect& r )
{
  sum.first.x += r.x;
  sum.first.y += r.y;
  sum.second.x += ( r.x + r.width );
  sum.second.y += ( r.y + r.height );
}

/**
��������� ������ ��������
*/
void mergeObjects( const std::vector< cv::Rect >& objects, 
                  const int delta,
                  std::vector< cv::Rect >& merged_objects_pairs )//������� ��������
{
  //������� ����� ����������
  std::vector< int > merged_objects_count;
  //����� ��������� ��� �������� ��������������� ������� ��� ������� ��������(����� �������, ������ ������)
  std::vector< std::pair< cv::Point, cv::Point > > sum;

  for ( size_t i = 0; i < objects.size(); i++ )
  {
    bool is_merged = false;
    const cv::Rect& object = objects[ i ];
    //������� ����� �������
    const cv::Point object_center( object.x + object.width/2, object.y + object.height/2 );
    //�� ���� ������������ �������� 
    for ( size_t j = 0; j < merged_objects_pairs.size(); j++ )
    {
      const cv::Rect& merged_object = merged_objects_pairs[ j ];//����� ������������� �� ����
      const cv::Point merged_object_center( merged_object.x + merged_object.width/2, //������� ��� �����
        merged_object.y + merged_object.height/2 );

      //�������� ���������� ���������� ����� ������� ��� ������������ ����� � ������� ������ �������,
      //���� �� ������ ��������� ��������� delta
      if ( std::max( abs(merged_object_center.x - object_center.x ), abs( merged_object_center.y - object_center.y ) ) < delta )
      {
        merged_objects_pairs[ j ] = merged_object | object;//������ ����������� �� ���������������
        addRectangleInSum(sum[ j ], objects[ i ]);
        merged_objects_count[ j ] += 1;//���������� ������������ ��������
        is_merged = true;
       	break;
      }
    }

    if ( is_merged == false )//���� ����������� �� ����(������ ����������� � ��� �� �������������� ��������)
    {
      merged_objects_pairs.push_back( objects[ i ] );//������� ���
      merged_objects_count.push_back( 1 );
      sum.push_back( std::make_pair( cv::Point( 0, 0 ), cv::Point( 0, 0 ) ) );
      addRectangleInSum( sum[ sum.size() - 1 ], objects[ i ] );
    }
  }

  for ( size_t i = 0; i < merged_objects_pairs.size(); ++i )
  {
    merged_objects_pairs[ i ].x = sum[ i ].first.x / merged_objects_count[ i ];//����� �������� �� x  //����� �������
    merged_objects_pairs[ i ].y = sum[ i ].first.y / merged_objects_count[ i ];
    merged_objects_pairs[ i ].width = sum[ i ].second.x / merged_objects_count[ i ]//������ �� ������ ��������
    - merged_objects_pairs[ i ].x;
    merged_objects_pairs[ i ].height = sum[ i ].second.y / merged_objects_count[ i ]//������ � ������ ��������
    - merged_objects_pairs[ i ].y; 
  }
}

/**
��������� <= (frame_count + 1)��������� ������    (������ � ��� ������, ���� ��� ������� ������ �� ���������)
*/
void saveTrajectoryRect( std::queue< cv::Rect >& trajectory_rect, //������ ���������� ��������
                        const std::vector< cv::Rect >& lights,   //������� � �������� �����
                        std::queue< int >& count_frame_lights )   //���������� �������� � ������ ����, ������ ��������� frame_count �����
{
  //���� ����� ������, ��� ������ ���� ���������� �����
  const int frame_count = 6;
  if( count_frame_lights.size() > frame_count )//�� ���������  frame_count �.�. ������ ��������� ��� ����
  {
    for( int i = 0; i < count_frame_lights.front(); i++ )
    {
      trajectory_rect.pop();//�������� ���� �� ������� ���������
    }
    count_frame_lights.pop();//������� ������ ���������� ��������
  }
  count_frame_lights.push( lights.size() );
  for( size_t i = 0; i < lights.size(); i++ )
  {
    trajectory_rect.push( lights[ i ] );
  }
}

/**
������� ���������� �������������� "��������" ��� � ��������������
search_window
*/ 
void localLightsSearch( const cv::Mat& sum,              //������� �����������
                       const cv::Rect& search_window,   //�������������, � ������� ���������� �����
                       std::vector< cv::Rect >& lights, //������ �������� (�� ���������� �������) ���� ������ ������� � ���
                       const int min_size,              //����������� ������ �������
                       const int max_size )              //������������ ������ �������
{
  //�������� �� ������������ ����������� �������������� ������ 
  cv::Point start( search_window.x, search_window.y );
  cv::Point finish( search_window.x + search_window.width, search_window.y + search_window.height );
  if( start.x < 0 )
  {
    start.x = 0; //���� ������� �� ����, ����� ����
  }
  if( start.y < 0 )
  {
    start.y = 0;
  }
  if( finish.x > sum.cols - 2 )// (sum - 1) - ������ ��������� �����������, -1 - ������ ���������� 
  {
    finish.x = sum.cols - 2;
  }
  if( finish.y > sum.rows - 2 )
  {
    finish.y = sum.rows - 2;
  }  

  for( int target = min_size; target < max_size; target++ )//��������� ������� ���� ������
  {
    for( int y = start.y; y < finish.y - 2*target; y++ ) //height //!!! finish.y - 2*target!!!!
    {
      for( int x = start.x; x < finish.x - 2*target; x++ ) //width
      {
        cv::Rect rc_internal( x, y, target, target ); //width, height
        int img_internal = imgSum( sum, rc_internal );

        cv::Rect rc_external( x, y, 2*target, 2*target );///!!!��������� �� ������� �� �����!!!
        int img_external = imgSum( sum, rc_external );

        // -> max
        double difference = img_internal - img_external;

        if( difference > brightness_threshold )
        {
          lights.push_back( cv::Rect( x, y, target, target ) );//���������� ��������
        }
      }
    }
  }
}

/**
������� "��������" �������������� "��������" ���
����� ���������� ������ � ����������� ������� ��������������� ��  prev_lights                                                    
*/ 
void lazyLightsSearch( const cv::Mat& sum,                     //������� �����������
                      const std::vector< cv::Rect >& prev_lights,
                      std::vector< cv::Rect >& lights,        //������ �������� (�� ���������� �������)
                      const int min_size,
                      const int max_size )
{ 
  int neighborhood_width = 50;//�������� ������/����� �������������� � ������� ����� ���� �����

  for ( size_t i = 0; i < prev_lights.size(); i++ )
  {
    const cv::Rect search_window(
      prev_lights[i].x + prev_lights[i].width/2 - neighborhood_width,
      prev_lights[i].y + prev_lights[i].height/2 - neighborhood_width,
      2*neighborhood_width,
      2*neighborhood_width );
    localLightsSearch( sum, search_window, lights, min_size, max_size );
  }
}