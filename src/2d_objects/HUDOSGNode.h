#pragma once

#include "HUDElement.h"

#include <osg/MatrixTransform>
#include <string>
#include <vector>

namespace mars
{
    namespace graphics
    {

        class HUDOSGNode : public HUDElement
        {

        public:
            HUDOSGNode(osg::ref_ptr<osg::Group> group);
            HUDOSGNode(void);
            ~HUDOSGNode(void);
  
            void setPos(double x, double y);
            void setViewSize(double widht, double height);
            void setOSGNode(osg::Node* node);
            void resize(double _width, double _height);
            void setRenderOrder(int val) {render_order = val;}
            osg::Group* getNode(void);
            void switchCullMask(void);
            void xorCullMask(unsigned int mask);
  
        private:
            osg::ref_ptr<osg::Group> parent;
            osg::ref_ptr<osg::MatrixTransform> scaleTransform;

            double view_width, view_height;
            unsigned int cull_mask;
            int render_order;
            bool visible, init;
            double posx, posy;
        }; // end of class HUDOSGNode

    } // end of namespace graphics
} // end of namespace mars

