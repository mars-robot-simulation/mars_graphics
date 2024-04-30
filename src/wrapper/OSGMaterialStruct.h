#pragma once

#include <osg/Material>

#include <mars_interfaces/MaterialData.h>

namespace mars
{
    namespace graphics
    {

        /**
         * Wraps MaterialData in osg::Material.
         */
        class OSGMaterialStruct : public osg::Material
        {
        public:
            OSGMaterialStruct(const mars::interfaces::MaterialData &mat);
        }; // end of class OSGMaterialStruct

    } // end of namespace graphics
} // end of namespace mars
