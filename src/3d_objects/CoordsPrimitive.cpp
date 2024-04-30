#include "CoordsPrimitive.h"
#include "gui_helper_functions.h"
#include "../wrapper/OSGDrawItem.h"

namespace mars
{
    namespace graphics
    {

        using namespace std;
        using mars::utils::Vector;

        CoordsPrimitive::CoordsPrimitive(GraphicsWidget *gw, const Vector &size,
                                         std::string resPath,
                                         bool transformFlag)
        {

            string objPath = resPath;
            objPath.append("/Objects/coords.3ds");
            string fontPath = resPath;
            fontPath.append("/Fonts");

            osg::ref_ptr<osg::Node> coordsCompleteNode;
            osg::ref_ptr<OSGDrawItem> group;

            coordsCompleteNode = GuiHelper::readNodeFromFile(objPath);

            //add child to parent osg groups
            if(transformFlag)
            {
                // FIXME: scaling of coordsCompleteNode wrong!
                addChild(coordsCompleteNode.get());

                group = new OSGDrawItem(gw);
                OSGDrawItem::createLabel(group, Vector(size.x()/2+0.2, 0.0,0.0), 0.1,"X",
                                         fontPath);
                addChild(group.get());

                group = new OSGDrawItem(gw);
                OSGDrawItem::createLabel(group, Vector(0.0, size.y()/2+0.2, 0.0), 0.1,"Y",
                                         fontPath);
                addChild(group.get());

                group = new OSGDrawItem(gw);
                OSGDrawItem::createLabel(group, Vector(0.0,0.0, size.z()/2+0.2), 0.1,"Z",
                                         fontPath);
                addChild(group.get());
            } else
            {
                addChild(coordsCompleteNode.get());

                group = new OSGDrawItem(gw);
                OSGDrawItem::createLabel(group, Vector(1.1,0.0,0.0),0.1,"X", fontPath);
                addChild(group.get());

                group = new OSGDrawItem(gw);
                OSGDrawItem::createLabel(group, Vector(0.0,1.1,0.0),0.1,"Y", fontPath);
                addChild(group.get());

                group = new OSGDrawItem(gw);
                OSGDrawItem::createLabel(group, Vector(0.0,0.0,1.1),0.1,"Z", fontPath);
                addChild(group.get());
            }
        }

        CoordsPrimitive::CoordsPrimitive(GraphicsWidget *gw,
                                         std::string resPath)
        {

            string objPath = resPath;
            objPath.append("/Objects/coords.3ds");

            osg::ref_ptr<osg::Node> coordsCompleteNode;
            osg::ref_ptr<OSGDrawItem> group;

            coordsCompleteNode = GuiHelper::readNodeFromFile(objPath);

            addChild(coordsCompleteNode.get());

            group = new OSGDrawItem(gw);
            string fontPath = resPath;
            fontPath.append("/Fonts");
            OSGDrawItem::createLabel(group, Vector(1.1,0.0,0.0),0.1,"X", fontPath);
            addChild(group.get());

            group = new OSGDrawItem(gw);
            OSGDrawItem::createLabel(group, Vector(0.0,1.1,0.0),0.1,"Y", fontPath);
            addChild(group.get());

            group = new OSGDrawItem(gw);
            OSGDrawItem::createLabel(group, Vector(0.0,0.0,1.1),0.1,"Z", fontPath);
            addChild(group.get());
        }

    } // end of namespace graphics
} // end of namespace mars
