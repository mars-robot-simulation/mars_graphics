#pragma once

#include "DrawObject.h"

#include <mars_utils/Vector.h>

#include <list>

namespace mars
{
    namespace graphics
    {

        class CubeDrawObject : public DrawObject
        {
        public:
            CubeDrawObject(GraphicsManager *g);

        protected:
            static osg::ref_ptr<osg::Geode> sharedCube;
            virtual std::list< osg::ref_ptr< osg::Geode > > createGeometry();
        };

    } // end of namespace graphics
} // end of namespace mars
