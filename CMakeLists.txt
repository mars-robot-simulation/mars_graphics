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

find_package(lib_manager REQUIRED)
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

set(DEPENDENCIES
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
    openscenegraph-osgManipulator
    openscenegraph-osgViewer
    openscenegraph-osgFX
    openscenegraph-osgShadow
    openscenegraph-osgParticle
    openscenegraph-osgTerrain
    openscenegraph-osgDB
    openscenegraph-osgGA
    openscenegraph-osgWidget
    openscenegraph-osgText
    openscenegraph-osgUtil
)

set (QT_USE_QTOPENGL TRUE)
setup_qt()

if (${USE_QT5})
set( OSG_QT openscenegraph-osgQt5)
else (${USE_QT5})
set( OSG_QT openscenegraph-osgQt)
endif (${USE_QT5})

pkg_check_modules(OSGQT ${OSG_QT})
if(OSGQT_FOUND)
    list(APPEND DEPENDENCIES ${OSG_QT})
endif()
pkg_check_modules(Dependencies REQUIRED IMPORTED_TARGET ${DEPENDENCIES})

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
 SET(APPLE_LIBS ${OPENGL_LIBRARY} -L/opt/local/lib opencv_core opencv_highgui opencv_imgproc opencv_imgcodecs)
endif(APPLE)

if(WIN32)
  if(USE_VERTEX_BUFFER)
    SET(WIN_LIBS glew32 opengl32)
  else(USE_VERTEX_BUFFER)
    SET(WIN_LIBS opengl32)
  endif(USE_VERTEX_BUFFER)
endif(WIN32)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        PkgConfig::Dependencies
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
list(JOIN DEPENDENCIES " " PKGCONFIG_REQUIRES)
configure_file(configuration/${PROJECT_NAME}.pc.in ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.pc DESTINATION lib/pkgconfig)

#configure_file(configuration/mars_Graphics.in ${CMAKE_BINARY_DIR}/mars_Graphics.yaml @ONLY)
#install(FILES ${CMAKE_BINARY_DIR}/mars_Graphics.yaml DESTINATION configuration/mars_default/)

INSTALL(DIRECTORY resources DESTINATION share/${PROJECT_NAME})
