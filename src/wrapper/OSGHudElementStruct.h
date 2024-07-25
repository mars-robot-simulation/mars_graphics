#pragma once

#include "../2d_objects/HUDElement.h"

#include <mars_interfaces/graphics/draw_structs.h>

#include <string>

#include <osg/Group>

namespace mars
{
    namespace graphics
    {

        /**
         * Wraps hudElementStruct in osg::Group.
         */
        class OSGHudElementStruct : public osg::Group
        {
        public:
            /**
             * Constructor creates HUDElement from hudElementStruct.
             */
            OSGHudElementStruct(
                const interfaces::hudElementStruct &he,
                const std::string &config_path,
                unsigned int id,
                osg::Node* node = nullptr);
            ~OSGHudElementStruct();

            HUDElement *getHUDElement();
        private:
            HUDElement *elem_;
        }; // end of class OSGHudElementStruct

    } // end of namespace graphics
} // end of namespace mars
