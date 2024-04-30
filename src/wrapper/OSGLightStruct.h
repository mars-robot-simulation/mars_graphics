#pragma once

#include <mars_interfaces/LightData.h>

#include <osg/LightSource>
#include <osg/ShapeDrawable>

namespace mars
{
    namespace graphics
    {

        /**
         * Wraps lightStruct in osg::LightSource.
         */
        class OSGLightStruct : public osg::LightSource
        {
        public:
            /**
             * Constructor creates osg::LightSource from lightStruct.
             */
            OSGLightStruct(const mars::interfaces::LightData &ls);
            void update(const mars::interfaces::LightData &ls);
        private:
            osg::ref_ptr<osg::Light> light_;
            osg::ref_ptr<osg::Geode> lightMarkerGeode;
        };

    } // end of namespace graphics
} // end of namespace mars
