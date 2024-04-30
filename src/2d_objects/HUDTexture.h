#pragma once

#include "HUDElement.h"

#define TEXTURE_UNKNOWN 0
#define TEXTURE_IMAGE   1
#define TEXTURE_RTT     2

#include <osg/MatrixTransform>

namespace mars
{
    namespace graphics
    {

        class HUDTexture : public HUDElement
        {

        public:
            HUDTexture(osg::Group *group);
            HUDTexture(void);
            ~HUDTexture(void);
  
            void setSize(double width, double height);
            void setTextureSize(double width, double height);
            void setPos(double x, double y);
            void setBorderColor(double r, double g, double b, double a);
            void setBorderWidth(double border_width);
  
            void createBox(void);
            osg::Group* getNode(void);
            void switchCullMask();
            void xorCullMask(unsigned int mask);
            void setImageData(void *data);
            void setTexture(osg::Texture2D *texture = 0);

        private:
            osg::ref_ptr<osg::Group> parent;
            osg::ref_ptr<osg::MatrixTransform> scaleTransform;
            osg::ref_ptr<osg::PositionAttitudeTransform> posTransform;
            osg::ref_ptr<osg::Image> image;
            osg::ref_ptr<osg::Texture2D> texture;
            osg::ref_ptr<osg::Geode> geode;

            double width, height, t_width, t_height;
            double posx, posy;
            double border_color[4];
            double border_width;
            unsigned int cull_mask;
            bool visible;
            unsigned int texture_type;
        }; // end of class HUDTexture

    } // end of namespace graphics
} // end of namespace mars

