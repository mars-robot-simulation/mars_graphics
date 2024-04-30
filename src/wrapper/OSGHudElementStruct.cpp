#include "OSGHudElementStruct.h"

#include "../2d_objects/HUDLabel.h"
#include "../2d_objects/HUDLines.h"
#include "../2d_objects/HUDTexture.h"
#include "../2d_objects/HUDOSGNode.h"

namespace mars
{
    namespace graphics
    {

        using namespace interfaces;

        int HUDElement::elemCount = 1;

        OSGHudElementStruct::OSGHudElementStruct(const hudElementStruct &he,
                                                 const std::string &config_path,
                                                 unsigned int id, osg::Node* node)
            : elem_(NULL)
        {
            switch (he.type)
            {
            case HUD_ELEMENT_LABEL:
            {
                HUDLabel *myLabel = new HUDLabel(this);
                myLabel->setConfigPath(config_path);
                myLabel->setID(id);
                myLabel->setPos(he.posx, he.posy);
                myLabel->setBackgroundColor(he.background_color[0],
                                            he.background_color[1],
                                            he.background_color[2],
                                            he.background_color[3]);
                myLabel->setBorderColor(he.border_color[0], he.border_color[1],
                                        he.border_color[2], he.border_color[3]);
                myLabel->setBorderWidth(he.border_width);
                myLabel->setFontSize(he.font_size);
                myLabel->setPadding(he.padding[0], he.padding[1],
                                    he.padding[2], he.padding[3]);
                myLabel->setDirection(he.direction);
                elem_ = myLabel;
                break;
            }
            case HUD_ELEMENT_LINES:
            {
                HUDLines *myLines = new HUDLines(this);
                myLines->setID(id);
                myLines->setPos(he.posx, he.posy);
                myLines->setBackgroundColor(he.background_color[0],
                                            he.background_color[1],
                                            he.background_color[2],
                                            he.background_color[3]);
                myLines->setBorderColor(he.border_color[0], he.border_color[1],
                                        he.border_color[2], he.border_color[3]);
                myLines->setBorderWidth(he.border_width);
                myLines->setPadding(he.padding[0], he.padding[1],
                                    he.padding[2], he.padding[3]);
                myLines->setPointSize(he.font_size);
                if(he.direction > 0) myLines->setRenderOrder(he.direction);
                elem_ = myLines;
                break;
            }
            case HUD_ELEMENT_TERMINAL:
                break;
            case HUD_ELEMENT_TEXTURE:
            {
                HUDTexture *myTexture = new HUDTexture(this);
                myTexture->setID(id);
                myTexture->setSize(he.width, he.height);
                myTexture->setTextureSize(he.texture_width, he.texture_height);
                myTexture->setPos(he.posx, he.posy);
                myTexture->setBorderColor(he.border_color[0], he.border_color[1],
                                          he.border_color[2], he.border_color[3]);
                myTexture->setBorderWidth(he.border_width);
                myTexture->createBox();
                elem_ = myTexture;
                break;
            }
            case HUD_ELEMENT_OSGNODE:
            {
                HUDOSGNode *myNode = new HUDOSGNode(this);
                myNode->setID(id);
                myNode->setOSGNode(node);
                elem_ = myNode;
                break;
            }
            }
            osg::StateSet *state = elem_->getNode()->getOrCreateStateSet();
            state->setRenderBinDetails(HUDElement::elemCount++, "RenderBin");
        }

        OSGHudElementStruct::~OSGHudElementStruct()
        {
            if(elem_ != NULL) delete elem_;
        }

        HUDElement* OSGHudElementStruct::getHUDElement()
        {
            return elem_;
        }

    } // end of namespace graphics
} // end of namespace mars
