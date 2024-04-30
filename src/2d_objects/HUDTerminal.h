#pragma once

#include "HUDElement.h"

#include <osg/MatrixTransform>
#include <string>

namespace mars
{
    namespace graphics
    {

        class HUDTerminal : public HUDElement
        {

        public:
            HUDTerminal(void);
            ~HUDTerminal(void);
  
            void setSize(double width, double height);
            void setFontSize(double _font_size);
            void setMaxCaracters(int size);
            void setLineSpacing(double _line_spacing);
            void setPos(double x, double y);
            void setViewSize(double widht, double height);
            void setBackgroundColor(double r, double g, double b, double a);
            void setBorderColor(double r, double g, double b, double a);
            void setBorderWidth(double border_width);
  
            void createBox(void);
            void addText(std::string text, double color[4]);
            void resize(double _width, double _height);
            osg::Group* getNode(void);
            void switchCullMask();
            void xorCullMask(unsigned int mask);

        private:
            osg::ref_ptr<osg::Group> hudBox;
            osg::ref_ptr<osg::Group> hudTerminalList;

            double width, height, view_width, view_height;
            double posx, posy;
            double background_color[4], border_color[4];
            double border_width;
            double font_size;
            double line_spacing;
            unsigned int row_index;
            int max_caracters;
            unsigned int cull_mask;
            bool visible;

            osg::ref_ptr<osg::Geode> createLabel(double pos,
                                                 std::string label,
                                                 double color[4]);
        }; // end of class HUDTerminal

    } // end of namespace graphics
} // end of namespace mars

