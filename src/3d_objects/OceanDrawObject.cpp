#ifdef __OCEAN__

#include "OceanDrawObject.h"

namespace mars
{
    namespace graphics
    {

        class CameraTrackCallback: public osg::NodeCallback
        {
        public:
            virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
                {
                    if( nv->getVisitorType() == osg::NodeVisitor::CULL_VISITOR )
                    {
                        osgUtil::CullVisitor* cv = static_cast<osgUtil::CullVisitor*>(nv);
                        osg::Vec3f centre,up,eye;
                        // get MAIN camera eye,centre,up
                        cv->getRenderStage()->getCamera()->getViewMatrixAsLookAt(eye,centre,up);
                        // update position
                        osg::MatrixTransform* mt = static_cast<osg::MatrixTransform*>(node);
                        mt->setMatrix( osg::Matrix::translate( eye.x(), eye.y(), mt->getMatrix().getTrans().z() ) );
                    }

                    traverse(node, nv); 
                }
        };


        OceanDrawObject::OceanDrawObject() : DrawObject()
        {
            // nothing to do here so far

            _cubemapDirs.push_back( "sky_clear" );
            _cubemapDirs.push_back( "sky_dusk" );
            _cubemapDirs.push_back( "sky_fair_cloudy" );

            _fogColors.push_back( intColor( 199,226,255 ) );
            _fogColors.push_back( intColor( 244,228,179 ) );
            _fogColors.push_back( intColor( 172,224,251 ) );

            _waterFogColors.push_back( intColor(27,57,109) );
            _waterFogColors.push_back( intColor(44,69,106 ) );
            _waterFogColors.push_back( intColor(84,135,172 ) );

            _underwaterAttenuations.push_back( osg::Vec3f(0.015f, 0.0075f, 0.005f) );
            _underwaterAttenuations.push_back( osg::Vec3f(0.015f, 0.0075f, 0.005f) );
            _underwaterAttenuations.push_back( osg::Vec3f(0.008f, 0.003f, 0.002f) );

            _underwaterDiffuse.push_back( intColor(27,57,109) );
            _underwaterDiffuse.push_back( intColor(44,69,106) );
            _underwaterDiffuse.push_back( intColor(84,135,172) );

            _lightColors.push_back( intColor( 105,138,174 ) );
            _lightColors.push_back( intColor( 105,138,174 ) );
            _lightColors.push_back( intColor( 105,138,174 ) );

            _sunPositions.push_back( osg::Vec3f(326.573, 1212.99 ,1275.19) );
            _sunPositions.push_back( osg::Vec3f(520.f, 1900.f, 550.f ) );
            _sunPositions.push_back( osg::Vec3f(-1056.89f, -771.886f, 1221.18f ) );

            _sunDiffuse.push_back( intColor( 191, 191, 191 ) );
            _sunDiffuse.push_back( intColor( 251, 251, 161 ) );
            _sunDiffuse.push_back( intColor( 191, 191, 191 ) );
  
            windDirection = osg::Vec2f(1.0f,1.0f);
            windSpeed = 12.f;
            depth = 10000.f;
            reflectionDamping = 0.35f;
            scale = 1e-8;
            isChoppy = true;
            choppyFactor = -2.5f;
            crestFoamHeight = 2.2f;
            _sceneType = CLOUDY;
            waveScale = 1e-8;

            osgOcean::ShaderManager::instance().enableShaders(true);

        }

        OceanDrawObject::~OceanDrawObject(void)
        {

        }

        // have to get a Vector with data for the length of the edges
        void OceanDrawObject::createObject(unsigned long id_, void *data,
                                           const mars::base::Vector *_pivot)
        {
            group_ = new osg::Group;

            _cubemap = loadCubeMapTextures( _cubemapDirs[_sceneType] );

            _oceanSurface = new osgOcean::FFTOceanSurface( 64, 256, 17, 
                                                           windDirection, windSpeed,
                                                           depth, reflectionDamping,
                                                           waveScale, isChoppy,
                                                           choppyFactor, 10.f, 256 );  

            _oceanSurface->setEnvironmentMap( _cubemap.get() );
            _oceanSurface->setFoamBottomHeight( 2.2f );
            _oceanSurface->setFoamTopHeight( 3.0f );
            _oceanSurface->enableCrestFoam( true );
            _oceanSurface->setLightColor( _lightColors[_sceneType] );
            // Make the ocean surface track with the main camera position, giving the illusion
            // of an endless ocean surface.
            _oceanSurface->enableEndlessOcean(true);

            osg::Vec3f sunDir = -_sunPositions[_sceneType];
            sunDir.normalize();
                
            _oceanScene = new osgOcean::OceanScene( _oceanSurface.get() );
            _oceanScene->setLightID(0);
            _oceanScene->enableReflections(true);
            _oceanScene->enableRefractions(true);

            // Set the size of _oceanCylinder which follows the camera underwater. 
            // This cylinder prevents the clear from being visible past the far plane 
            // instead it will be the fog color.
            // The size of the cylinder should be changed according the size of the ocean surface.
            _oceanScene->setCylinderSize( 1900.f, 4000.f );
  
            _oceanScene->setAboveWaterFog(0.0012f, _fogColors[_sceneType] );
            _oceanScene->setUnderwaterFog(0.002f,  _waterFogColors[_sceneType] );
            _oceanScene->setUnderwaterDiffuse( _underwaterDiffuse[_sceneType] );
            _oceanScene->setUnderwaterAttenuation( _underwaterAttenuations[_sceneType] );
  
            _oceanScene->setSunDirection( sunDir );
            _oceanScene->enableGodRays(true);
            _oceanScene->enableSilt(true);
            _oceanScene->enableUnderwaterDOF(true);
            _oceanScene->enableDistortion(true);
            _oceanScene->enableGlare(false);
            _oceanScene->setGlareAttenuation(0.8f);
  
            _skyDome = new SkyDome( 1900.f, 16, 16, _cubemap.get() );
            _skyDome->setNodeMask( _oceanScene->getReflectedSceneMask() | _oceanScene->getNormalSceneMask() );

            // add a pat to track the camera
            osg::MatrixTransform* transform = new osg::MatrixTransform;
            transform->setDataVariance( osg::Object::DYNAMIC );
            transform->setMatrix( osg::Matrixf::translate( osg::Vec3f(0.f, 0.f, 0.f) ));
            transform->setCullCallback( new CameraTrackCallback );
            transform->addChild( _skyDome.get() );
            _oceanScene->addChild( transform );


            // Create and add fake texture for use with nodes without any texture
            // since the OceanScene default scene shader assumes that texture unit 
            // 0 is used as a base texture map.
            osg::Image * image = new osg::Image;
            image->allocateImage( 1, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE );
            *(osg::Vec4ub*)image->data() = osg::Vec4ub( 0xFF, 0xFF, 0xFF, 0xFF );
  
            osg::Texture2D* fakeTex = new osg::Texture2D( image );
            fakeTex->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::REPEAT);
            fakeTex->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::REPEAT);
            fakeTex->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::NEAREST);
            fakeTex->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::NEAREST);
  
            osg::StateSet* stateset = _oceanScene->getOrCreateStateSet();
            stateset->setTextureAttribute(0,fakeTex,osg::StateAttribute::ON);
            stateset->setTextureMode(0,GL_TEXTURE_1D,osg::StateAttribute::OFF);
            stateset->setTextureMode(0,GL_TEXTURE_2D,osg::StateAttribute::ON);
            stateset->setTextureMode(0,GL_TEXTURE_3D,osg::StateAttribute::OFF);
  
            posTransform_ = new osg::PositionAttitudeTransform();
            posTransform_->setPosition(osg::Vec3(0.0, 0.0, 0.0));
            posTransform_->addChild(_oceanScene.get());
            _oceanScene->setScreenDims(osg::Vec2s(1920, 1200));
        }

        void OceanDrawObject::exportObject(void)
        {
            // will be implemented later
        }

        osg::ref_ptr<osg::TextureCubeMap> OceanDrawObject::loadCubeMapTextures(const std::string& dir)
        {
            enum {POS_X, NEG_X, POS_Y, NEG_Y, POS_Z, NEG_Z};
  
            std::string filenames[6];
  
            filenames[POS_X] = "resources/textures/" + dir + "/east.png";
            filenames[NEG_X] = "resources/textures/" + dir + "/west.png";
            filenames[POS_Z] = "resources/textures/" + dir + "/north.png";
            filenames[NEG_Z] = "resources/textures/" + dir + "/south.png";
            filenames[POS_Y] = "resources/textures/" + dir + "/down.png";
            filenames[NEG_Y] = "resources/textures/" + dir + "/up.png";
  
            osg::ref_ptr<osg::TextureCubeMap> cubeMap = new osg::TextureCubeMap;
            cubeMap->setInternalFormat(GL_RGBA);
  
            cubeMap->setFilter(osg::Texture::MIN_FILTER,
                               osg::Texture::LINEAR_MIPMAP_LINEAR);
            cubeMap->setFilter(osg::Texture::MAG_FILTER,
                               osg::Texture::LINEAR);
            cubeMap->setWrap(osg::Texture::WRAP_S,
                             osg::Texture::CLAMP_TO_EDGE);
            cubeMap->setWrap(osg::Texture::WRAP_T,
                             osg::Texture::CLAMP_TO_EDGE);
  
            cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_X,
                              osgDB::readImageFile( filenames[NEG_X] ) );
            cubeMap->setImage(osg::TextureCubeMap::POSITIVE_X,
                              osgDB::readImageFile( filenames[POS_X] ) );
            cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Y,
                              osgDB::readImageFile( filenames[NEG_Y] ) );
            cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Y,
                              osgDB::readImageFile( filenames[POS_Y] ) );
            cubeMap->setImage(osg::TextureCubeMap::NEGATIVE_Z,
                              osgDB::readImageFile( filenames[NEG_Z] ) );
            cubeMap->setImage(osg::TextureCubeMap::POSITIVE_Z,
                              osgDB::readImageFile( filenames[POS_Z] ) );
  
            return cubeMap;
        }

        osg::Vec4f OceanDrawObject::intColor(unsigned int r, unsigned int g,
                                             unsigned int b, unsigned int a)
        {
            float div = 1.f/255.f;
            return osg::Vec4f(div*(float)r, div*(float)g, div*float(b), div*(float)a);
        }

        void OceanDrawObject::setViewer(osgViewer::View* view)
        {
            view->addEventHandler(_oceanScene->getEventHandler());
            view->addEventHandler(_oceanSurface->getEventHandler());
        }

        void OceanDrawObject::addScene(osg::Group* scene)
        {
            _oceanScene->addChild(scene);
        }

        osg::Group* OceanDrawObject::getScene(void)
        {
            return _oceanScene.get();
        }

    } // end of namespace graphics
} // end of namespace mars

#endif // __OCEAN__
