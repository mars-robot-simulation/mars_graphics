#ifdef __OCEAN__

#pragma once

#include "DrawObject.h"
#include "SkyDome.h"

#include <mars_utils/Vector.h>

#include <string>
#include <osg/TextureCubeMap>
#include <osgDB/ReadFile>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/PositionAttitudeTransform>
#include <osg/Program>
#include <osgText/Text>
#include <osg/CullFace>
#include <osg/Fog>
#include <osgText/Font>
#include <osg/Switch>
#include <osg/Texture3D>
#include <osgViewer/View>
#include <osgOcean/Version>
#include <osgOcean/OceanScene>
#include <osgOcean/FFTOceanSurface>
#include <osgOcean/SiltEffect>
#include <osgOcean/ShaderManager>

namespace mars
{
    namespace graphics
    {

        class OceanDrawObject : public DrawObject
        {

        public:
            enum SCENE_TYPE{ CLEAR, DUSK, CLOUDY };

            OceanDrawObject();
            virtual ~OceanDrawObject(void);

            virtual void createObject(unsigned long id_, void* data,
                                      const mars::base::Vector *_pivot = 0);
            virtual void exportObject(void);
            void setViewer(osgViewer::View* view);
            void addScene(osg::Group* scene);
            osg::Group* getScene(void);

        private:
            SCENE_TYPE _sceneType;
            osg::Vec2f windDirection;
            float windSpeed;
            float depth;
            float reflectionDamping;
            float scale;
            bool  isChoppy;
            float choppyFactor;
            float crestFoamHeight;
            float waveScale;

            std::vector<std::string> _cubemapDirs;
            std::vector<osg::Vec4f>  _lightColors;
            std::vector<osg::Vec4f>  _fogColors;
            std::vector<osg::Vec3f>  _underwaterAttenuations;
            std::vector<osg::Vec4f>  _underwaterDiffuse;

            std::vector<osg::Vec3f>  _sunPositions;
            std::vector<osg::Vec4f>  _sunDiffuse;
            std::vector<osg::Vec4f>  _waterFogColors;

            osg::ref_ptr<osg::TextureCubeMap> _cubemap;
            osg::ref_ptr<osgOcean::FFTOceanSurface> _oceanSurface;
            osg::ref_ptr<osgOcean::OceanScene> _oceanScene;
            osg::ref_ptr<SkyDome> _skyDome;

            osg::ref_ptr<osg::TextureCubeMap> loadCubeMapTextures(const std::string& dir);
            osg::Vec4f intColor(unsigned int r, unsigned int g,
                                unsigned int b, unsigned int a = 255);
        }; // end of class OceanDrawObject

    } // end of namespace graphics
} // end of namespace mars

#endif // __OCEAN__
