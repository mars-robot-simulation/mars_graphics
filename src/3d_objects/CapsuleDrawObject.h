#pragma once

#include "DrawObject.h"

#include <mars_interfaces/MARSDefs.h>
#include <mars_utils/Vector.h>

#include <list>

namespace mars
{
    namespace graphics
    {

        class CapsuleDrawObject : public DrawObject
        {

        public:
            CapsuleDrawObject(GraphicsManager *g);
            virtual void setScaledSize(const mars::utils::Vector &scaledSize);

        protected:
            osg::ref_ptr<osg::Geometry> geom;

            virtual std::list< osg::ref_ptr< osg::Geode > > createGeometry();
        }; // end of class CapsuleDrawObject

    } // end of namespace graphics
} // end of namespace mars
