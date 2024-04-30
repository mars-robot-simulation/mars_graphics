#pragma once

#include <mars_utils/Vector.h>

#include <osg/Group>

namespace mars
{
    namespace graphics
    {

        class AxisPrimitive : public osg::Group
        {
        public:
            AxisPrimitive(const mars::utils::Vector &first,
                          const mars::utils::Vector &second,
                          const mars::utils::Vector &third,
                          const mars::utils::Vector &axis1,
                          const mars::utils::Vector &axis2);
        }; // end of class AxisPrimitive

    } // end of namespace graphics
} // end of namespace mars
