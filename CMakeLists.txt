project(mars_graphics)
set(PROJECT_VERSION 1.0)
set(PROJECT_DESCRIPTION "This library provides a 3d visualization through simple interfaces.")
cmake_minimum_required(VERSION 2.6)

include(FindPkgConfig)
include(CheckIncludeFileCXX)

CHECK_INCLUDE_FILE_CXX("tr1/functional" USE_TR1)
if(${USE_TR1})
    ADD_DEFINITIONS(-DUSE_TR1)
else(${USE_TR1})
    ADD_DEFINITIONS(-DNO_TR1)
endif()

find_package(lib_manager)
lib_defaults()
define_module_info()

setup_opencv()

include_directories(
      src
      src/interfaces
      src/hud_elements
      src/draw_objects
      src/glsl
      src/shadow
      src/3d_objects
)

OPTION(DEPTH_IMAGES "Compile with Depth image support, might not work on all machines" true)
if(DEPTH_IMAGES)
    ADD_DEFINITIONS(-DDEPTH_IMAGES)
endif()

set(USE_VERTEX_BUFFER 0)
if(USE_VERTEX_BUFFER)
    ADD_DEFINITIONS(-DUSE_VERTEX_BUFFER)
endif()

pkg_check_modules(OSGQT openscenegraph-osgQt5)
if(OSGQT_FOUND)
find_package(OpenSceneGraph REQUIRED osgManipulator osgViewer osgFX osgShadow osgParticle osgTerrain osgDB osgGA osgWidget osgText osgUtil)

include_directories(${OSGQT_INCLUDE_DIRS})
link_directories(${OSGQT_LIBRARY_DIRS})
add_definitions(${OSGQT_CFLAGS_OTHER})  #cflags without -I

else()
find_package(OpenSceneGraph REQUIRED osgManipulator osgViewer osgFX osgShadow osgParticle osgTerrain osgDB osgGA osgWidget osgText osgUtil osgQt)
endif()

include_directories(${OPENSCENEGRAPH_INCLUDE_DIRS})
link_directories(${OPENSCENEGRAPH_LIBRARY_DIRS})


set (CMAKE_REQUIRED_INCLUDES ${CMAKE_REQUIRED_INCLUDES} ${OPENSCENEGRAPH_INCLUDE_DIRS})
CHECK_INCLUDE_FILE_CXX("osg/Version" HAVE_OSG_VERSION_H)
if(${HAVE_OSG_VERSION_H})
    ADD_DEFINITIONS(-DHAVE_OSG_VERSION_H)
endif()

set (QT_USE_QTOPENGL TRUE)
setup_qt()

pkg_check_modules(PKGCONFIG REQUIRED
                  lib_manager
                  mars_interfaces
                  cfg_manager
                  data_broker
                  configmaps
                  mars_utils
                  osg_material_manager
                  osg_terrain
                  osg_lines
                  osg_frames
)

include_directories(${PKGCONFIG_INCLUDE_DIRS})
link_directories(${PKGCONFIG_LIBRARY_DIRS})
add_definitions(${PKGCONFIG_CFLAGS_OTHER})  #cflags without -I

set(HEADERS
           src/GraphicsCamera.h
           src/GraphicsManager.h
           #src/GraphicsViewer.h
           src/GraphicsWidget.h
           src/gui_helper_functions.h
           src/HUD.h
           src/PostDrawCallback.h
           src/QtOsgMixGraphicsWidget.h

           src/shadow/ShadowMap.h
           src/shadow/ParallelSplitShadowMap.h
)

set(HEADERS_2D
           src/2d_objects/HUDElement.h
           src/2d_objects/HUDNode.h
           src/2d_objects/HUDLabel.h
           src/2d_objects/HUDLines.h
           src/2d_objects/HUDTerminal.h
           src/2d_objects/HUDTexture.h
           src/2d_objects/HUDOSGNode.h

)

set(HEADERS_3D
           src/3d_objects/AxisPrimitive.h
           src/3d_objects/CapsuleDrawObject.h
           src/3d_objects/CoordsPrimitive.h
           src/3d_objects/CubeDrawObject.h
           src/3d_objects/CylinderDrawObject.h
           src/3d_objects/EmptyDrawObject.h
           src/3d_objects/DrawObject.h
           src/3d_objects/GridPrimitive.h
           src/3d_objects/LoadDrawObject.h
           src/3d_objects/OceanDrawObject.h
           src/3d_objects/PlaneDrawObject.h
           src/3d_objects/SphereDrawObject.h
           src/3d_objects/TerrainDrawObject.h
           src/3d_objects/VertexBufferTerrain.h
           src/3d_objects/MultiResHeightMapRenderer.h
)

set(HEADERS_WRAPPER
           src/wrapper/OSGDrawItem.h
           src/wrapper/OSGHudElementStruct.h
           src/wrapper/OSGLightStruct.h
           src/wrapper/OSGMaterialStruct.h
           src/wrapper/OSGNodeStruct.h
)

set(QT_MOC_HEADERS
           src/QtOsgMixGraphicsWidget.h
)

set(SOURCES
           src/GraphicsCamera.cpp
           src/GraphicsManager.cpp
           #src/GraphicsViewer.cpp
           src/GraphicsWidget.cpp
           src/gui_helper_functions.cpp
           src/HUD.cpp
           src/QtOsgMixGraphicsWidget.cpp
           src/PostDrawCallback.cpp

           src/wrapper/OSGDrawItem.cpp
           src/wrapper/OSGHudElementStruct.cpp
           src/wrapper/OSGLightStruct.cpp
           src/wrapper/OSGMaterialStruct.cpp
           src/wrapper/OSGNodeStruct.cpp

           src/3d_objects/AxisPrimitive.cpp
           src/3d_objects/CapsuleDrawObject.cpp
           src/3d_objects/CoordsPrimitive.cpp
           src/3d_objects/CubeDrawObject.cpp
           src/3d_objects/CylinderDrawObject.cpp
           src/3d_objects/DrawObject.cpp
           src/3d_objects/GridPrimitive.cpp
           src/3d_objects/LoadDrawObject.cpp
           src/3d_objects/OceanDrawObject.cpp
           src/3d_objects/PlaneDrawObject.cpp
           src/3d_objects/SphereDrawObject.cpp
           src/3d_objects/TerrainDrawObject.cpp
           src/3d_objects/VertexBufferTerrain.cpp
           src/3d_objects/MultiResHeightMapRenderer.cpp
           src/2d_objects/HUDNode.cpp
           src/2d_objects/HUDLabel.cpp
           src/2d_objects/HUDTerminal.cpp
           src/2d_objects/HUDTexture.cpp
           src/2d_objects/HUDLines.cpp
           src/2d_objects/HUDOSGNode.cpp

           src/shadow/ShadowMap.cpp
           src/shadow/ParallelSplitShadowMap.cpp
)

if (${USE_QT5})
qt5_wrap_cpp( QT_MOC_HEADER_SRC ${QT_MOC_HEADERS})
else (${USE_QT5})
qt4_wrap_cpp( QT_MOC_HEADER_SRC ${QT_MOC_HEADERS})
endif (${USE_QT5})

#cmake variables
configure_file(${CMAKE_SOURCE_DIR}/config.h.in ${CMAKE_BINARY_DIR}/config.h @ONLY)
include_directories("${CMAKE_BINARY_DIR}")

add_library(${PROJECT_NAME} SHARED ${SOURCES} ${QT_MOC_HEADER_SRC} config.h)

if (${USE_QT5})
qt5_use_modules(${PROJECT_NAME} Widgets OpenGL)
#qt5_use_modules(${PROJECT_NAME} MacExtras)
endif (${USE_QT5})

if(APPLE)
 FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
 SET(APPLE_LIBS ${OPENGL_LIBRARY} -L/opt/local/lib opencv_core opencv_highgui opencv_imgproc)
endif(APPLE)

if(WIN32)
  if(USE_VERTEX_BUFFER)
    SET(WIN_LIBS glew32 opengl32)
  else(USE_VERTEX_BUFFER)
    SET(WIN_LIBS opengl32)
  endif(USE_VERTEX_BUFFER)
  SET(WIN_LIBS ${WIN_LIBS} ${OSGQT_LIBRARIES})
endif(WIN32)

if(OSGQT_FOUND)
  SET(EXTRA_LIBS ${OSGQT_LIBRARIES})
endif()

target_link_libraries(${PROJECT_NAME}
            ${QT_LIBRARIES}
            ${OPENSCENEGRAPH_LIBRARIES}
            ${PKGCONFIG_LIBRARIES}
            ${OPENCV_LIBRARIES}
            ${APPLE_LIBS}
            ${WIN_LIBS}
            ${EXTRA_LIBS}
            pthread
)

if(WIN32)
  set(LIB_INSTALL_DIR bin) # .dll are in PATH, like executables
else(WIN32)
  set(LIB_INSTALL_DIR lib)
endif(WIN32)

set(_INSTALL_DESTINATIONS
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION ${LIB_INSTALL_DIR}
	ARCHIVE DESTINATION lib
)

# Install the library
install(TARGETS ${PROJECT_NAME} ${_INSTALL_DESTINATIONS})

# Install headers into mars include directory
install(FILES ${HEADERS} DESTINATION include/${PROJECT_NAME})
install(FILES ${HEADERS_2D} DESTINATION include/${PROJECT_NAME}/2d_objects)
install(FILES ${HEADERS_3D} DESTINATION include/${PROJECT_NAME}/3d_objects)
install(FILES ${HEADERS_WRAPPER} DESTINATION include/${PROJECT_NAME}/wrapper)

# Prepare and install necessary files to support finding of the library
# using pkg-config
configure_file(configuration/${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)

#configure_file(configuration/mars_Graphics.in ${CMAKE_BINARY_DIR}/mars_Graphics.yaml @ONLY)
#install(FILES ${CMAKE_BINARY_DIR}/mars_Graphics.yaml DESTINATION configuration/mars_default/)

INSTALL(DIRECTORY resources DESTINATION share/${PROJECT_NAME})
