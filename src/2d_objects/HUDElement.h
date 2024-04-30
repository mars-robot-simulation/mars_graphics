#pragma once

#include <osg/Group>
#include <string>

namespace mars
{
    namespace graphics
    {

        class HUDElement
        {

        public:
            HUDElement(void) {}
            virtual ~HUDElement(void) {}
            virtual osg::Group* getNode(void) = 0;
            virtual void switchCullMask() = 0;
            virtual void xorCullMask(unsigned int mask) = 0;
            virtual void setID(unsigned long id) {this->id = id;}
            virtual unsigned long getID(void) {return id;}
            virtual void setConfigPath(std::string path) {config_path = path;}
            virtual void setPos(double x, double y) = 0;

            static int elemCount;

        private:
            unsigned long id;
        protected:
            std::string config_path;
        };

    } // end of namespace graphics
} // end of namespace mars
