#include <osg/Node>
#include <osg/PositionAttitudeTransform>

#include "HUDOSGNode.h"
#include <stdio.h>

namespace mars
{
    namespace graphics
    {

        HUDOSGNode::HUDOSGNode(osg::ref_ptr<osg::Group> group)
            : HUDElement(),
              parent(group),
              scaleTransform(new osg::MatrixTransform),
              cull_mask(0xffffffff),
              render_order(10),
              visible(true)
        {
            scaleTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));
            if(parent.get() != NULL) parent->addChild(scaleTransform);
        }

        HUDOSGNode::HUDOSGNode()
            : HUDElement(),
              parent(NULL),
              scaleTransform(new osg::MatrixTransform),
              cull_mask(0xffffffff),
              render_order(10),
              visible(true)
        {
            scaleTransform->setMatrix(osg::Matrix::scale(1.0, 1.0, 1.0));
        }

        HUDOSGNode::~HUDOSGNode(void)
        {
        }

        void HUDOSGNode::setPos(double x, double y)
        {
            posx = x;
            posy = y;
        }

        void HUDOSGNode::setViewSize(double width, double height)
        {
            view_width = width;
            view_height = height;
        }

        void HUDOSGNode::setOSGNode(osg::Node* node)
        {
            //fprintf(stderr, "add osg node to hud element\n");
            scaleTransform->addChild(node);
        }

        osg::Group* HUDOSGNode::getNode(void)
        {
            if(parent.get() == NULL)
                return scaleTransform.get();
            else
                return parent.get();
        }

        void HUDOSGNode::resize(double _width, double _height)
        {
            double scale_x = _width / view_width;
            double scale_y = _height / view_height;

            scaleTransform->setMatrix(osg::Matrix::scale(scale_x, scale_y, 1.0));
        }

        void HUDOSGNode::switchCullMask()
        {
            if(visible)
            {
                visible = false;
                scaleTransform->setNodeMask(0);
            } else
            {
                visible = true;
                scaleTransform->setNodeMask(cull_mask);
            }
        }

        void HUDOSGNode::xorCullMask(unsigned int mask)
        {
            cull_mask = cull_mask^mask;
            scaleTransform->setNodeMask(cull_mask);
        }

    } // end of namespace graphics
} // end of namespace mars
