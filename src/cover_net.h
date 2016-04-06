// cover_net.h

#ifndef __COVER_NET_H
#define __COVER_NET_H

#include <vector>
#include <algorithm>
#include <cassert>

#include <cv.h>



#define COVER_NET_VERBOSE
//#define COVER_NET_VERBOSE_DETAILED

template < class PointType >
struct CoverSphere
{
  int parent;    // родитель // fat -- не обязательно для основных операций
  int prev_brother;   // индекс предыдущего брата; 0 -- нет
  int last_kid;  // указатель на последнего рожденного ребенка; 0 -- нет

  //... ?
  double distance_to_parent; // расстояние до родительской вершины  // fat?
  //std::vector< int > ancles; // вершины уровнем выше, которые покрывают данную // fat fat slow
  int ancle; // индекс начала списка дядьев --> ancles[ancle](дядья в списке идут в порядке возрастания расстояния?)
  //.....

  PointType center;
  int points;   // количество точек, ассоциированных с данной вершиной // fat
  int level;    // уровень вершины в дереве // fat

  CoverSphere( 
    int level,
    const PointType& center,
    int parent, 
    double distance_to_parent
    ):
  parent( parent ),
    center( center ), 
    level( level ),
    prev_brother( 0 ),
    last_kid( 0 ),
    ancle( -1 ),
    points( 0 ),
    distance_to_parent( distance_to_parent ){} 

  void print( int level );
};



template < class PointType >
void CoverSphere< PointType >::print( int level )
{
  std::string indent;
  for ( int i = 0; i < level; i++ )
    indent.push_back('\t');

  std::cout << indent << center;
  std::cout  << " parent=" << parent << " last_kid=" << last_kid << " prev_brother=" << prev_brother;

  std::cout << std::endl;
}



struct CoverRecord // регистрация рекордного расстояния
{
  double minDistance;
  int sphereIndex;
  int sphereLevel;
};



template < class PointType, class Metrics >
class CoverNet
{
  std::vector< CoverSphere< PointType > > spheres;
  std::vector< std::pair< int, int > > ancles; // <ancle sphere, next> списки вершин уровнем выше, которые покрывают данную 

  Metrics* ruler;
  double rootRadius;  // при создании рута изначально указывается ожидаемое расстояние между наиболее удаленными точками
  // если встречается точка, удаленная от рута на расстояние больше rootRadius дерево надстраивается вверх
  double minRadius;   // если расстояние до центра сферы меньше minRadius вложенные сферы не порождаются
  double maxRadius;   // если расстояние до центра сферы больше maxRadius точка игнорируется

  int iLastSphere;  // номер сферы, к которой была последний раз присоединена точка
  int iLastSphereLevel; // 0 -- рут
  //double lastComputedDistance; // значение последнего вычисленного расстояния

  ////int iRootSphere;  // индекс поддерева, _объявленного_ корнем "селектированного поддерева" 
  ////int iRootLevel;   // уровень корня "селектированного поддерева"
  //// задумка селектированного поддерева состоит в ограничении операции только рамками данного поддерева,
  //// для сокращения числа параметров рекурсивно вызываемых функций 

  int attemptsToInsert; // счетчик числа попыток вставки
  int rejectedInserts; // счетчик числа попыток вставки слишком далеких точек
  double squeezeRatio; // коэффициент уменьшения вложенных окружностей
  std::vector< double > levelsRadii;

public:

  bool isEmpty();//!!дописано 
  double getRadius( int level ) { return levelsRadii[level]; }//!!! levelsRadii пуст???
  double getMinRadius( int level ) { return minRadius; }
  //std::vector< double > levelsMinRadii;


  CoverNet( Metrics* ruler, double rootRadius, double minRadius, 
    double squeezeRatio = 0.5, // коэффициент уменьшения вложенных окружностей
    double maxRadius = -1 );
  
  //CoverNet(const CoverNet& cover_net);//!!!дописано 
  
  bool insert( const PointType& pt ); // false -- лежит за пределами центра нашей вселенной (радиуса рута)
  void clearNet();
 
private:
  enum { SPHERE_CREATED, POINT_ATTACHED };
  void  notifyParents( const PointType& pt, int iSphere, int iSphereLevel, int eventToNote );

  //#define DONT_FORCE_DIRECT_SUCCESSORS
  int makeSphere(             // создает сферу, ее прямых наследников и возвращает ее номер
    int iSphereLevel,         // уровень создаваемой сферы
    const PointType& pt,      // центр сферы
    int parent,               // родительская сфера (гарантируется, что центр лежит внутри родительской и не покрыт братьями
    double distance_to_parent // расстояние до центра родительской сферы
    );
 
  void makeRoot( const PointType& pt )  { makeSphere( 0, pt, -1, 0 ); }

  void insertPoint( 
    const PointType& pt,  // точка внутри сферы iSphere на уровне iSphereLevel
    int iSphere, 
    int iSphereLevel, 
    double dist           // расстояние до центра сферы iSphere (уже измерено)
    );
 
  // лепим непосредственно к данной сфере, не пытаясь свалиться вниз
  void attachPoint( const PointType& pt, int iSphere, int iSphereLevel, double dist );

  // ---- запросы и навигация ----
public:  
  double computeDistance( int iSphere,  const PointType& pt );

  int getSpheresCount() { return int( spheres.size() ); };

  const CoverSphere< PointType >& getSphere( int iSphere ) { return spheres[iSphere]; };

  int countKids( int iSphere ); // подсчитывает количество непосредственных детей у вершины
 

#if 1 // ----------------------------------- not implemented yet 

  int                               // номер сферы, (-1 если за пределами радиуса стартовой), полученный выполняя        
    branchSubTreeUsingFirstCover(   // прямолинейный спуск по поддереву -- первая фаза branch&bounds
    const PointType& pt,            // точка для которой ищем сферу по схеме -- если накрывает -- сразу спускаемся 
                                    // т.е. без попытки поиска более близкой на том же уровне 
    int iStartSphere = 0,           // с какой сферы начинать поиск, 0 - корень дерева 
    int iStartLevel = 0             // уровень стартовой сферы, 0 - уровень кореня дерева 
    //    ,CoverRecord& record      // регистрируем рекорд расстояния?
    ); 
 
public:
  const PointType& 
    findNearestPoint(       // ближайший к указанной точке центр сферы из дерева
    const PointType& pt,    // точка для которой ищем сферу с ближайшим центром
    double &best_distance,   // [in/out] рекорд расстояния -- оно же и отсечение (не искать дальше чем указано)
    int iStartSphere = 0  // с какой сферы начинать поиск, 0 - корень дерева 
    );
 

public:
  // функция, проверяющая корректность построения дерева -- на данный момент тот факт, что все потомки внутри данной сферы
  bool checkCoverNetSphere( int iSphere, int iKidSphere ); 
 
public: 
  bool checkCoverNet();

  int                         // номер сферы, (-1 если за пределами радиуса стартовой)
    findNearestSphere(
    const PointType& pt,      // точка для которой ищем сферу с ближайшим центром
    double& best_distance,    // [in/out] рекорд расстояния -- оно же и отсечение (не искать дальше чем указано)
    double distanceToPt = -1, // расстояние до точки, maxRadius -- если не посчитано
    int iStartSphere = 0      // с какой сферы начинать поиск, 0 - корень дерева 
    );
 

public:
  int                      // номер сферы
    dropToNearestKid(      // рекурсивное "проваливание" в ближайшую детскую сферу. 
                           // на входе -- точка внутри или вне родительской сферы
                           // на выходе -- номер найденной сферы нижнего уровня и расстояние до нее
    int isphere,           // проваливаемся в данную сферу
    const PointType& pt,   // точка для которой ищем сферу с ближайшим центром
    double* best_distance  // [in/out] рекорд расстояния -- оно же и отсечение (не искать дальше чем указано)
    );

#endif // ---------------------- not implemented yet

  void printNode( int node, int level );

  void reportStatistics( int node = 0,  int detailsLevel = 2 ); 
    // 0-none, 1-overall statistics, 2-by levels, 3-print tree hierarchy, 4-print tree array with links
};


//////////////////////////////////////////////////////////////////////
template < class PointType, class Metrics >
CoverNet< PointType, Metrics >::CoverNet( 
  Metrics* ruler,
  double rootRadius,
  double minRadius, 
  double squeezeRatio = 0.5, // коэффициент уменьшения вложенных окружностей
  double maxRadius = -1 )
  :ruler( ruler ),
  rootRadius( rootRadius ),
  minRadius( minRadius ),
  iLastSphere( -1 ),
  iLastSphereLevel( -1 )
{
  if ( maxRadius == -1 )
    maxRadius = rootRadius; 

  double rad = rootRadius;
  for ( int i = 0; i < 256; i++ )
  {
    levelsRadii.push_back( rad );
    rad *= squeezeRatio;
  }

  attemptsToInsert = 0; // счетчик числа попыток вставки
  rejectedInserts = 0;  // счетчик числа попыток вставки слишком далеких точек
};

/*CoverNet(const CoverNet& cover_net)//!!!дописано 
{
  ruler = cover_net.ruler;
  rootRadius = cover_net.rootRadius;
  minRadius = cover_net.minRadius;
  iLastSphere = cover_net.iLastSphere;
  iLastSphereLevel = cover_net.iLastSphereLevel;

  maxRadius = cover_net.maxRadius; 

  for (size_t i = 0; i < cover_net.levelsRadii.size(); i++)
  {
    levelsRadii = cover_net.levelsRadii;
  }

  attemptsToInsert = cover_net.attemptsToInsert; // счетчик числа попыток вставки
  rejectedInserts = cover_net.rejectedInserts; // счетчик числа попыток вставки слишком далеких точек

  //!!!дописать копирование недостающих членов
}*/



template < class PointType, class Metrics >//!!!! 
bool CoverNet< PointType, Metrics >::insert( const PointType& pt ) // false -- лежит за пределами центра нашей вселенной (радиуса рута)
{
  attemptsToInsert++;

  if ( spheres.size() == 0 )
  {
    makeRoot( pt );
    spheres[0].points++;
    return true;
  }

  double dist_root = computeDistance( 0, pt );
  if ( !( dist_root < getRadius(0) ) )
  {
#ifdef COVER_NET_VERBOSE_DETAILED
    std::cout << "Point " << pt << " lies too far, distance to root == " << dist_root << std::endl;
#endif
    rejectedInserts++;
    return false; // игнорируем слишком далекую точку
  }

  ////////#define NO_SEQUENTAL_PROXIMITY_ASSUMPTION
  //////#ifndef NO_SEQUENTAL_PROXIMITY_ASSUMPTION  // если регистрируем пиксели подряд, то соседи вероятно близки
  //////    if (iLastSphere != -1)
  //////    {
  //////      double dist = computeDistance( iLastSphere, pt );
  //////      double rad = getRadius(iLastSphereLevel);
  //////      assert( spheres[iLastSphere].level == iLastSphereLevel );
  //////      if ( dist < rad ) // <<<<<<<<<<<<<<< MUST CHECK PARENT, GRANDPARENT and so on !!!!!
  //////      {
  //////        int start=iLastSphere;
  //////        insertPoint( pt, iLastSphere, iLastSphereLevel, dist ); // изменяет iLastSphere & iLastSphereLevel
  //////        for (int isp = spheres[start].parent; isp >=0; isp = spheres[isp].parent)
  //////          spheres[isp].points++;
  //////        return true;
  //////      }
  //////    }
  //////#endif

  insertPoint( pt, 0, 0, dist_root ); // добавляем спускаясь с рута
  return true;
}



template < class PointType, class Metrics >
void CoverNet< PointType, Metrics >::clearNet()
  {
    //остались неизменными
    ///////////////////////
    /*Metrics* ruler;
    double rootRadius;  // при создании рута изначально указывается ожидаемое расстояние между наиболее удаленными точками
    // если встречается точка, удаленная от рута на расстояние больше rootRadius дерево надстраивается вверх
    double minRadius;   // если расстояние до центра сферы меньше minRadius вложенные сферы не порождаются
    double maxRadius;*/
    ////////////////////////
      squeezeRatio = 0.5; // коэффициент уменьшения вложенных окружностей
      maxRadius = -1;
  
      iLastSphere = -1;
      iLastSphereLevel = -1;

      attemptsToInsert = 0;
      rejectedInserts = 0; 

      spheres.clear();
      ancles.clear();
      //levelsRadii.clear(); не чистим!!
  }



template < class PointType, class Metrics >
void CoverNet< PointType, Metrics >::notifyParents( const PointType& pt,
                                                   int iSphere,
                                                   int iSphereLevel,
                                                   int eventToNote )
{
  switch ( eventToNote ) 
  {
  case SPHERE_CREATED: break;
  case POINT_ATTACHED: break;
  }
}



//#define DONT_FORCE_DIRECT_SUCCESSORS
template < class PointType, class Metrics >
int CoverNet< PointType, Metrics >::makeSphere(    // создает сферу, ее прямых наследников и возвращает ее номер
                         int iSphereLevel,         // уровень создаваемой сферы
                         const PointType& pt,      // центр сферы
                         int parent,               // родительская сфера (гарантируется, что центр лежит внутри родительской и не покрыт братьями
                         double distance_to_parent // расстояние до центра родительской сферы
                         ) 
{
  assert( distance_to_parent == ( (parent >= 0) ? computeDistance( parent, pt ) : distance_to_parent ) );

  if ( iSphereLevel > 0 ) // not root
    assert( computeDistance( parent, pt ) <= getRadius( iSphereLevel-1 ) + 0.0000001 );


  do {
    spheres.push_back( CoverSphere<PointType>( iSphereLevel, pt, parent, distance_to_parent ) );
    iLastSphere = spheres.size() - 1;
    iLastSphereLevel = iSphereLevel;
    if ( parent >= 0 ) // not root
    {
#ifdef DONT_FORCE_DIRECT_SUCCESSORS 
      int bro = spheres[ parent ].last_kid; // поддержка списка детей упорядоченных по расстоянию от родителя???
      spheres[ parent ].last_kid = iLastSphere;
      spheres[ iLastSphere ].prev_brother = bro;
#else
      int bro = spheres[ parent ].last_kid;//последний ребенок
      if ( bro == 0 ) // bro нету
      {
        spheres[ parent ].last_kid = iLastSphere;//тогда iLastSphere -- первый ребенок
        spheres[ iLastSphere ].prev_brother = bro;// = 0;
        /*if ( spheres[ iLastSphere ].center != spheres[ parent ].center )
        cout << "O_O" << endl;
        else
        cout << "+" << endl;*/
      }
      else
      {
        int prev_bro = spheres[ bro ].prev_brother;     //предпоследний ребенок
        spheres[ bro ].prev_brother = iLastSphere;      //предпоследний теперь новый
        spheres[ iLastSphere ].prev_brother = prev_bro; //перед новым стоит prev_bro
      }
#endif
    }
#ifdef DONT_FORCE_DIRECT_SUCCESSORS
    break;
#endif
    iSphereLevel++;
    parent = iLastSphere;
    distance_to_parent = 0;
  } while ( getRadius( iSphereLevel ) > getMinRadius( iSphereLevel ) );

  notifyParents( pt, parent, iSphereLevel, SPHERE_CREATED );
  return iLastSphere;
}



template < class PointType, class Metrics >
void CoverNet< PointType, Metrics >::insertPoint( 
                 const PointType& pt, // точка внутри сферы iSphere на уровне iSphereLevel
                 int iSphere, 
                 int iSphereLevel, 
                 double dist          // расстояние до центра сферы iSphere (уже измерено)
                 )
{ 
  // мы внутри сферы
  assert( computeDistance( iSphere, pt ) <= getRadius( iSphereLevel ) + 0.0000001 );
  spheres[ iSphere ].points++;

  double minrad = getMinRadius( iSphereLevel );
  if ( dist < minrad )
  { // лепим непосредственно к данной сфере
    attachPoint( pt, iSphere, iSphereLevel, dist ); // лепим непосредственно к данной сфере, не пытаясь свалиться вниз
    return;
  }

  // поищем нет ли ребенка
  int kid = spheres[ iSphere ].last_kid;
  double rlast_kid = getRadius( iSphereLevel + 1 );
  while ( kid > 0 )
  {
    double dist_kid = computeDistance( kid, pt );
    if ( dist_kid < rlast_kid )
    {
      insertPoint( pt, kid, iSphereLevel + 1, dist_kid ); // провалились
      return; // конец банкета
    }
    kid = spheres[ kid ].prev_brother;
  }
  // нет детей, место свободно, создаем сферу и, возможно, ее прямых наследников
  makeSphere( iSphereLevel + 1,  pt, iSphere, dist );
}



template < class PointType, class Metrics >
void CoverNet< PointType, Metrics >::attachPoint( const PointType& pt,
                                                 int iSphere,
                                                 int iSphereLevel,
                                                 double dist )
// лепим непосредственно к данной сфере, не пытаясь свалиться вниз
{
#ifdef COVER_NET_VERBOSE_DETAILED
  std::cout << "Point " << pt << " lies near to sphere " << spheres[iSphere].center << " distance == " << dist << std::endl;
#endif
  iLastSphere = iSphere;
  iLastSphereLevel = iSphereLevel;
  // add weight?
  // average radii? other node statistics?
  //spheres[iSphere].sumradii += dist;
  //spheres[iSphere].sumradiisq += dist*dist;

  notifyParents( pt, iSphere, iSphereLevel, POINT_ATTACHED );
}

////////////////////////////////////////////////////////////////////////////////////
//запросы и навигация
template < class PointType, class Metrics >
double CoverNet< PointType, Metrics >::computeDistance( int iSphere,  const PointType& pt )
{
  double dist = ruler->computeDistance( spheres[ iSphere ].center, pt );
  return dist;
}

template < class PointType, class Metrics >
int CoverNet< PointType, Metrics >::countKids( int iSphere ) // подсчитывает количество непосредственных детей у вершины
{ 
  int count=0;
  for (int kid = spheres[ iSphere ].last_kid; kid > 0; kid = spheres[ kid ].prev_brother)
    count++;
  return count; 
};



template < class PointType, class Metrics >
const PointType& 
CoverNet< PointType, Metrics >::findNearestPoint( // ближайший к указанной точке центр сферы из дерева
                 const PointType& pt,             // точка для которой ищем сферу с ближайшим центром
                 double& best_distance,            // [in/out] рекорд расстояния -- оно же и отсечение (не искать дальше чем указано)
                 int iStartSphere = 0           // с какой сферы начинать поиск, 0 - корень дерева 
                 )
{//если количество сфер не нулевое!!!
  //!!здесь
    int iNearestSphere = findNearestSphere( pt, best_distance, -1, iStartSphere );
    if ( iNearestSphere == -1 )
    {
       throw std::invalid_argument( "distance to root is more then maximal radius" );
      //std::cerr << "error: distance to root is more then maximal radius" << std::endl;
    }
    return spheres[ iNearestSphere ].center;    
}



template < class PointType, class Metrics >
bool CoverNet< PointType, Metrics >::checkCoverNetSphere( int iSphere, int iKidSphere ) // функция, проверяющая корректность построения дерева -- на данный момент тот факт, что все потомки внутри данной сферы
{ 
  int isp = iSphere; // текущая сфера
  int lev = spheres[ isp ].level; // текущий уровень
  double  rad = getRadius( lev );
  double dist = computeDistance( isp, spheres[ iKidSphere ].center );
  if ( dist > rad )// если расстояние до потомка больше радиуса
  {
    cout << "build tree error" << endl;
    cout << "incorrect distance " << dist << " from sphere N = " << iSphere 
      << " with center in " << spheres[ isp ].center << " and R = " << rad << " to kid N = " << iKidSphere 
      << " with center in " << spheres[ iKidSphere ].center << endl;
    system ( "pause" );
    return false;
  }
  for (int kid = spheres[ iKidSphere ].last_kid; kid > 0; kid = spheres[ kid ].prev_brother)// идем по всем детям
  {
    int kid_ans = checkCoverNetSphere( iSphere, kid );
    if ( !kid_ans )
      return false;
  }

  return true;    
}



template < class PointType, class Metrics >
int                                                             // номер сферы, (-1 если за пределами радиуса стартовой), полученный выполняя        
CoverNet< PointType, Metrics >::branchSubTreeUsingFirstCover(   // прямолинейный спуск по поддереву -- первая фаза branch&bounds
                             const PointType& pt,               // точка для которой ищем сферу по схеме -- если накрывает -- сразу спускаемся 
                                                                // т.е. без попытки поиска более близкой на том же уровне 
                             int iStartSphere = 0,              // с какой сферы начинать поиск, 0 - корень дерева 
                             int iStartLevel = 0                // уровень стартовой сферы, 0 - уровень кореня дерева 
                             //    ,CoverRecord& record         // регистрируем рекорд расстояния?
                             ) 
{
  double dist = computeDistance( iStartSphere, pt );
  //  record.update( dist, iStartSphere, iStartLevel );
  if ( !( dist < getRadius( iStartLevel ) ) )
    return -1;
  // находимся в покрытом пространстве

  /// .... todo ....
  return 0;
}



template < class PointType, class Metrics >
bool CoverNet< PointType, Metrics >::checkCoverNet()
{
  for ( int i = 0; i < spheres.size(); ++i )
  {
    if ( !checkCoverNetSphere( i, i ) )
      return false;
  }
  return true;
}



template < class PointType, class Metrics >
int                                                // номер сферы, (-1 если за пределами радиуса стартовой)
CoverNet< PointType, Metrics >::findNearestSphere(
                  const PointType& pt,             // точка для которой ищем сферу с ближайшим центром
                  double& best_distance,           // [in/out] рекорд расстояния -- оно же и отсечение (не искать дальше чем указано)
                  double distanceToPt = -1,        // расстояние до точки, maxRadius -- если не посчитано
                  int iStartSphere = 0             // с какой сферы начинать поиск, 0 - корень дерева 
                  )
{
  int isp = iStartSphere;       // текущая сфера
  int lev = spheres[ isp ].level; // текущий уровень
  double  rad = getRadius( lev ); // радиус текущей сферы (на данном уровне)
  double dist = distanceToPt;

  if ( dist == -1 )
    dist = computeDistance( isp, pt );

  if ( isp == 0 && dist >= rad )// если за пределами стартовой
  {
    return -1;
  }

  int ans = -1;
  double min_dist = std::max( 0.0, dist - rad );// расстояние от pt до сферы  
  if ( min_dist > best_distance )// если минимальное расстояние больше оптимального
  {
    return -1;
  }
  if ( dist < best_distance )// если можно улучшить ответ
  {
    ans = isp;
    best_distance = dist;//уменьшается сильно,
  }

  const int MAX_KIDS_SIZE = 256;
  std::pair< double, int > kids[ MAX_KIDS_SIZE ];

  if ( spheres[ isp ].last_kid == 0 ) // лист
    return ans;

  int kid = spheres[ isp ].last_kid;
  int kids_size = 0;
  if ( spheres[ kid ].center == spheres[ isp ].center )//isp - текущая сфера
  {
    kids[ kids_size++ ] = std::make_pair( dist, kid );
    kid = spheres[ kid ].prev_brother;
  }

  /*for (; kid > 0; kid = spheres[kid].prev_brother)// идем по всем детям
  {
    if (fabs(spheres[kid].distance_to_parent - dist) < best_distance + getRadius(lev + 1)) // если по неравенству треугольника, можно улучшить ответ
      kids[kids_size++] = std::make_pair(computeDistance(kid, pt), kid);  // <<<<<<<<<< CHECK kids_size++
  }

  sort(kids + 0, kids + kids_size);
  for (int i = 0; i < kids_size; ++i)
  {
    if (best_distance + getRadius(lev + 1) <= fabs(spheres[kids[i].second].distance_to_parent - dist))// если по неравенству треугольника, нельзя улучшить ответ
      continue;
    int new_ans = findNearestSphere(pt, best_distance, kids[i].first, kids[i].second);
    if (new_ans != -1)
      ans = new_ans;
  }*/
  for ( ; kid > 0; kid = spheres[kid].prev_brother )// идем по всем детям
    kids[kids_size++] = std::make_pair( computeDistance( kid, pt ), kid );

  sort( kids + 0, kids + kids_size );
  for ( int i = 0; i < kids_size; ++i )
  {
    // if (best_distance < spheres[kids[i].second].distance_to_parent + dist)
    //   continue;
    int new_ans = findNearestSphere( pt, best_distance, kids[i].first, kids[i].second );
    if ( new_ans != -1 )
      ans = new_ans;
  }
  return ans;
}



template < class PointType, class Metrics >
int // номер сферы
CoverNet< PointType, Metrics >::dropToNearestKid( // рекурсивное "проваливание" в ближайшую детскую сферу. 
                 // на входе -- точка внутри или вне родительской сферы
                 // на выходе -- номер найденной сферы нижнего уровня и расстояние до нее
                 int isphere, // проваливаемся в данную сферу
                 const PointType& pt, // точка для которой ищем сферу с ближайшим центром
                 double* best_distance // [in/out] рекорд расстояния -- оно же и отсечение (не искать дальше чем указано)
                 )
{
  // пока есть ребенок, в которого попадаем (из нескольких -- ближайший) -- проваливаемся в него
  return 0; // todo
}



template < class PointType, class Metrics >
void CoverNet< PointType, Metrics >::printNode( int node, int level )
{
  spheres[ node ].print( level );
  for ( int kid = spheres[node].last_kid; kid > 0; kid = spheres[kid].prev_brother )
  {
    printNode( kid, level + 1 );
  }
}



template < class PointType, class Metrics >
void CoverNet< PointType, Metrics >::reportStatistics( int node = 0,  int detailsLevel = 2 ) 
// 0-none, 1-overall statistics, 2-by levels, 3-print tree hierarchy, 4-print tree array with links
{
  if ( detailsLevel < 1 )
    return; // set breakpoint here for details

  int maxLevel = -1; // рут на нулевом
  for ( int i = 0; i < int( spheres.size() ); i++ )
    maxLevel = std::max( maxLevel, spheres[i].level );

  std::cout 
    << "********** CoverNet: " << std::endl 
    << "spheres=" << spheres.size() 
    << "\tlevels=" << maxLevel + 1 
    << "\tattemptsToInsert=" << attemptsToInsert // счетчик числа попыток вставки
    << "\trejectedInserts=" << rejectedInserts // счетчик числа попыток вставки слишком далеких точек
    << std::endl;

  if ( detailsLevel < 2 || maxLevel < 0 )
    return; 

  if ( detailsLevel < 3 || maxLevel < 0 )
    return; 

  std::cout 
    << "---------- " << std::endl 
    << "Statistics by level:" << std::endl;
  std::vector< int > spheresByLevel( maxLevel + 1 );
  std::vector< int > pointsByLevel( maxLevel + 1 );
  std::vector< int > kidsByLevel( maxLevel + 1 );

  for ( int i = 0; i < int( spheres.size() ); i++ )
  {
    spheresByLevel[ spheres[i].level ]++;
    pointsByLevel[ spheres[i].level ]+= spheres[i].points;
    kidsByLevel[ spheres[i].level ] += countKids(i);
  }

  for ( int i = 0; i <= maxLevel; i++ )
  {
    std::cout 
      //<< fixed 
      //<< setprecision(5)
      << "tree level=" << i << "\t" 
      << "\tradius=" << getRadius( i ) 
      << "\tspheres=" << spheresByLevel[ i ] 
    << "\tpoints="  << pointsByLevel[ i ]  << "\tave=" << double( pointsByLevel[ i ])/ spheresByLevel[ i ]
    << "\tsum kids="    << kidsByLevel[ i ]    << "\tave=" << double( kidsByLevel[ i ])/ spheresByLevel[ i ]
    << std::endl;
  }

  if ( detailsLevel < 4 )
    return; 

  std::cout 
    << "---------- " << std::endl 
    << "Spheres heirarchy:" << std::endl;
  printNode(0, 0);
  std::cout << "********** " << std::endl;

  if ( detailsLevel < 4 )
    for ( int i = 0; i < int( spheres.size() ); i++ )
      spheres[ i ].print( 0 );

}


template < class PointType, class Metrics >
bool CoverNet< PointType, Metrics >::isEmpty( )
{ 
  if ( iLastSphere < 0 )
  {
    return true;
  }
  return false; 
};

#endif // __COVER_NET_H