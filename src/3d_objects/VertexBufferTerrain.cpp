#ifdef USE_VERTEX_BUFFER

/* The prototypes are only declared in GL/glext.h if this is defined */
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1 //for glGenBuffers, glBindBuffer,
                              //glDeleteBuffers
#endif

#include <osg/Version>

#include "VertexBufferTerrain.h"

#ifdef WIN32
#include <windows.h>
#endif

//#define DEBUG_TIME

#ifdef DEBUG_TIME
#include <mars_utils/misc.h>
#endif

namespace mars
{
    namespace graphics
    {

        VertexBufferTerrain::VertexBufferTerrain()
        {

            setSupportsDisplayList(false);
            setUseDisplayList(false);

            mrhmr = new MultiResHeightMapRenderer(1, 1, 1, 1, 1.0, 1.0, 1, 1.0, 1.0);
            width = height = scale = 1.0;
        }

        VertexBufferTerrain::VertexBufferTerrain(const interfaces::terrainStruct *ts)
        {

            setSupportsDisplayList(false);

            mrhmr = new MultiResHeightMapRenderer(ts->width, ts->height,
                                                  ts->targetWidth, ts->targetHeight,
                                                  1.0, 1.0, 1.0, ts->texScaleX,
                                                  ts->texScaleY);
            double maxHeight = 0.0;
            double offset;

            for(int i=0; i<ts->height; ++i)
                for(int j=0; j<ts->width; ++j)
                {
                    if(i==0 || j==0 || i==ts->height-1 || j==ts->width-1) offset = -0.1;
                    else offset = 0.0;
                    mrhmr->setHeight(j, i, offset+ts->scale*ts->pixelData[i*ts->width+j]);
                    if(ts->pixelData[i*ts->width+j]*ts->scale > maxHeight)
                    {
                        maxHeight = ts->pixelData[i*ts->width+j]*ts->scale;
                    }
                }

            width = ts->targetWidth;
            height = ts->targetHeight;
            scale = maxHeight;
        }

        VertexBufferTerrain::~VertexBufferTerrain()
        {
            delete mrhmr;
        }

        void VertexBufferTerrain::drawImplementation(osg::RenderInfo& renderInfo) const
        {
#ifdef DEBUG_TIME
            long drawTime = utils::getTime();
#endif
            osg::State& state = *renderInfo.getState();
            state.disableAllVertexArrays();
#if (OPENSCENEGRAPH_MAJOR_VERSION < 3 || ( OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION < 5) || ( OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION == 5 && OPENSCENEGRAPH_PATCH_VERSION < 9))
            osg::ArrayDispatchers& arrayDispatchers = state.getArrayDispatchers();
#elif (OPENSCENEGRAPH_MAJOR_VERSION > 3 || (OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION > 5) || (OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION == 5 && OPENSCENEGRAPH_PATCH_VERSION >= 9))
            osg::AttributeDispatchers& arrayDispatchers = state.getAttributeDispatchers();
#else
#error Unknown OSG Version
#endif

            arrayDispatchers.reset();
            //arrayDispatchers.dispatch(osg::Geometry::BIND_OVERALL,0);

            state.lazyDisablingOfVertexAttributes();
            state.applyDisablingOfVertexAttributes();

            mrhmr->render();
#ifdef DEBUG_TIME
            drawTime = utils::getTimeDiff(drawTime);
            if(drawTime > 1)
                fprintf(stderr, "MultiResHeightMapRenderer: drawTime: %ld\n", drawTime);
#endif
        }

        void VertexBufferTerrain::collideSphere(double xPos, double yPos,
                                                double zPos, double radius)
        {

            mrhmr->collideSphere(xPos, yPos, zPos, radius);
        }

#if (OPENSCENEGRAPH_MAJOR_VERSION < 3 || ( OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION < 4))
        osg::BoundingBox VertexBufferTerrain::computeBound() const
        {
            return osg::BoundingBox(0.0, 0.0, 0.0, width, height, scale);
        }
#elif (OPENSCENEGRAPH_MAJOR_VERSION > 3 || (OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION >= 4))
        osg::BoundingSphere VertexBufferTerrain::computeBound() const
        {
            return osg::BoundingSphere(osg::Vec3(width*0.5, height*0.5, scale*0.5), sqrt(width*width+height*height+scale*scale));
        }
#else
#error Unknown OSG Version
#endif

        void VertexBufferTerrain::setSelected(bool val)
        {
            if(val)
            {
                mrhmr->setDrawSolid(false);
                mrhmr->setDrawWireframe(true);
            } else
            {
                mrhmr->setDrawSolid(true);
                mrhmr->setDrawWireframe(false);
            }
        }

    } // end of namespace graphics
} // end of namespace mars

#endif /* USE_VERTEX_BUFFER */
