#pragma once

#include "GraphicsWidget.h"

#include <mars_utils/Vector.h>

#include <string>
#include <osg/Group>

namespace mars
{
    namespace graphics
    {

        class CoordsPrimitive : public osg::Group
        {
        public:
            /**
             * creates a coordination reference node showing axis x,y and z directions with
             * appropriate labels
             *
             * @param size The stretching factor of the coordination frame
             * @param transformFlag If set false the main coordination frame will be build
             *
             */
            CoordsPrimitive(GraphicsWidget *gw, const mars::utils::Vector &size,
                            std::string object_path, bool transformFlag=true);

            CoordsPrimitive(GraphicsWidget *gw, std::string object_path);
        }; // end of class CoordsPrimitive

    } // end of namespace graphics
} // end of namespace mars
