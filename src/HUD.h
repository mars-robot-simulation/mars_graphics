#pragma once

#include <osg/Texture2D>
#include <osg/Camera>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osgViewer/GraphicsWindow>

#include <mars_interfaces/MARSDefs.h>
#include <mars_utils/Vector.h>
#include <mars_utils/Quaternion.h>
#include <mars_utils/Color.h>

#include "2d_objects/HUDElement.h"


namespace mars
{
    namespace graphics
    {

        class HUD
        {

        public:
            HUD(unsigned long id);
            ~HUD(void);

            void init(osgViewer::GraphicsWindow *gw);
            void init(mars::interfaces::sReal width, mars::interfaces::sReal height);

            osg::ref_ptr<osg::Texture2D> getTexture(void);
            osg::ref_ptr<osg::Camera> getCamera(void);
            void getSize(mars::interfaces::sReal &width, mars::interfaces::sReal &height);
            void setViewSize(double width, double height);
            void getOffset(mars::utils::Vector &offset);
            void resize(double width, double height);
            void setCullMask(unsigned int cull_mask);
            void addHUDElement(HUDElement *elem);
            void removeHUDElement(HUDElement *elem);
            void switchCullElement(int key);
            void switchElementVis(int num_element);
            void setViewOffsets(double x1, double y1, double x2, double y2);

        private:
            osg::ref_ptr<osg::Camera> hudCamera;
            osg::ref_ptr<osg::Group> hudTerminalList;
            osg::ref_ptr<osg::MatrixTransform> scaleTransform;
            std::vector<HUDElement*> elements;
            mars::interfaces::sReal swidth;
            mars::interfaces::sReal sheight;
            unsigned long id;
            mars::utils::Color myColor;
            mars::utils::Vector myoff;
            unsigned int width, height;
            double view_width, view_height;
            double x1, x2, y1, y2;
            unsigned int row_index;
            unsigned int cull_mask;
            void initialize(osgViewer::GraphicsWindow* gw);

        }; // end of class HUD

    } // end of namespace graphics
} // end of namespace mars
