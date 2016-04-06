cmake_minimum_required ( VERSION 2.6 ) 
 
set ( PROJECT lights_tracks ) 
 
project ( ${PROJECT} ) 

find_package( OpenCV REQUIRED )
  
set ( SOURCES 
    src/main.cpp
    src/light.cpp
    src/lights_detector.cpp
    src/lights_tracker.cpp
    )

 
set( HEADERS  
    src/ticker.h
    src/cover_net.h
    src/euclid_2d.h
    src/light.h
    src/lights_detector.h
    src/lights_tracker.h
    )


 
add_executable ( ${PROJECT} ${SOURCES} ${HEADERS} )

add_dependencies ( lights_tracks ${OpenCV_LIBS} )
target_link_libraries( lights_tracks ${OpenCV_LIBS} )