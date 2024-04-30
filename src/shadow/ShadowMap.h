/**
* \file ShadowMap.h
* \author Malte Langosz
* \brief The ShadowMap is a clone of the original osgShadow::ShadowMap but
*        allows to render the shadow texture in a given area of
*        a defined node
*/

#pragma once

#include <osg/Camera>
#include <osg/Material>
#include <osg/MatrixTransform>
#include <osg/Object>
#include <osgShadow/ShadowTechnique>

#include "DrawObject.h"

namespace mars
{
    namespace graphics
    {

        class ShadowMap  : public osgShadow::ShadowTechnique
        {

        public:
            ShadowMap(void);
            ShadowMap(const ShadowMap& copy,
                      const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);
            virtual ~ShadowMap(void) {}

            META_Object(mars_graphics, ShadowMap);

            void setLight(osg::Light* light);
            void setLight(osg::LightSource* ls);

            virtual void init();
            virtual void update(osg::NodeVisitor& nv);
            virtual void cull(osgUtil::CullVisitor& cv);
            virtual void cleanSceneGraph() {}

            void setCenterObject(DrawObject *cO)
                {
                    centerObject = cO;
                }

            void setRadius(double v)
                {
                    radius = v;
                }

            void setShadowTextureSize(int v)
                {
                    shadowTextureSize = v;
                }

            void initTexture();
            osg::Texture2D* getTexture()
                {
                    return texture.get();
                }
            void applyState(osg::StateSet *state);
            void removeState(osg::StateSet* state);
            void removeTexture(osg::StateSet* state);
            void addTexture(osg::StateSet* state);
            void updateTexScale();
            float getTexScale()
                {
                    return texscale;
                }

            virtual void resizeGLObjectBuffers(unsigned int maxSize);
            virtual void releaseGLObjects(osg::State* = 0) const;

        protected:
            virtual void createUniforms();

            osg::ref_ptr<osg::Camera> camera;
            osg::ref_ptr<osg::TexGen> texgen;
            osg::ref_ptr<osg::Image> image;
            osg::ref_ptr<osg::Texture2D> texture;
            osg::ref_ptr<osg::StateSet> stateset;
            osg::ref_ptr<osg::Light> light;
            osg::ref_ptr<osg::LightSource>  ls;
            DrawObject* centerObject;
            double radius;

            osg::ref_ptr<osg::Uniform> ambientBiasUniform;
            osg::ref_ptr<osg::Uniform> textureScaleUniform;
            osg::ref_ptr<osg::Uniform> texGenMatrixUniform;
            std::vector< osg::ref_ptr<osg::Uniform> > uniformList;
            unsigned int shadowTextureUnit;
            int shadowTextureSize;
            float texscale;
        }; // end of class ShadowMap

    } // end of namespace graphics
} // end of namespace mars
