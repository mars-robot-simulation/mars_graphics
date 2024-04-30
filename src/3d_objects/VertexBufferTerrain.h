#pragma once

// On MS Windows MultiResHeightMapRenderer.h needs to be included before
// any OpenGL stuff!
#include "MultiResHeightMapRenderer.h"
#include "LoadDrawObject.h"
#include <osg/Drawable>
#include <mars_interfaces/terrainStruct.h>

namespace mars
{
    namespace graphics
    {

        class VertexBufferTerrain : public osg::Drawable
        {

        public:
            VertexBufferTerrain();  
            VertexBufferTerrain(const interfaces::terrainStruct *ts);

            VertexBufferTerrain(const VertexBufferTerrain &pg,
                                const osg::CopyOp &copyop=osg::CopyOp::SHALLOW_COPY)
                {
                    fprintf(stderr, "error: not implemented yet!!");
                }

            virtual ~VertexBufferTerrain();

            virtual osg::Object* cloneType() const
                {
                    fprintf(stderr, "error: not implemented yet!!");
                    return new VertexBufferTerrain();
                }

            virtual osg::Object* clone(const osg::CopyOp& copyop) const
                {
                    fprintf(stderr, "error: not implemented yet!!");
                    return new VertexBufferTerrain (*this, copyop);
                }

            virtual void drawImplementation(osg::RenderInfo& renderInfo) const;
            void collideSphere(double xPos, double yPos, double zPos, double radius);
#if (OPENSCENEGRAPH_MAJOR_VERSION < 3 || ( OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION < 4))
            osg::BoundingBox computeBound() const;
#elif (OPENSCENEGRAPH_MAJOR_VERSION > 3 || (OPENSCENEGRAPH_MAJOR_VERSION == 3 && OPENSCENEGRAPH_MINOR_VERSION >= 4))
            osg::BoundingSphere computeBound() const;
#else
#error Unknown OSG Version
#endif
            void setSelected(bool val);

        private:
            MultiResHeightMapRenderer *mrhmr;
            double width, height, scale;

        }; // end of class VertexBufferTerrain

    } // end of namespace graphics
} // end of namespace mars
