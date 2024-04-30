#pragma once

#include "DrawObject.h"

#include <mars_interfaces/MARSDefs.h>
#include <mars_utils/Vector.h>

#include <list>

namespace mars
{
    namespace graphics
    {

        class CylinderDrawObject : public DrawObject
        {

        public:
            CylinderDrawObject(GraphicsManager *g,
                               mars::interfaces::sReal radius,
                               mars::interfaces::sReal height);
            virtual void setScaledSize(const mars::utils::Vector &scaledSize);

            static void createShellGeometry(osg::Vec3Array *vertices,
                                            osg::Vec3Array *normals,
                                            osg::Vec2Array *uv,
                                            float radius, float height,
                                            float theta);

        private:
            mars::interfaces::sReal radius_;
            mars::interfaces::sReal height_;

            virtual std::list< osg::ref_ptr< osg::Geode > > createGeometry();
        }; // end of class CylinderDrawObject

    } // end of namespace graphics
} // end of namespace mars
