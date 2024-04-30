#pragma once

#include "DrawObject.h"

#include <mars_utils/Vector.h>

#include <list>

namespace mars
{
    namespace graphics
    {

        class PlaneDrawObject : public DrawObject
        {

        public:
            PlaneDrawObject(GraphicsManager *g, const mars::utils::Vector &ext);
            virtual void setScaledSize(const mars::utils::Vector &scaledSize);

        private:
            mars::utils::Vector extent_;

            virtual std::list< osg::ref_ptr< osg::Geode > > createGeometry();
        }; // end of class PlaneDrawObject

    } // end of namespace graphics
} // end of namespace mars
