#pragma once

#include <osg/Geometry>
#include <osg/Camera>

namespace mars
{
    namespace graphics
    {

        class HUDDataType : public osg::Referenced
        {
        public:
            HUDDataType(osg::Geometry *g, osg::Camera *c);
            ~HUDDataType(void);
            void updateRect(void);
        protected:
            osg::Geometry *geometry;
            osg::Camera *camera;
        }; // end of class HUDDataType

        class HUDNodeCallback : public osg::NodeCallback
        {
        public:
            virtual void operator()(osg::Node *node, osg::NodeVisitor *nv);
        }; // end of class HUDNodeCallback

    } // end of namespace graphics
} // end of namespace mars
