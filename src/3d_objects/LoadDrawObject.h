#pragma once

#include "DrawObject.h"

#include <mars_utils/Vector.h>
#include <configmaps/ConfigData.h>

#include <string>
#include <list>

namespace mars
{
    namespace graphics
    {
        struct LoadDrawObjectPSetBox
        {
            osg::PrimitiveSet* primitiveSet;
            mars::utils::Vector min, max;
        };

        class LoadDrawObject : public DrawObject
        {

        public:
            LoadDrawObject(GraphicsManager *g, configmaps::ConfigMap &map,
                           const mars::utils::Vector &ext);

        protected:
            configmaps::ConfigMap info_;
            std::vector<std::vector<LoadDrawObjectPSetBox*>*> gridPSets_;

            virtual std::list< osg::ref_ptr< osg::Geode > > createGeometry();

        private:
            std::list< osg::ref_ptr< osg::Geode > > loadGeodes(std::string filename,
                                                               std::string objname);
            void checkBobj(std::string &filename);
        };

    } // end of namespace graphics
} // end of namespace mars
