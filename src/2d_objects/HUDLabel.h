#pragma once
#include "HUDElement.h"

#include <osg/MatrixTransform>
#include <osgText/Text>
#include <string>

namespace mars
{
    namespace graphics
    {

        class HUDLabel : public HUDElement
        {

        public:
            HUDLabel(osg::ref_ptr<osg::Group> group);
            HUDLabel(void);
            ~HUDLabel(void);
  
            void setFontSize(double _font_size);
            void setPos(double x, double y);
            void setBackgroundColor(double r, double g, double b, double a);
            void setBorderColor(double r, double g, double b, double a);
            void setBorderWidth(double border_width);
            void setPadding(double left, double top, double right, double bottom);
            void setDirection(int _direction);
            void setText(std::string text, double color[4]);
            osg::Group* getNode(void);
            void switchCullMask(void);
            void xorCullMask(unsigned int mask);

        private:
            osg::ref_ptr<osg::Group> parent;
            osg::ref_ptr<osg::MatrixTransform> scaleTransform;
            osg::ref_ptr<osgText::Text> labelText;

            double posx, posy;
            double background_color[4], border_color[4];
            double border_width;
            double font_size;
            double pl, pt, pr, pb;
            int direction;
            bool label_created;
            unsigned int cull_mask;
            bool visible;

            osg::ref_ptr<osg::Geode> createLabel(std::string label,
                                                 double color[4]);

        }; // end of class HUDLabel

    } // end of namespace graphics
} // end of namespace mars
