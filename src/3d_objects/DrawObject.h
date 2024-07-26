#pragma once

#include <mars_interfaces/MARSDefs.h>
#include <mars_utils/Vector.h>
#include <mars_utils/Quaternion.h>
#include <mars_interfaces/LightData.h>
#include <mars_interfaces/MaterialData.h>

#include <string>
#include <vector>
#include <list>
#include <map>

#include <osg/Material>
#include <osg/Group>
#include <osg/PositionAttitudeTransform>
#include <osg/MatrixTransform>
#include <osg/Geode>
#include <osg/LOD>
#include <osg/Program>

#include <osg_material_manager/OsgMaterial.hpp>
#include <osg_material_manager/MaterialNode.hpp>
#include <osg_frames/Frame.hpp>


namespace mars
{
    namespace graphics
    {

        class GraphicsManager;

        class DrawObject
        {
        public:
            static osg::ref_ptr<osg::Material> selectionMaterial;

            DrawObject(GraphicsManager *g);
            virtual ~DrawObject();

            virtual void createObject(unsigned long id,
                                      const mars::utils::Vector &_pivot,
                                      unsigned long sharedID);

            void setStateFilename(const std::string &filename, int create);
            void exportState(void);
            void exportModel(const std::string &filename);

            // the material struct can also contain a static texture (texture file)
            virtual void setMaterial(const std::string &name);
            virtual void collideSphere(mars::utils::Vector pos,
                                       mars::interfaces::sReal radius);

            virtual void setPosition(const mars::utils::Vector &_pos);
            virtual void setQuaternion(const mars::utils::Quaternion &_q);
            virtual const mars::utils::Vector& getPosition()
                { return position_; }
            virtual const mars::utils::Quaternion& getQuaternion()
                { return quaternion_; }

            virtual void setScaledSize(const mars::utils::Vector &scaledSize);
            void setScale(const mars::utils::Vector &scale);

            void removeBits(unsigned int bits);
            void setBits(unsigned int bits);
            void setNodeMask(unsigned int mask)
                {
                    nodeMask_ = mask;
                    group_->setNodeMask(mask);
                }
            void setBrightness(double v);
            void setRenderBinNumber(int number);
            bool containsNode(osg::Node* node);

            virtual void setSelected(bool val);
            bool isSelected(void)
                {
                    return selected_;
                }
            void setSelectable(bool val)
                {
                    selectable_ = val;
                }
            void setShowSelected(bool val);
            unsigned long getID(void) const;
            void setID(unsigned long _id);
            void showNormals(bool val);

            osg::ref_ptr<osg::PositionAttitudeTransform> getTransform()
                {
                    return posTransform_;
                }
            osg::ref_ptr<osg::MatrixTransform> getScaleMatrix()
                {
                    return scaleTransform_;
                }

            osg::Group* getObject(void) const
                {
                    return group_.get();
                }
            osg::PositionAttitudeTransform* getPosTransform(void) const
                {
                    return posTransform_.get();
                }
            osg::MatrixTransform* getScaleTransform(void) const
                {
                    return scaleTransform_.get();
                }

            /**
             * Sets the line laser
             * @pos: position of the laser
             * @normal: normalvector of the laser-plane
             * @color: color of the laser in RGB
             * @laser: Angle of the laser, as an direction-vector
             * @openingAngle: Opening angle of the laser; for complete laserLine, choose PI
             */
            void setExperimentalLineLaser(utils::Vector lineLasePos,
                                          utils::Vector lineLaserNormal,
                                          utils::Vector lineLaserColor,
                                          utils::Vector LaserAngle,
                                          float openingAngle);

            void setMaxNumLights(int n) {maxNumLights = n;}
            void show();
            void hide();
            osg_material_manager::MaterialNode* getStateGroup() {return materialNode.get();}
            void addLODGeodes(std::list< osg::ref_ptr< osg::Geode > > geodes,
                              float start, float end);

            void seperateMaterial();
            void addSelectionChild(DrawObject* c) {selectionChilds.push_back(c);}

            osg_frames::Frame* frame;
        protected:
            unsigned long id_;
            unsigned int nodeMask_;

            std::string stateFilename_;
            bool selected_;
            bool selectable_;
            bool showSelected;
            osg::ref_ptr<osg_material_manager::MaterialNode> materialNode;
            osg::ref_ptr<osg::Group> group_;
            std::list< osg::ref_ptr<osg::Geometry> > geometry_;
            osg::ref_ptr<osg::Geode> normal_geode;
            osg::ref_ptr<osg::LOD> lod;
            osg::ref_ptr<osg::PositionAttitudeTransform> posTransform_;
            osg::ref_ptr<osg::MatrixTransform> scaleTransform_;
            std::vector<DrawObject*> selectionChilds;
            mars::utils::Vector position_;
            mars::utils::Vector pivot_;
            mars::utils::Vector geometrySize_;
            mars::utils::Vector scaledSize_;
            mars::utils::Quaternion quaternion_;
            osg::ref_ptr<osg::Program> selectShader;
            int maxNumLights;
            bool sharedStateGroup;
            bool isHidden;
            double brightness;
            GraphicsManager *graphicsManager;

            virtual std::list< osg::ref_ptr< osg::Geode > > createGeometry() = 0;
        }; // end of class DrawObject

    } // end of namespace graphics
} // end of namespace mars
