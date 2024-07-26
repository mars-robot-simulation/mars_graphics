#pragma once

#include "../3d_objects/DrawObject.h"

#include <mars_interfaces/NodeData.h>
#include <mars_interfaces/LightData.h>

#include <osg/Group>

namespace mars
{
    namespace graphics
    {
        /**
         * Wraps a NodeData in a osg::Group.
         * Handles previews and editing of NodeData groups.
         */
        class GraphcisManager;
        class OSGNodeStruct : public osg::Group
        {
        public:
            /**
             * Constructor creates a node in PREVIEW or CREATED state.
             * In PREVIEW state some ressources are not allocated (material,..)
             */
            OSGNodeStruct(GraphicsManager *g, const mars::interfaces::NodeData &node, bool isPreview, unsigned long id);
            virtual ~OSGNodeStruct();
            /**
             * Edit this node, works only in the PREVIEW state.
             */
            void edit(const mars::interfaces::NodeData &node, bool resize);

            inline DrawObject* object() {return drawObject_;}

            inline unsigned int id() const {return id_;}
            inline const std::string& name() const {return name_;}

        private:
            DrawObject *drawObject_;
            unsigned long id_;
            std::string name_;
            bool isPreview_;
        }; // end of class OSGNodeStruct

    } // end of namespace graphics
} // end of namespace mars
