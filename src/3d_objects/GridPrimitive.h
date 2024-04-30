#pragma once

#include <osg/Group>

#include "GraphicsWidget.h"

namespace mars
{
    namespace graphics
    {

        class GridPrimitive : public osg::Group
        {
        public:
            GridPrimitive(GraphicsWidget *gw);
        };

    } // end of namespace graphics
} // end of namespace mars
