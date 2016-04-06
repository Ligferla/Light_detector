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
  int parent;    // �������� // fat -- �� ����������� ��� �������� ��������
  int prev_brother;   // ������ ����������� �����; 0 -- ���
  int last_kid;  // ��������� �� ���������� ���������� �������; 0 -- ���

  //... ?
  double distance_to_parent; // ���������� �� ������������ �������  // fat?
  //std::vector< int > ancles; // ������� ������� ����, ������� ��������� ������ // fat fat slow
  int ancle; // ������ ������ ������ ������ --> ancles[ancle](����� � ������ ���� � ������� ����������� ����������?)
  //.....

  PointType center;
  int points;   // ���������� �����, ��������������� � ������ �������� // fat
  int level;    // ������� ������� � ������ // fat

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



struct CoverRecord // ����������� ���������� ����������
{
  double minDistance;
  int sphereIndex;
  int sphereLevel;
};



template < class PointType, class Metrics >
class CoverNet
{
  std::vector< CoverSphere< PointType > > spheres;
  std::vector< std::pair< int, int > > ancles; // <ancle sphere, next> ������ ������ ������� ����, ������� ��������� ������ 

  Metrics* ruler;
  double rootRadius;  // ��� �������� ���� ���������� ����������� ��������� ���������� ����� �������� ���������� �������
  // ���� ����������� �����, ��������� �� ���� �� ���������� ������ rootRadius ������ �������������� �����
  double minRadius;   // ���� ���������� �� ������ ����� ������ minRadius ��������� ����� �� �����������
  double maxRadius;   // ���� ���������� �� ������ ����� ������ maxRadius ����� ������������

  int iLastSphere;  // ����� �����, � ������� ���� ��������� ��� ������������ �����
  int iLastSphereLevel; // 0 -- ���
  //double lastComputedDistance; // �������� ���������� ������������ ����������

  ////int iRootSphere;  // ������ ���������, _������������_ ������ "���������������� ���������" 
  ////int iRootLevel;   // ������� ����� "���������������� ���������"
  //// ������� ���������������� ��������� ������� � ����������� �������� ������ ������� ������� ���������,
  //// ��� ���������� ����� ���������� ���������� ���������� ������� 

  int attemptsToInsert; // ������� ����� ������� �������
  int rejectedInserts; // ������� ����� ������� ������� ������� ������� �����
  double squeezeRatio; // ����������� ���������� ��������� �����������
  std::vector< double > levelsRadii;

public:

  bool isEmpty();//!!�������� 
  double getRadius( int level ) { return levelsRadii[level]; }//!!! levelsRadii ����???
  double getMinRadius( int level ) { return minRadius; }
  //std::vector< double > levelsMinRadii;


  CoverNet( Metrics* ruler, double rootRadius, double minRadius, 
    double squeezeRatio = 0.5, // ����������� ���������� ��������� �����������
    double maxRadius = -1 );
  
  //CoverNet(const CoverNet& cover_net);//!!!�������� 
  
  bool insert( const PointType& pt ); // false -- ����� �� ��������� ������ ����� ��������� (������� ����)
  void clearNet();
 
private:
  enum { SPHERE_CREATED, POINT_ATTACHED };
  void  notifyParents( const PointType& pt, int iSphere, int iSphereLevel, int eventToNote );

  //#define DONT_FORCE_DIRECT_SUCCESSORS
  int makeSphere(             // ������� �����, �� ������ ����������� � ���������� �� �����
    int iSphereLevel,         // ������� ����������� �����
    const PointType& pt,      // ����� �����
    int parent,               // ������������ ����� (�������������, ��� ����� ����� ������ ������������ � �� ������ ��������
    double distance_to_parent // ���������� �� ������ ������������ �����
    );
 
  void makeRoot( const PointType& pt )  { makeSphere( 0, pt, -1, 0 ); }

  void insertPoint( 
    const PointType& pt,  // ����� ������ ����� iSphere �� ������ iSphereLevel
    int iSphere, 
    int iSphereLevel, 
    double dist           // ���������� �� ������ ����� iSphere (��� ��������)
    );
 
  // ����� ��������������� � ������ �����, �� ������� ��������� ����
  void attachPoint( const PointType& pt, int iSphere, int iSphereLevel, double dist );

  // ---- ������� � ��������� ----
public:  
  double computeDistance( int iSphere,  const PointType& pt );

  int getSpheresCount() { return int( spheres.size() ); };

  const CoverSphere< PointType >& getSphere( int iSphere ) { return spheres[iSphere]; };

  int countKids( int iSphere ); // ������������ ���������� ���������������� ����� � �������
 

#if 1 // ----------------------------------- not implemented yet 

  int                               // ����� �����, (-1 ���� �� ��������� ������� ���������), ���������� ��������        
    branchSubTreeUsingFirstCover(   // ������������� ����� �� ��������� -- ������ ���� branch&bounds
    const PointType& pt,            // ����� ��� ������� ���� ����� �� ����� -- ���� ��������� -- ����� ���������� 
                                    // �.�. ��� ������� ������ ����� ������� �� ��� �� ������ 
    int iStartSphere = 0,           // � ����� ����� �������� �����, 0 - ������ ������ 
    int iStartLevel = 0             // ������� ��������� �����, 0 - ������� ������ ������ 
    //    ,CoverRecord& record      // ������������ ������ ����������?
    ); 
 
public:
  const PointType& 
    findNearestPoint(       // ��������� � ��������� ����� ����� ����� �� ������
    const PointType& pt,    // ����� ��� ������� ���� ����� � ��������� �������
    double &best_distance,   // [in/out] ������ ���������� -- ��� �� � ��������� (�� ������ ������ ��� �������)
    int iStartSphere = 0  // � ����� ����� �������� �����, 0 - ������ ������ 
    );
 

public:
  // �������, ����������� ������������ ���������� ������ -- �� ������ ������ ��� ����, ��� ��� ������� ������ ������ �����
  bool checkCoverNetSphere( int iSphere, int iKidSphere ); 
 
public: 
  bool checkCoverNet();

  int                         // ����� �����, (-1 ���� �� ��������� ������� ���������)
    findNearestSphere(
    const PointType& pt,      // ����� ��� ������� ���� ����� � ��������� �������
    double& best_distance,    // [in/out] ������ ���������� -- ��� �� � ��������� (�� ������ ������ ��� �������)
    double distanceToPt = -1, // ���������� �� �����, maxRadius -- ���� �� ���������
    int iStartSphere = 0      // � ����� ����� �������� �����, 0 - ������ ������ 
    );
 

public:
  int                      // ����� �����
    dropToNearestKid(      // ����������� "������������" � ��������� ������� �����. 
                           // �� ����� -- ����� ������ ��� ��� ������������ �����
                           // �� ������ -- ����� ��������� ����� ������� ������ � ���������� �� ���
    int isphere,           // ������������� � ������ �����
    const PointType& pt,   // ����� ��� ������� ���� ����� � ��������� �������
    double* best_distance  // [in/out] ������ ���������� -- ��� �� � ��������� (�� ������ ������ ��� �������)
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
  double squeezeRatio = 0.5, // ����������� ���������� ��������� �����������
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

  attemptsToInsert = 0; // ������� ����� ������� �������
  rejectedInserts = 0;  // ������� ����� ������� ������� ������� ������� �����
};

/*CoverNet(const CoverNet& cover_net)//!!!�������� 
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

  attemptsToInsert = cover_net.attemptsToInsert; // ������� ����� ������� �������
  rejectedInserts = cover_net.rejectedInserts; // ������� ����� ������� ������� ������� ������� �����

  //!!!�������� ����������� ����������� ������
}*/



template < class PointType, class Metrics >//!!!! 
bool CoverNet< PointType, Metrics >::insert( const PointType& pt ) // false -- ����� �� ��������� ������ ����� ��������� (������� ����)
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
    return false; // ���������� ������� ������� �����
  }

  ////////#define NO_SEQUENTAL_PROXIMITY_ASSUMPTION
  //////#ifndef NO_SEQUENTAL_PROXIMITY_ASSUMPTION  // ���� ������������ ������� ������, �� ������ �������� ������
  //////    if (iLastSphere != -1)
  //////    {
  //////      double dist = computeDistance( iLastSphere, pt );
  //////      double rad = getRadius(iLastSphereLevel);
  //////      assert( spheres[iLastSphere].level == iLastSphereLevel );
  //////      if ( dist < rad ) // <<<<<<<<<<<<<<< MUST CHECK PARENT, GRANDPARENT and so on !!!!!
  //////      {
  //////        int start=iLastSphere;
  //////        insertPoint( pt, iLastSphere, iLastSphereLevel, dist ); // �������� iLastSphere & iLastSphereLevel
  //////        for (int isp = spheres[start].parent; isp >=0; isp = spheres[isp].parent)
  //////          spheres[isp].points++;
  //////        return true;
  //////      }
  //////    }
  //////#endif

  insertPoint( pt, 0, 0, dist_root ); // ��������� ��������� � ����
  return true;
}



template < class PointType, class Metrics >
void CoverNet< PointType, Metrics >::clearNet()
  {
    //�������� �����������
    ///////////////////////
    /*Metrics* ruler;
    double rootRadius;  // ��� �������� ���� ���������� ����������� ��������� ���������� ����� �������� ���������� �������
    // ���� ����������� �����, ��������� �� ���� �� ���������� ������ rootRadius ������ �������������� �����
    double minRadius;   // ���� ���������� �� ������ ����� ������ minRadius ��������� ����� �� �����������
    double maxRadius;*/
    ////////////////////////
      squeezeRatio = 0.5; // ����������� ���������� ��������� �����������
      maxRadius = -1;
  
      iLastSphere = -1;
      iLastSphereLevel = -1;

      attemptsToInsert = 0;
      rejectedInserts = 0; 

      spheres.clear();
      ancles.clear();
      //levelsRadii.clear(); �� ������!!
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
int CoverNet< PointType, Metrics >::makeSphere(    // ������� �����, �� ������ ����������� � ���������� �� �����
                         int iSphereLevel,         // ������� ����������� �����
                         const PointType& pt,      // ����� �����
                         int parent,               // ������������ ����� (�������������, ��� ����� ����� ������ ������������ � �� ������ ��������
                         double distance_to_parent // ���������� �� ������ ������������ �����
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
      int bro = spheres[ parent ].last_kid; // ��������� ������ ����� ������������� �� ���������� �� ��������???
      spheres[ parent ].last_kid = iLastSphere;
      spheres[ iLastSphere ].prev_brother = bro;
#else
      int bro = spheres[ parent ].last_kid;//��������� �������
      if ( bro == 0 ) // bro ����
      {
        spheres[ parent ].last_kid = iLastSphere;//����� iLastSphere -- ������ �������
        spheres[ iLastSphere ].prev_brother = bro;// = 0;
        /*if ( spheres[ iLastSphere ].center != spheres[ parent ].center )
        cout << "O_O" << endl;
        else
        cout << "+" << endl;*/
      }
      else
      {
        int prev_bro = spheres[ bro ].prev_brother;     //������������� �������
        spheres[ bro ].prev_brother = iLastSphere;      //������������� ������ �����
        spheres[ iLastSphere ].prev_brother = prev_bro; //����� ����� ����� prev_bro
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
                 const PointType& pt, // ����� ������ ����� iSphere �� ������ iSphereLevel
                 int iSphere, 
                 int iSphereLevel, 
                 double dist          // ���������� �� ������ ����� iSphere (��� ��������)
                 )
{ 
  // �� ������ �����
  assert( computeDistance( iSphere, pt ) <= getRadius( iSphereLevel ) + 0.0000001 );
  spheres[ iSphere ].points++;

  double minrad = getMinRadius( iSphereLevel );
  if ( dist < minrad )
  { // ����� ��������������� � ������ �����
    attachPoint( pt, iSphere, iSphereLevel, dist ); // ����� ��������������� � ������ �����, �� ������� ��������� ����
    return;
  }

  // ������ ��� �� �������
  int kid = spheres[ iSphere ].last_kid;
  double rlast_kid = getRadius( iSphereLevel + 1 );
  while ( kid > 0 )
  {
    double dist_kid = computeDistance( kid, pt );
    if ( dist_kid < rlast_kid )
    {
      insertPoint( pt, kid, iSphereLevel + 1, dist_kid ); // �����������
      return; // ����� �������
    }
    kid = spheres[ kid ].prev_brother;
  }
  // ��� �����, ����� ��������, ������� ����� �, ��������, �� ������ �����������
  makeSphere( iSphereLevel + 1,  pt, iSphere, dist );
}



template < class PointType, class Metrics >
void CoverNet< PointType, Metrics >::attachPoint( const PointType& pt,
                                                 int iSphere,
                                                 int iSphereLevel,
                                                 double dist )
// ����� ��������������� � ������ �����, �� ������� ��������� ����
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
//������� � ���������
template < class PointType, class Metrics >
double CoverNet< PointType, Metrics >::computeDistance( int iSphere,  const PointType& pt )
{
  double dist = ruler->computeDistance( spheres[ iSphere ].center, pt );
  return dist;
}

template < class PointType, class Metrics >
int CoverNet< PointType, Metrics >::countKids( int iSphere ) // ������������ ���������� ���������������� ����� � �������
{ 
  int count=0;
  for (int kid = spheres[ iSphere ].last_kid; kid > 0; kid = spheres[ kid ].prev_brother)
    count++;
  return count; 
};



template < class PointType, class Metrics >
const PointType& 
CoverNet< PointType, Metrics >::findNearestPoint( // ��������� � ��������� ����� ����� ����� �� ������
                 const PointType& pt,             // ����� ��� ������� ���� ����� � ��������� �������
                 double& best_distance,            // [in/out] ������ ���������� -- ��� �� � ��������� (�� ������ ������ ��� �������)
                 int iStartSphere = 0           // � ����� ����� �������� �����, 0 - ������ ������ 
                 )
{//���� ���������� ���� �� �������!!!
  //!!�����
    int iNearestSphere = findNearestSphere( pt, best_distance, -1, iStartSphere );
    if ( iNearestSphere == -1 )
    {
       throw std::invalid_argument( "distance to root is more then maximal radius" );
      //std::cerr << "error: distance to root is more then maximal radius" << std::endl;
    }
    return spheres[ iNearestSphere ].center;    
}



template < class PointType, class Metrics >
bool CoverNet< PointType, Metrics >::checkCoverNetSphere( int iSphere, int iKidSphere ) // �������, ����������� ������������ ���������� ������ -- �� ������ ������ ��� ����, ��� ��� ������� ������ ������ �����
{ 
  int isp = iSphere; // ������� �����
  int lev = spheres[ isp ].level; // ������� �������
  double  rad = getRadius( lev );
  double dist = computeDistance( isp, spheres[ iKidSphere ].center );
  if ( dist > rad )// ���� ���������� �� ������� ������ �������
  {
    cout << "build tree error" << endl;
    cout << "incorrect distance " << dist << " from sphere N = " << iSphere 
      << " with center in " << spheres[ isp ].center << " and R = " << rad << " to kid N = " << iKidSphere 
      << " with center in " << spheres[ iKidSphere ].center << endl;
    system ( "pause" );
    return false;
  }
  for (int kid = spheres[ iKidSphere ].last_kid; kid > 0; kid = spheres[ kid ].prev_brother)// ���� �� ���� �����
  {
    int kid_ans = checkCoverNetSphere( iSphere, kid );
    if ( !kid_ans )
      return false;
  }

  return true;    
}



template < class PointType, class Metrics >
int                                                             // ����� �����, (-1 ���� �� ��������� ������� ���������), ���������� ��������        
CoverNet< PointType, Metrics >::branchSubTreeUsingFirstCover(   // ������������� ����� �� ��������� -- ������ ���� branch&bounds
                             const PointType& pt,               // ����� ��� ������� ���� ����� �� ����� -- ���� ��������� -- ����� ���������� 
                                                                // �.�. ��� ������� ������ ����� ������� �� ��� �� ������ 
                             int iStartSphere = 0,              // � ����� ����� �������� �����, 0 - ������ ������ 
                             int iStartLevel = 0                // ������� ��������� �����, 0 - ������� ������ ������ 
                             //    ,CoverRecord& record         // ������������ ������ ����������?
                             ) 
{
  double dist = computeDistance( iStartSphere, pt );
  //  record.update( dist, iStartSphere, iStartLevel );
  if ( !( dist < getRadius( iStartLevel ) ) )
    return -1;
  // ��������� � �������� ������������

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
int                                                // ����� �����, (-1 ���� �� ��������� ������� ���������)
CoverNet< PointType, Metrics >::findNearestSphere(
                  const PointType& pt,             // ����� ��� ������� ���� ����� � ��������� �������
                  double& best_distance,           // [in/out] ������ ���������� -- ��� �� � ��������� (�� ������ ������ ��� �������)
                  double distanceToPt = -1,        // ���������� �� �����, maxRadius -- ���� �� ���������
                  int iStartSphere = 0             // � ����� ����� �������� �����, 0 - ������ ������ 
                  )
{
  int isp = iStartSphere;       // ������� �����
  int lev = spheres[ isp ].level; // ������� �������
  double  rad = getRadius( lev ); // ������ ������� ����� (�� ������ ������)
  double dist = distanceToPt;

  if ( dist == -1 )
    dist = computeDistance( isp, pt );

  if ( isp == 0 && dist >= rad )// ���� �� ��������� ���������
  {
    return -1;
  }

  int ans = -1;
  double min_dist = std::max( 0.0, dist - rad );// ���������� �� pt �� �����  
  if ( min_dist > best_distance )// ���� ����������� ���������� ������ ������������
  {
    return -1;
  }
  if ( dist < best_distance )// ���� ����� �������� �����
  {
    ans = isp;
    best_distance = dist;//����������� ������,
  }

  const int MAX_KIDS_SIZE = 256;
  std::pair< double, int > kids[ MAX_KIDS_SIZE ];

  if ( spheres[ isp ].last_kid == 0 ) // ����
    return ans;

  int kid = spheres[ isp ].last_kid;
  int kids_size = 0;
  if ( spheres[ kid ].center == spheres[ isp ].center )//isp - ������� �����
  {
    kids[ kids_size++ ] = std::make_pair( dist, kid );
    kid = spheres[ kid ].prev_brother;
  }

  /*for (; kid > 0; kid = spheres[kid].prev_brother)// ���� �� ���� �����
  {
    if (fabs(spheres[kid].distance_to_parent - dist) < best_distance + getRadius(lev + 1)) // ���� �� ����������� ������������, ����� �������� �����
      kids[kids_size++] = std::make_pair(computeDistance(kid, pt), kid);  // <<<<<<<<<< CHECK kids_size++
  }

  sort(kids + 0, kids + kids_size);
  for (int i = 0; i < kids_size; ++i)
  {
    if (best_distance + getRadius(lev + 1) <= fabs(spheres[kids[i].second].distance_to_parent - dist))// ���� �� ����������� ������������, ������ �������� �����
      continue;
    int new_ans = findNearestSphere(pt, best_distance, kids[i].first, kids[i].second);
    if (new_ans != -1)
      ans = new_ans;
  }*/
  for ( ; kid > 0; kid = spheres[kid].prev_brother )// ���� �� ���� �����
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
int // ����� �����
CoverNet< PointType, Metrics >::dropToNearestKid( // ����������� "������������" � ��������� ������� �����. 
                 // �� ����� -- ����� ������ ��� ��� ������������ �����
                 // �� ������ -- ����� ��������� ����� ������� ������ � ���������� �� ���
                 int isphere, // ������������� � ������ �����
                 const PointType& pt, // ����� ��� ������� ���� ����� � ��������� �������
                 double* best_distance // [in/out] ������ ���������� -- ��� �� � ��������� (�� ������ ������ ��� �������)
                 )
{
  // ���� ���� �������, � �������� �������� (�� ���������� -- ���������) -- ������������� � ����
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

  int maxLevel = -1; // ��� �� �������
  for ( int i = 0; i < int( spheres.size() ); i++ )
    maxLevel = std::max( maxLevel, spheres[i].level );

  std::cout 
    << "********** CoverNet: " << std::endl 
    << "spheres=" << spheres.size() 
    << "\tlevels=" << maxLevel + 1 
    << "\tattemptsToInsert=" << attemptsToInsert // ������� ����� ������� �������
    << "\trejectedInserts=" << rejectedInserts // ������� ����� ������� ������� ������� ������� �����
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