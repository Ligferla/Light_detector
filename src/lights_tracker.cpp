//lights_tracker.cpp
#include <stdexcept>

#include "lights_tracker.h"

const int frames_count = 10;//количество сохраненных кадров(столько будет сохранено деревьев)

//!!дописать проверку на пустые траектории, которые нужно удалять и освобождать номер(помещать в free_tr_numbers)
void LightsTracker::update( const std::vector< cv::Rect >& frame_lights, const double time )//огни с кадра времени time
{
  //помещаем в дерево, запоминаем место в дереве (в дереве должен храниться номер траектории!!!!)
  Euclid2D ruler;
  CoverNet< Light*, Euclid2D > coverTree( &ruler, 3000, 10 );//метрика, рутовый радиус, минимальний радиус
  std::map< int, std::deque< Light > >::iterator it;

  int NewClassID = -1;
  for( size_t i = 0; i < frame_lights.size(); i++ )
  {
    Light l( frame_lights[ i ].x, frame_lights[ i ].y,
      std::max(frame_lights[ i ].width, frame_lights[ i ].height ), -1, time );
    classify( l );
    NewClassID = l.class_id;
    if( NewClassID >= 0 )//определили класс - траекторию точки
    {
      it =  mStorage.find( NewClassID );
      //вставляем в хранилище на место нужной траектории (#траектории отличается на 1 больше инднкса)
      it->second.push_back( l );

      //вставляем в дерево
      coverTree.insert( &( it->second.back() ) );
    }
    else//точка слишком далекая и не относится ни к одной из предложенных траекторий
    {
      if( !free_tr_numbers.empty() )//если есть номера освободившихся траекторий - используем их
      {
        NewClassID = free_tr_numbers.front();
        free_tr_numbers.pop();
      }
      else//если нет - берем номер по порядку(нумерация с 1 до mStorage.size())
      {
        NewClassID = mStorage.size() + 1;
      }
      l.setClassID( NewClassID );
      std::deque< Light > light_track;
      light_track.push_back( l );
      mStorage.insert( std::pair< int, std::deque< Light > >( NewClassID, light_track ));//ключ - номер траектории, значение - траектория
      it = mStorage.find( NewClassID );//!!!возможно, можно улучшить (вставка, а потом еще и поиск)

      //заполнение очереди деревьев     
      coverTree.insert( &( it->second.back() ) );//в дереве хранятся ссылки на точки 
    }
  }
  mTreesOfFrames.push_back( coverTree );

  //очистка от старой информации
  const int size = mTreesOfFrames.size();
  if( size > frames_count )//размер превышает заданный
  {
    clear( 0 );//0е - самое старое дерево
  }
}



void LightsTracker::print( cv::Mat& img, const int i )
{
  //проверка на существоввание траектории
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

void LightsTracker::classify( Light& l )//!!!лучше возвращать значение класса но как передать в Light в classify, чтобы можно было передать в findNearestPoint?
{
  int NewClassID = -1;
  for( int j = mTreesOfFrames.size() - 1; j >= 0; j-- )//на size_t не заменять
  {
    try
    {
      double dist = 20;//объявлять перед findNearestPoint внутри цикла!!
      Light* nearest_to_l = mTreesOfFrames[ j ].findNearestPoint( &l, dist );//перебор по деревьям(3 последних)
      //std::cout << l.pt.x << " " << l.pt.y << " " << nearest_to_l->pt.x << " " << nearest_to_l->pt.y << "\n";

      NewClassID = nearest_to_l->class_id;
      l.setClassID( NewClassID );
      break;
    }
    catch( std::invalid_argument& e )
    { 
      std::cerr << e.what() << std::endl;//значение класса должно быть (-1)
    }
  }
}

void LightsTracker::clear( const int i )//номер удаляемого элемента в mTreesOfFrames
{
  if( ( 0 <= i ) && ( i < mTreesOfFrames.size() ) )//если элемент с таким номером существует
  {
    if( !mTreesOfFrames[ i ].isEmpty() )//если дерево не пустое
    {
      const double time = mTreesOfFrames[ i ].getSphere( 0 ).center->getTime();//время кадра, для которого дерево построено
      
      std::map< int, std::deque< Light > >::iterator it;

      std::vector< std::map< int, std::deque< Light > >::iterator > empty_tracks_its;
      for ( it = mStorage.begin(); it != mStorage.end(); ++it )//по всем траекториям
      {
        int j = 0;
        while( ( j < it->second.size() ) && ( time == it->second[ j ].getTime() ) )//не выходит за границу траектории и точка является "старой"
        {
          it->second.pop_front();
        }

        if( 0 == it->second.size() )//траектория пуста
        {
          empty_tracks_its.push_back( it );//запоминаем пустые
        }
      }
      //убираем пустые траектории
      int j = 0;   
      while( j < empty_tracks_its.size() )
      {
        it = empty_tracks_its[ j ];
        free_tr_numbers.push( it->first );//помечаем номер траектории как свободный
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
