#include "GridPrimitive.h"
#include "../wrapper/OSGDrawItem.h"

#include <mars_interfaces/MARSDefs.h>
#include <mars_utils/Vector.h>
#include <mars_interfaces/graphics/draw_structs.h>

namespace mars
{
    namespace graphics
    {

        using mars::utils::Vector;
        using mars::utils::Color;

        GridPrimitive::GridPrimitive(GraphicsWidget *gw)
        {
            double x, y;
            Vector start, end;
            Color myColor;

            myColor.r = 1.0;
            myColor.g = 1.0;
            myColor.b = 1.0;
            myColor.a = 0.8;

            for(x=-2; x<3; x++)
            {
                for(y=-2; y<3; y++)
                {
                    start.x() = end.x() = x;
                    start.y() = end.y() = y;
                    start.z() = 0;
                    end.z() = 2;
                    osg::ref_ptr<OSGDrawItem> osgNode = new OSGDrawItem(gw);
                    OSGDrawItem::createLine(osgNode.get(), start, end, myColor);
                    addChild(osgNode.get());
                }
            }
            myColor.r = 0.0;
            myColor.g = 1.0;
            myColor.b = 0.0;
            for(x=-2; x<3; x++)
            {
                for(y=1; y<3; y++)
                {
                    start.x() = end.x() = x;
                    start.z() = end.z() = y;
                    start.y() = 2;
                    end.y() = -2;
                    osg::ref_ptr<OSGDrawItem> osgNode = new OSGDrawItem(gw);
                    OSGDrawItem::createLine(osgNode, start, end, myColor);
                    addChild(osgNode.get());
                }
            }
            myColor.r = 0.0;
            myColor.g = 0.0;
            myColor.b = 1.0;
            for(x=-2; x<3; x++)
            {
                for(y=1; y<3; y++)
                {
                    start.y() = end.y() = x;
                    start.z() = end.z() = y;
                    start.x() = 2;
                    end.x() = -2;
                    osg::ref_ptr<OSGDrawItem> osgNode = new OSGDrawItem(gw);
                    OSGDrawItem::createLine(osgNode, start, end, myColor);
                    addChild(osgNode.get());
                }
            }

            myColor.r = 0.0;
            myColor.g = 1.0;
            myColor.b = 0.0;
            for(x=-1.9; x<2.1; x+=0.1)
            {
                for(y=0; y<1; y++)
                {
                    start.x() = end.x() = x;
                    start.z() = end.z() = y+0.01;
                    start.y() = 2;
                    end.y() = -2;
                    osg::ref_ptr<OSGDrawItem> osgNode = new OSGDrawItem(gw);
                    OSGDrawItem::createLine(osgNode, start, end, myColor);
                    addChild(osgNode.get());
                }
            }
            myColor.r = 0.0;
            myColor.g = 0.0;
            myColor.b = 1.0;
            for(x=-1.9; x<2.1; x+=0.1)
            {
                for(y=0; y<1; y++)
                {
                    start.y() = end.y() = x;
                    start.z() = end.z() = y+0.01;
                    start.x() = 2;
                    end.x() = -2;
                    osg::ref_ptr<OSGDrawItem> osgNode = new OSGDrawItem(gw);
                    OSGDrawItem::createLine(osgNode, start, end, myColor);
                    addChild(osgNode.get());
                }
            }

            osg::StateSet *stateset = getOrCreateStateSet();
            stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        }

    } // end of namespace graphics
} // end of namespace mars
