//lights_tracker.cpp
#include <stdexcept>

#include "lights_tracker.h"

const int frames_count = 10;//���������� ����������� ������(������� ����� ��������� ��������)

//!!�������� �������� �� ������ ����������, ������� ����� ������� � ����������� �����(�������� � free_tr_numbers)
void LightsTracker::update( const std::vector< cv::Rect >& frame_lights, const double time )//���� � ����� ������� time
{
  //�������� � ������, ���������� ����� � ������ (� ������ ������ ��������� ����� ����������!!!!)
  Euclid2D ruler;
  CoverNet< Light*, Euclid2D > coverTree( &ruler, 3000, 10 );//�������, ������� ������, ����������� ������
  std::map< int, std::deque< Light > >::iterator it;

  int NewClassID = -1;
  for( size_t i = 0; i < frame_lights.size(); i++ )
  {
    Light l( frame_lights[ i ].x, frame_lights[ i ].y,
      std::max(frame_lights[ i ].width, frame_lights[ i ].height ), -1, time );
    classify( l );
    NewClassID = l.class_id;
    if( NewClassID >= 0 )//���������� ����� - ���������� �����
    {
      it =  mStorage.find( NewClassID );
      //��������� � ��������� �� ����� ������ ���������� (#���������� ���������� �� 1 ������ �������)
      it->second.push_back( l );

      //��������� � ������
      coverTree.insert( &( it->second.back() ) );
    }
    else//����� ������� ������� � �� ��������� �� � ����� �� ������������ ����������
    {
      if( !free_tr_numbers.empty() )//���� ���� ������ �������������� ���������� - ���������� ��
      {
        NewClassID = free_tr_numbers.front();
        free_tr_numbers.pop();
      }
      else//���� ��� - ����� ����� �� �������(��������� � 1 �� mStorage.size())
      {
        NewClassID = mStorage.size() + 1;
      }
      l.setClassID( NewClassID );
      std::deque< Light > light_track;
      light_track.push_back( l );
      mStorage.insert( std::pair< int, std::deque< Light > >( NewClassID, light_track ));//���� - ����� ����������, �������� - ����������
      it = mStorage.find( NewClassID );//!!!��������, ����� �������� (�������, � ����� ��� � �����)

      //���������� ������� ��������     
      coverTree.insert( &( it->second.back() ) );//� ������ �������� ������ �� ����� 
    }
  }
  mTreesOfFrames.push_back( coverTree );

  //������� �� ������ ����������
  const int size = mTreesOfFrames.size();
  if( size > frames_count )//������ ��������� ��������
  {
    clear( 0 );//0� - ����� ������ ������
  }
}



void LightsTracker::print( cv::Mat& img, const int i )
{
  //�������� �� �������������� ����������
  std::map< int, std::deque< Light > >::iterator it;
  
  if( ( it = mStorage.find(i) ) != mStorage.end() )
  {
    int size = it->second.size();

    cv::Scalar color( 0, 0, 255 );
    for( size_t j = 0; j < it->second.size(); j++ )
    {
      if( j != 0 )
      {
        //cv::line( img,
        //  cv::Point( it->second[ j ].pt.x , it->second[ j ].pt.y ),
        //  cv::Point( it->second[ j - 1 ].pt.x , it->second[ j - 1 ].pt.y ),
        //  color, 1 );
      }
      //cv::rectangle(img, 
      //  cv::Point( it->second[ j ].pt.x, it->second[ j ].pt.y ),
      //  cv::Point( it->second[ j ].pt.x, it->second[ j ].pt.y), color, 4 );
    }
  }  
  else
  {
    std::cerr << "no such number trajectory\n";
  }
}

void LightsTracker::classify( Light& l )//!!!����� ���������� �������� ������ �� ��� �������� � Light � classify, ����� ����� ���� �������� � findNearestPoint?
{
  int NewClassID = -1;
  for( int j = mTreesOfFrames.size() - 1; j >= 0; j-- )//�� size_t �� ��������
  {
    try
    {
      double dist = 20;//��������� ����� findNearestPoint ������ �����!!
      Light* nearest_to_l = mTreesOfFrames[ j ].findNearestPoint( &l, dist );//������� �� ��������(3 ���������)
      //std::cout << l.pt.x << " " << l.pt.y << " " << nearest_to_l->pt.x << " " << nearest_to_l->pt.y << "\n";

      NewClassID = nearest_to_l->class_id;
      l.setClassID( NewClassID );
      break;
    }
    catch( std::invalid_argument& e )
    { 
      std::cerr << e.what() << std::endl;//�������� ������ ������ ���� (-1)
    }
  }
}

void LightsTracker::clear( const int i )//����� ���������� �������� � mTreesOfFrames
{
  if( ( 0 <= i ) && ( i < mTreesOfFrames.size() ) )//���� ������� � ����� ������� ����������
  {
    if( !mTreesOfFrames[ i ].isEmpty() )//���� ������ �� ������
    {
      const double time = mTreesOfFrames[ i ].getSphere( 0 ).center->getTime();//����� �����, ��� �������� ������ ���������
      
      std::map< int, std::deque< Light > >::iterator it;

      std::vector< std::map< int, std::deque< Light > >::iterator > empty_tracks_its;
      for ( it = mStorage.begin(); it != mStorage.end(); ++it )//�� ���� �����������
      {
        int j = 0;
        while( ( j < it->second.size() ) && ( time == it->second[ j ].getTime() ) )//�� ������� �� ������� ���������� � ����� �������� "������"
        {
          it->second.pop_front();
        }

        if( 0 == it->second.size() )//���������� �����
        {
          empty_tracks_its.push_back( it );//���������� ������
        }
      }
      //������� ������ ����������
      int j = 0;   
      while( j < empty_tracks_its.size() )
      {
        it = empty_tracks_its[ j ];
        free_tr_numbers.push( it->first );//�������� ����� ���������� ��� ���������
        mStorage.erase( it );
        j++;
      }
      mTreesOfFrames.erase( mTreesOfFrames.begin() + i );
    }
  }
  else
  {
    std::cerr << "can't remove a non-existent element" << std::endl;
  }
} 
