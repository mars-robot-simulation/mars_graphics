#pragma once

#include "DrawObject.h"

#include <mars_interfaces/MARSDefs.h>
#include <mars_utils/Vector.h>

namespace mars
{
    namespace graphics
    {

        class SphereDrawObject : public DrawObject
        {

        public:
            SphereDrawObject(GraphicsManager *g);
            ~SphereDrawObject();

            static void createGeometry(osg::Vec3Array *vertices,
                                       osg::Vec3Array *normals,
                                       osg::Vec2Array *uv,
                                       float radius,
                                       const osg::Vec3 &topOffset,
                                       const osg::Vec3 &bottomOffset,
                                       bool backfaces=false,
                                       unsigned int levelOfDetail=4);

            //virtual void setScaledSize(const mars::utils::Vector &scaledSize);

        protected:
            static osg::ref_ptr<osg::Geode> sharedCube;
            virtual std::list< osg::ref_ptr< osg::Geode > > createGeometry();

        }; // end of class SphereDrawObject

    } // end of namespace graphics
} // end of namespace mars
