#pragma once

#include "DrawObject.h"

namespace mars
{
    namespace graphics
    {

        class EmptyDrawObject : public DrawObject
        {
        public:
            EmptyDrawObject(GraphicsManager *g) : DrawObject(g) {}

        protected:
            std::list< osg::ref_ptr< osg::Geode > > createGeometry()
                {
                    return std::list< osg::ref_ptr< osg::Geode > >();
                }
        };

    } // end of namespace graphics
} // end of namespace mars
