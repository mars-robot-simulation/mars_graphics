#include "GraphicsManager.h"
#include "config.h"
#include <mars_utils/misc.h>

#include <osgDB/WriteFile>
#include <osg/Fog>
#include <osg/LightModel>

#include <osgParticle/FireEffect>
#include <osgParticle/SmokeEffect>
#include <osgParticle/SmokeTrailEffect>
#include <osgParticle/FireEffect>

#include <mars_utils/misc.h>

#include "3d_objects/GridPrimitive.h"
#include "3d_objects/DrawObject.h"
#include "3d_objects/CoordsPrimitive.h"
#include "3d_objects/AxisPrimitive.h"

#include "2d_objects/HUDLabel.h"
#include "2d_objects/HUDTerminal.h"
#include "2d_objects/HUDLines.h"
#include "2d_objects/HUDTexture.h"

#include "wrapper/OSGLightStruct.h"
#include "wrapper/OSGMaterialStruct.h"
#include "wrapper/OSGDrawItem.h"
#include "wrapper/OSGHudElementStruct.h"

#include "GraphicsWidget.h"
#include "HUD.h"

#include "wrapper/OSGNodeStruct.h"
#include "QtOsgMixGraphicsWidget.h"

#include <iostream>
#include <cassert>
#include <stdexcept>

#define SINGLE_THREADED
#define NUM_PSSM_SPLITS 3

using namespace osg_material_manager;
using namespace configmaps;


namespace mars
{
    namespace graphics
    {

        using namespace std;
        using mars::utils::Vector;
        using mars::utils::Quaternion;
        using namespace mars::interfaces;
        using namespace utils;

        static int ReceivesShadowTraversalMask = 0x10000000;
        static int CastsShadowTraversalMask = 0x20000000;


        GraphicsManager::GraphicsManager(lib_manager::LibManager *theManager,
                                         void *myQTWidget)
            : GraphicsManagerInterface(theManager),
              osgWidget{nullptr},
              guiHelper{new GuiHelper{this}},
              next_hud_id{1},
              next_draw_object_id{1},
              next_window_id{1},
              nextPreviewID{1},
              viewer{nullptr},
              scene{new osg::Group},
              shadowedScene{new osgShadow::ShadowedScene},
              lightGroup{new osg::Group},
              globalStateset{new osg::StateSet},
              grid{nullptr},
              show_grid{false},
              showClouds_{false},
              show_coords{true},
              useFog{true},
              useNoise{false},
              drawLineLaser{false},
              cfg{nullptr},
              ignore_next_resize{0},
              set_window_prop{false},
              initialized{false},
              activeWindow{nullptr},
              materialManager{nullptr}
        {
            //osg::setNotifyLevel( osg::WARN );

            // first check if we have the cfg_manager lib
            framesFactory = new osg_frames::FramesFactory();
            dataBroker = libManager->getLibraryAs<data_broker::DataBrokerInterface>("data_broker");
            preTime = avgPreTime = 0;
            updateTime = avgUpdateTime = 0;
            lightTime = avgLightTime = 0;
            materialTime = avgMaterialTime = 0;
            frameTime = avgFrameTime = 0;
            postTime = avgPostTime = 0;
            avgTimeCount = 0;
            dbPackageMapping.add("preTime", &avgPreTime);
            dbPackageMapping.add("updateTime", &avgUpdateTime);
            dbPackageMapping.add("lightTime", &avgLightTime);
            dbPackageMapping.add("materialTime", &avgMaterialTime);
            dbPackageMapping.add("frameTime", &avgFrameTime);
            dbPackageMapping.add("postTime", &avgPostTime);

            if(dataBroker)
            {
                std::string groupName, dataName;
                groupName = "MarsGraphcis";
                dataName = "debugTime";
                // initialize the dataBroker Package
                data_broker::DataPackage dbPackage;
                dbPackageMapping.writePackage(&dbPackage);
                dataBroker->pushData(groupName, dataName, dbPackage, nullptr,
                                     data_broker::DATA_PACKAGE_READ_FLAG);
                // register as producer
                dataBroker->registerTimedProducer(this, groupName, dataName,
                                                  "_REALTIME_", 100);
            }
        }

        GraphicsManager::~GraphicsManager()
        {
            if(cfg)
            {
                string saveFile = configPath.sValue;
                saveFile.append("/mars_Graphics.yaml");
                cfg->writeConfig(saveFile.c_str(), "Graphics");
                libManager->releaseLibrary("cfg_manager");
            }

            if(materialManager)
            {
                libManager->releaseLibrary("osg_material_manager");
            }

            //fprintf(stderr, "Delete mars_graphics\n");
            delete framesFactory;
            if(dataBroker)
            {
                std::string groupName, dataName;
                groupName = "MarsGraphics";
                dataName = "debugTime";
                dataBroker->unregisterTimedProducer(this, groupName, dataName,
                                                    "_REALTIME_");
                libManager->releaseLibrary("data_broker");
            }
        }

        void GraphicsManager::initializeOSG(void *data, bool createWindow)
        {
            if(!initialized)
            {
                cfg = libManager->getLibraryAs<cfg_manager::CFGManagerInterface>("cfg_manager");
                if(!cfg)
                {
                    fprintf(stderr, "******* mars_graphics: couldn't find cfg_manager\n");
                    return;
                }

                resources_path.propertyType = cfg_manager::stringProperty;
                resources_path.propertyIndex = 0;
                resources_path.sValue = ".";

                if(cfg)
                {
                    configPath = cfg->getOrCreateProperty("Config", "config_path",
                                                          std::string{"."});

                    auto loadFile = configPath.sValue;
                    loadFile.append("/mars_Graphics.yaml");
                    cfg->loadConfig(loadFile.c_str());

                    // have to handle multisampling here
                    multisamples.propertyType = cfg_manager::intProperty;
                    multisamples.propertyIndex = 0;
                    multisamples.iValue = 0;
                    if(cfg->getPropertyValue("Graphics", "num multisamples", "value",
                                             &multisamples.iValue))
                    {
                        multisamples.paramId = cfg->getParamId("Graphics", "num multisamples");
                    }
                    else
                    {
                        multisamples.paramId = cfg->createParam(string("Graphics"),
                                                                string("num multisamples"),
                                                                cfg_manager::intParam);
                        cfg->setProperty(multisamples);
                    }
                    cfg->registerToParam(multisamples.paramId,
                                         dynamic_cast<cfg_manager::CFGClient*>(this));
                    setMultisampling(multisamples.iValue);

                    const auto& s = cfg->getOrCreateProperty("Graphics", "resources_path",
                                                             "",
                                                             dynamic_cast<cfg_manager::CFGClient*>(this)).sValue;
                    if(s != "")
                    {
                        resources_path.sValue = s;
                    } else
                    {
                        resources_path.sValue = string(MARS_GRAPHICS_DEFAULT_RESOURCES_PATH);
                    }

                    noiseProp = cfg->getOrCreateProperty("Graphics", "useNoise",
                                                         true, this);
                    useNoise = noiseProp.bValue;

                    drawLineLaserProp = cfg->getOrCreateProperty("Graphics", "drawLineLaser",
                                                                 false, this);
                    drawLineLaser = drawLineLaserProp.bValue;

                    hudWidthProp = cfg->getOrCreateProperty("Graphics", "hudWidth",
                                                            1920, this);
                    hudWidth = hudWidthProp.iValue;

                    hudHeightProp = cfg->getOrCreateProperty("Graphics", "hudHeight",
                                                             1080, this);
                    hudHeight = hudHeightProp.iValue;

                    marsShadow = cfg->getOrCreateProperty("Graphics", "marsShadow",
                                                          false, this);
                    shadowTechnique = cfg->getOrCreateProperty("Graphics", "shadowTechnique",
                                                               "pssm", this);
                    defaultMaxNumNodeLights = cfg->getOrCreateProperty("Graphics",
                                                                       "defaultMaxNumNodeLights",
                                                                       1, this);
                    shadowTextureSize = cfg->getOrCreateProperty("Graphics",
                                                                 "shadowTextureSize",
                                                                 2048, this);
                    shadowSubTextureSize = cfg->getOrCreateProperty("Graphics",
                                                                    "shadowSubTextureSize",
                                                                    1024, this);
                    shadowSamples = cfg->getOrCreateProperty("Graphics",
                                                             "shadowSamples",
                                                             1, this);
                    showGridProp = cfg->getOrCreateProperty("Graphics", "showGrid",
                                                            false, this);
                    showCoordsProp = cfg->getOrCreateProperty("Graphics", "showCoords",
                                                              true, this);
                    showSelectionProp = cfg->getOrCreateProperty("Graphics",
                                                                 "showSelection",
                                                                 true, this);
                    vsyncProp = cfg->getOrCreateProperty("Graphics",
                                                         "vsync",
                                                         false, this);
                    zNear = cfg->getOrCreateProperty("Graphics", "zNear", 0.01, this);
                    zFar = cfg->getOrCreateProperty("Graphics", "zFar", 1000.0, this);
                    noZCompute = cfg->getOrCreateProperty("Graphics", "noZCompute", false, this);
                }
                else
                {
                    marsShadow.bValue = false;
                }
                globalStateset->setGlobalDefaults();

                // with backface culling backfaces are not processed,
                // else front and back faces are always processed.
                // Its a good idea to turn this on for perfomance reasons,
                // 2D objects in 3D scene may want to overwrite this setting, or
                // walk through indices front to back and vice versa
                // to get two front faces.
                cull = new osg::CullFace();
                cull->setMode(osg::CullFace::BACK);

                { // setup LIGHT
                    globalStateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
                    globalStateset->setMode(GL_LIGHTING, osg::StateAttribute::ON);

                    globalStateset->setMode(GL_LIGHT0, osg::StateAttribute::OFF);
                    globalStateset->setMode(GL_LIGHT1, osg::StateAttribute::OFF);
                    globalStateset->setMode(GL_LIGHT2, osg::StateAttribute::OFF);
                    globalStateset->setMode(GL_LIGHT3, osg::StateAttribute::OFF);
                    globalStateset->setMode(GL_LIGHT4, osg::StateAttribute::OFF);
                    globalStateset->setMode(GL_LIGHT5, osg::StateAttribute::OFF);
                    globalStateset->setMode(GL_LIGHT6, osg::StateAttribute::OFF);
                    globalStateset->setMode(GL_LIGHT7, osg::StateAttribute::OFF);
                    globalStateset->setMode(GL_BLEND,osg::StateAttribute::OFF);
                }

                // background color for the scene
                graphicOptions.clearColor = mars::utils::Color{0.55, 0.67, 0.88, 1.0};

                { // setup FOG
                    graphicOptions.fogColor = mars::utils::Color{0.2, 0.2, 0.2, 1.0};
                    graphicOptions.fogEnabled = true;
                    graphicOptions.fogDensity = 0.35;
                    graphicOptions.fogStart = 10.0;
                    graphicOptions.fogEnd = 30.0;

                    osg::ref_ptr<osg::Fog> myFog = new osg::Fog;
                    myFog->setMode(osg::Fog::LINEAR);
                    myFog->setColor(toOSGVec4(graphicOptions.fogColor));
                    myFog->setStart(graphicOptions.fogStart);
                    myFog->setEnd(graphicOptions.fogEnd);
                    myFog->setDensity(graphicOptions.fogDensity);
                    globalStateset->setAttributeAndModes(myFog.get(), osg::StateAttribute::ON);
                }

                // some fixed function pipeline stuff...
                // i guess the default is smooth shading, that means
                // light influence is calculated per vertex and interpolated for fragments.
                auto myLightModel = osg::ref_ptr<osg::LightModel>{new osg::LightModel};
                myLightModel->setTwoSided(false);
                globalStateset->setAttributeAndModes(myLightModel.get(), osg::StateAttribute::ON);

                // associate scene with global states
                scene->setStateSet(globalStateset.get());
                scene->addChild(lightGroup.get());
                scene->addChild(shadowedScene.get());

                // init light (osg can have only 8 lights enabled at a time)
                for(unsigned int i=0; i<8; i++)
                {
                    lightmanager ltemp;
                    ltemp.free=true;
                    myLights.push_back(ltemp);
                }

                initDefaultLight();


                shadowedScene->setReceivesShadowTraversalMask(ReceivesShadowTraversalMask);
                shadowedScene->setCastsShadowTraversalMask(CastsShadowTraversalMask);
                shadowStateset = shadowedScene->getOrCreateStateSet();
                {
// #if USE_LSPSM_SHADOW
//           osg::ref_ptr<osgShadow::LightSpacePerspectiveShadowMapDB> sm =
//             new osgShadow::LightSpacePerspectiveShadowMapDB;

//           //sm->setDebugDraw(true);
//           sm->setMinLightMargin( 10.0f );
//           sm->setMaxFarPlane( 0.0f );
//           sm->setTextureSize( osg::Vec2s( 2028, 2028 ) );
//           sm->setShadowTextureCoordIndex( 6 );
//           sm->setShadowTextureUnit( 6 );

//           shadowedScene->setShadowTechnique( sm.get() );
// #elif USE_PSSM_SHADOW
                    // TODO: it shoud be fine to use setShadowTechnique here but somehow it is not working
                    if(shadowTechnique.sValue == "pssm")
                    {
                        pssm = new ParallelSplitShadowMap(nullptr, NUM_PSSM_SPLITS, shadowSubTextureSize.iValue);
                        //pssm->enableShadowGLSLFiltering(false);
                        pssm->setTextureResolution(shadowTextureSize.iValue);
                        pssm->setMinNearDistanceForSplits(0);
                        pssm->setMaxFarDistance(500);
                        pssm->setMoveVCamBehindRCamFactor(0);
                        pssm->setPolygonOffset(osg::Vec2{1.2, 1.2});
                        //pssm->applyState(shadowStateset.get());
                        if(marsShadow.bValue)
                        {
                            shadowedScene->setShadowTechnique(pssm.get());
                        }
                        pssm->applyState(scene->getOrCreateStateSet());
                    } else
                    {
                        shadowMap = new ShadowMap;
                        shadowMap->setShadowTextureSize(shadowTextureSize.iValue);
                        shadowMap->initTexture();
                        shadowMap->applyState(shadowStateset.get());
                        if(marsShadow.bValue)
                        {
                            shadowedScene->setShadowTechnique(shadowMap.get());
                            //shadowMap->setTextureSize(osg::Vec2s(4096,4096));
                            //shadowMap->setTextureUnit(2);
                            //shadowMap->clearShaderList();
                            //shadowMap->setAmbientBias(osg::Vec2(0.5f,0.5f));
                            //shadowMap->setPolygonOffset(osg::Vec2(-1.2,-1.2));
                        }
                    }
// #endif
                }

                // TODO: check this out:
                //   i guess fire.rgb is a 1D texture
                //   there is something to generate these in OGLE
                //osg::ref_ptr<osgParticle::ParticleEffect> effectNode =
                //new osgParticle::FireEffect;
                //effectNode->setTextureFileName("fire.rgb");
                //effectNode->setIntensity(2.5);
                //effectNode->setScale(4);
                //scene->addChild(effectNode.get());
                grid = new GridPrimitive{osgWidget};
                if(showGridProp.bValue) showGrid();

                show_coords = showCoordsProp.bValue;
                if(show_coords)
                {
                    showCoords();
                }

                // reset number of frames
                framecount = 0;

                viewer = new osgViewer::CompositeViewer();
                viewer->setKeyEventSetsDone(0);
#ifdef SINGLE_THREADED
                viewer->setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);
#else
                viewer->setThreadingModel(osgViewer::CompositeViewer::DrawThreadPerContext);
#endif

                if(createWindow)
                {
                    new3DWindow(data);
                }

                //guiHelper->setGraphicsWidget(graphicsWindows[0]);
                setupCFG();

                // init materialManager
                materialManager = libManager->getLibraryAs<OsgMaterialManager>("osg_material_manager", true);
                if(materialManager)
                {
                    materialManager->setUseShader(marsShader.bValue);
                    materialManager->setShadowTextureSize(shadowTextureSize.iValue);
                    materialManager->setUseFog(useFog);
                    materialManager->setUseNoise(useNoise);
                    materialManager->setDrawLineLaser(drawLineLaser);
                    materialManager->setUseShadow(marsShadow.bValue);
                    materialManager->setShadowSamples(shadowSamples.iValue);
                    materialManager->setDefaultMaxNumLights(defaultMaxNumNodeLights.iValue);
                    materialManager->setBrightness((float)brightness.dValue);
                    materialManager->setNoiseAmmount((float)noiseAmmount.dValue);
                    materialManager->setShadowTechnique(shadowTechnique.sValue);
                    if(pssm.valid())
                    {
                        pssm->applyState(materialManager->getMainStateGroup()->getOrCreateStateSet());
                    }
                    else if(shadowMap.valid())
                    {
                        shadowMap->applyState(materialManager->getMainStateGroup()->getOrCreateStateSet());
                    }
                    shadowedScene->addChild(materialManager->getMainStateGroup());
                    auto map = ConfigMap::fromYamlFile(resources_path.sValue+"/defaultMaterials.yml");
                    MaterialData md;
                    ConfigMap defM;
                    if(map.hasKey("Materials"))
                    {
                        for(size_t i=0; i<map["Materials"].size(); ++i)
                        {
                            md.toConfigMap(&defM);
                            defM.append(map["Materials"][i]);
                            if(defM.hasKey("diffuseTexture"))
                            {
                                defM["diffuseTexture"] = (resources_path.sValue + "/" + defM["diffuseTexture"].toString());
                            }
                            materialManager->createMaterial(defM["name"], defM);
                        }
                    }
                }
                // initialize shadow technique
                //setShadowTechnique(shadowTechnique.sValue);

                if(backfaceCulling.bValue)
                {
                    globalStateset->setAttributeAndModes(cull, osg::StateAttribute::ON);
                }
                else
                {
                    globalStateset->setAttributeAndModes(cull, osg::StateAttribute::OFF);
                }
                initialized = true;
            }
        }

        /**\brief resets scene */
        void GraphicsManager::reset()
        {
            //remove graphics stuff & rearrange light numbers
            for(size_t i=0; i<myLights.size(); i++)
            {
                //removeLight(i);
            }
            for(const auto& drawObject: drawObjects_)
            {
                // TODO: solve differently! This modifies drawObjects_!
                removeDrawObject(drawObject.first);
            }
            DrawCoreIds.clear();
            clearDrawItems();

        }

        void GraphicsManager::addGraphicsUpdateInterface(GraphicsUpdateInterface *g)
        {
            graphicsUpdateObjects.push_back(g);
        }

        void GraphicsManager::removeGraphicsUpdateInterface(GraphicsUpdateInterface *g)
        {
            auto it = find(std::begin(graphicsUpdateObjects), std::end(graphicsUpdateObjects), g);
            if(it!=std::end(graphicsUpdateObjects))
            {
                graphicsUpdateObjects.erase(it);
            }
        }

        /**
         * sets the camera type
         * @param: type
         */
        void GraphicsManager::setCamera(int type)
        {
            osgWidget->getCameraInterface()->setCamera(type);
        }

        /**
         * returns actual camera information
         */
        void GraphicsManager::getCameraInfo(mars::interfaces::cameraStruct *cs) const
        {
            if(activeWindow)
            {
                activeWindow->getCameraInterface()->getCameraInfo(cs);
            }
        }

        void* GraphicsManager::getScene() const
        {
            return static_cast<void*>(scene.get());
        }

        void* GraphicsManager::getScene2() const
        {
            return static_cast<void*>(dynamic_cast<osg::Node*>(shadowedScene.get()));
        }

        void GraphicsManager::saveScene(const string &filename) const
        {
            osgDB::writeNodeFile(*(shadowedScene.get()), filename);
        }

        void GraphicsManager::exportScene(const string &filename) const
        {
            osgDB::writeNodeFile(*(shadowedScene.get()), filename.data());
        }

        void* GraphicsManager::getStateSet() const
        {
            return static_cast<void*>(globalStateset.get());
        }

        void GraphicsManager::update()
        {
            //update drawElements
            for(size_t i=0; i<draws.size(); i++)
            {
                auto& draw = draws[i];
                vector<draw_item> tmp_ditem;
                vector<osg::Node*> tmp_nodes;
                //update draws
                draw.ds.ptr_draw->update(&(draw.ds.drawItems));

                for(size_t j=0; j<draw.ds.drawItems.size(); j++)
                {
                    auto& di = draw.ds.drawItems[j];

                    if(di.draw_state == DRAW_STATE_ERASE)
                    {
                        scene->removeChild(draw.nodes[j]);
                    }
                    else if(di.draw_state == DRAW_STATE_CREATE)
                    {
                        auto font_path = resources_path.sValue;
                        font_path.append("/Fonts");
                        auto osgNode = osg::ref_ptr<osg::Group>{new OSGDrawItem(osgWidget, di, font_path)};
                        scene->addChild(osgNode.get());

                        di.draw_state = DRAW_UNKNOWN;
                        tmp_ditem.push_back(di);
                        tmp_nodes.push_back(osgNode.get());
                    }
                    else if(di.draw_state == DRAW_STATE_UPDATE)
                    {
                        assert(draws[i].nodes.size() > j);
                        auto* n = draws[i].nodes[j];
                        auto* diWrapper = dynamic_cast<OSGDrawItem*>(n->asGroup()); // TODO: asGroup unneeded?
                        assert(diWrapper != nullptr); // TODO: handle this case better

                        diWrapper->update(di);

                        di.draw_state = DRAW_UNKNOWN;
                        tmp_ditem.push_back(di);
                        tmp_nodes.push_back(n);
                    }
                    else
                    { // invalid draw state!
                        di.draw_state = DRAW_UNKNOWN;
                        tmp_ditem.push_back(di);
                        tmp_nodes.push_back(draws[i].nodes[j]);
                    }
                }

                draws[i].ds.drawItems.clear();
                draws[i].ds.drawItems = tmp_ditem;
                draws[i].nodes.clear();
                draws[i].nodes = tmp_nodes;
            }
        }

        const mars::interfaces::GraphicData GraphicsManager::getGraphicOptions(void) const
        {
            return graphicOptions;
        }

        void GraphicsManager::setGraphicOptions(const mars::interfaces::GraphicData &options,
                                                bool ignoreClearColor)
        {
            auto* myFog = dynamic_cast<osg::Fog*>(globalStateset->getAttribute(osg::StateAttribute::FOG));

            graphicOptions = options;
            if(!ignoreClearColor)
            {
                for(size_t i=0; i<graphicsWindows.size(); i++)
                {
                    graphicsWindows[i]->setClearColor(graphicOptions.clearColor);
                }
            }

            myFog->setColor(osg::Vec4(graphicOptions.fogColor.r,
                                      graphicOptions.fogColor.g,
                                      graphicOptions.fogColor.b, 1.0));
            myFog->setStart(graphicOptions.fogStart);
            myFog->setEnd(graphicOptions.fogEnd);
            myFog->setDensity(graphicOptions.fogDensity);

            if(graphicOptions.fogEnabled)
            {
                globalStateset->setMode(GL_FOG, osg::StateAttribute::ON);
                useFog = true;
            }
            else
            {
                globalStateset->setMode(GL_FOG, osg::StateAttribute::OFF);
                useFog = false;
            }

            if(materialManager)
            {
                materialManager->setUseFog(useFog);
            }
        }

        void GraphicsManager::setWidget(GraphicsWidget *widget)
        {
            //guiHelper->setGraphicsWidget(widget);
        }

        // this function should be deprecated
        void GraphicsManager::setTexture(unsigned long id, const string &filename)
        {
            for(auto& node: myNodes)
            {
                if(node.index == id)
                {
                    auto* const state = node.matrix->getChild(0)->getOrCreateStateSet();
                    state->setTextureAttributeAndModes(0, GuiHelper::loadTexture(filename).get(),
                                                       osg::StateAttribute::ON |
                                                       osg::StateAttribute::PROTECTED);
                    break;
                }
            }
        }

        unsigned long GraphicsManager::new3DWindow(void *myQTWidget, bool rtt,
                                                   int width, int height, const std::string &name)
        {
            GraphicsWidget* gw; // TODO: Lambda for initialization
            if(graphicsWindows.size() > 0)
            {
                gw = QtOsgMixGraphicsWidget::createInstance(myQTWidget, scene.get(),
                                                            next_window_id++, rtt,
                                                            this);
                gw->initializeOSG(myQTWidget, graphicsWindows[0], width, height);
            }
            else
            {

                gw = QtOsgMixGraphicsWidget::createInstance(myQTWidget, scene.get(),
                                                            next_window_id++, 0,
                                                            this);

                // this will open an osg widget without qt wrapping
                /*
                  gw = new GraphicsWidget(myQTWidget, scene.get(),
                  next_window_id++, 0,
                  0, this);
                */
                gw->initializeOSG(myQTWidget, 0, width, height, vsyncProp.bValue);
            }

            activeWindow = gw;
            gw->setName(name);
            gw->setClearColor(graphicOptions.clearColor);
            auto* const gc = dynamic_cast<GraphicsCamera*>(gw->getCameraInterface());
            gc->setZNear(zNear.dValue);
            gc->setZFar(zFar.dValue);
            gc->setNoZCompute(noZCompute.bValue);
            viewer->addView(gw->getView());
            graphicsWindows.push_back(gw);

            if(!rtt)
            {
                setActiveWindow(next_window_id-1);
                gw->setGraphicsEventHandler(static_cast<GraphicsEventInterface*>(this));

                auto* const myHUD = new HUD{next_window_id};
                myHUD->init(gw->getGraphicsWindow());
                myHUD->setViewSize(hudWidth, hudHeight);

                gw->setHUD(myHUD);

                // iterator over hudElements

                for(const auto& hudElement: hudElements)
                {
                    gw->addHUDElement(hudElement->getHUDElement());
                }
            }
            return next_window_id - 1;
        }

        void* GraphicsManager::getView(unsigned long id)
        {
            auto* const gw = getGraphicsWindow(id);

            if(gw == nullptr)
            {
                return gw;
            }
            return static_cast<void*>(gw->getView());
        }

        void GraphicsManager::deactivate3DWindow(unsigned long id)
        {
            auto* const gw = getGraphicsWindow(id);

            if(gw == nullptr)
            {
                return;
            }
            //viewer->removeView(gw->getView());
            gw->getCameraInterface()->deactivateCam();
        }

        void GraphicsManager::activate3DWindow(unsigned long id)
        {
            auto* const gw = getGraphicsWindow(id);

            if(gw == nullptr)
            {
                return;
            }
            gw->getCameraInterface()->activateCam();
        }

        GraphicsWindowInterface* GraphicsManager::get3DWindow(unsigned long id) const
        {
            auto gwItr = std::find_if(  std::begin(graphicsWindows), std::end(graphicsWindows), 
                                        [id](GraphicsWidget* w){ return w->getID() == id;});
            if (gwItr == std::end(graphicsWindows))
            {
                return nullptr;
            }

            return static_cast<GraphicsWindowInterface*>(*gwItr);
        }

        GraphicsWindowInterface* GraphicsManager::get3DWindow(const std::string &name) const
        {
            auto gwItr = std::find_if(  std::begin(graphicsWindows), std::end(graphicsWindows), 
                                        [&name](GraphicsWidget* w){ return w->getName() == name;});
            if (gwItr == std::end(graphicsWindows))
            {
                return nullptr;
            }

            return static_cast<GraphicsWindowInterface*>(*gwItr);
        }


        void GraphicsManager::remove3DWindow(unsigned long id)
        {
            auto gwItr = std::find_if(  std::begin(graphicsWindows), std::end(graphicsWindows), 
                                        [id](GraphicsWidget* w){ return w->getID() == id;});
            if (gwItr == std::end(graphicsWindows))
            {
                return;
            }

            delete *gwItr;
            // TODO: Also erase pointer?!
            // graphicsWindows.erase(gwItr);
        }

        void GraphicsManager::removeGraphicsWidget(unsigned long id)
        {
            auto gwItr = std::find_if(  std::begin(graphicsWindows), std::end(graphicsWindows), 
                                        [id](GraphicsWidget* w){ return w->getID() == id;});
            if (gwItr == std::end(graphicsWindows))
            {
                return;
            }

            viewer->removeView((*gwItr)->getView());
            // TODO: Also delete graphicsWindow?
            // delete *gwItr;
            graphicsWindows.erase(gwItr);
        }


        GraphicsWidget* GraphicsManager::getGraphicsWindow(unsigned long id) const
        {
            auto gwItr = std::find_if(  std::begin(graphicsWindows), std::end(graphicsWindows), 
                                        [id](GraphicsWidget* w){ return w->getID() == id;});
            if (gwItr == std::end(graphicsWindows))
            {
                return nullptr;
            }

            return *gwItr;
        }

        void GraphicsManager::getList3DWindowIDs(std::vector<unsigned long> *ids) const
        {
            for(const auto& graphicsWindow: graphicsWindows)
            {
                ids->push_back(graphicsWindow->getID());
            }
        }

        void GraphicsManager::draw()
        {
            long myTime = utils::getTime();
            long timeDiff;

            for(auto& graphicsUpdateObject: graphicsUpdateObjects)
            {
                graphicsUpdateObject->preGraphicsUpdate();
            }
            timeDiff = getTimeDiff(myTime);
            myTime += timeDiff;
            preTime += timeDiff;

            update();
            for(auto& graphicsWindow: graphicsWindows)
            {
                graphicsWindow->updateView();
            }
            timeDiff = getTimeDiff(myTime);
            myTime += timeDiff;
            updateTime += timeDiff;

            vector<mars::interfaces::LightData*> lightList;
            getLights(&lightList);
            if(lightList.size() == 0)
            {
                lightList.push_back(&defaultLight.lStruct);
            }

            for(size_t i=0; i < myLights.size(); i++)
            {
                //return only the used lights
                if(!myLights[i].free)
                {
                    const auto& drawID = myLights[i].lStruct.drawID;
                    if(drawID != 0)
                    {
                        auto drawObjectItr = std::find_if(  std::begin(drawObjects_), std::end(drawObjects_), 
                                                            [&drawID](const std::pair<unsigned long, osg::ref_ptr<OSGNodeStruct>>& x)
                                                            { return x.first == drawID; });
                        // Set position
                        auto pos = drawObjectItr->second->object()->getPosition();
                        const auto& q = drawObjectItr->second->object()->getQuaternion();
                        myLights[i].lStruct.pos = pos;
                        myLights[i].light->setPosition(osg::Vec4(pos.x(), pos.y(), pos.z()+0.1, 1.0));
                        // Set direction
                        pos = q*Vector{1, 0, 0};
                        myLights[i].lStruct.lookAt = pos;
                        myLights[i].light->setDirection(osg::Vec3(pos.x(), pos.y(), pos.z()));
                    }
                    else if(myLights[i].lStruct.node != "")
                    {
                        const auto& nodeName = myLights[i].lStruct.node;
                        auto drawObjectItr = std::find_if(  std::begin(drawObjects_), std::end(drawObjects_), 
                                                            [&nodeName](const std::pair<unsigned long, osg::ref_ptr<OSGNodeStruct>>& x)
                                                            { return x.second->name() == nodeName; });
                        myLights[i].lStruct.drawID = drawObjectItr->first;
                    }
                }
            }

            timeDiff = getTimeDiff(myTime);
            myTime += timeDiff;
            lightTime += timeDiff;

            if(materialManager)
            {
                materialManager->updateLights(lightList);

                if(useNoise)
                {
                    materialManager->updateShadowSamples();
                }
                if(shadowMap.valid())
                {
                    materialManager->setShadowScale(shadowMap->getTexScale());
                }
            }
            timeDiff = getTimeDiff(myTime);
            myTime += timeDiff;
            materialTime += timeDiff;

            mutex.lock();
            // Render a complete new frame.
            if(viewer)
            {
                viewer->frame();
            }
            mutex.unlock();
            timeDiff = getTimeDiff(myTime);
            myTime += timeDiff;
            frameTime += timeDiff;
            ++framecount;
            for(auto& graphicsUpdateObject: graphicsUpdateObjects)
            {
                graphicsUpdateObject->postGraphicsUpdate();
            }

            timeDiff = getTimeDiff(myTime);
            myTime += timeDiff;
            postTime += timeDiff;
            if(++avgTimeCount == 100)
            {
                avgPreTime = preTime / avgTimeCount;
                avgUpdateTime = updateTime / avgTimeCount;
                avgLightTime = lightTime / avgTimeCount;
                avgMaterialTime = materialTime / avgTimeCount;
                avgFrameTime = frameTime / avgTimeCount;
                avgPostTime = postTime / avgTimeCount;
                preTime = 0;
                updateTime = 0;
                lightTime = 0;
                materialTime = 0;
                frameTime = 0;
                postTime = 0;
                avgTimeCount = 0;
            }
        }

        void GraphicsManager::setGrabFrames(bool value)
        {
            graphicsWindows[0]->setGrabFrames(value);
            graphicsWindows[0]->setSaveFrames(value);
        }

        void GraphicsManager::setActiveWindow(unsigned long win_id)
        {
            for(auto w: graphicsWindows)
            {
                if(w->getID() == win_id)
                {
                    w->grabFocus();
                    viewer->setCameraWithFocus(w->getMainCamera());
                }
                else
                {
                    w->unsetFocus();
                }
            }
        }

        void* GraphicsManager::getQTWidget(unsigned long id) const
        {
            auto graphicsWindowItr = std::find_if(  std::begin(graphicsWindows), std::end(graphicsWindows), 
                                                    [id](GraphicsWidget* const x) { return x->getID() == id; });

            if (graphicsWindowItr == std::end(graphicsWindows))
            {
                return nullptr;
            }

            return (*graphicsWindowItr)->getWidget();
        }

        void GraphicsManager::showQTWidget(unsigned long id)
        {
            auto graphicsWindowItr = std::find_if(  std::begin(graphicsWindows), std::end(graphicsWindows), 
                                                    [id](GraphicsWidget* const x) { return x->getID() == id; });

            if (graphicsWindowItr == std::end(graphicsWindows))
            {
                return;
            }

            (*graphicsWindowItr)->showWidget();
        }

        void GraphicsManager::setGraphicsWindowGeometry(unsigned long id,
                                                        int top, int left,
                                                        int width, int height)
        {
            auto graphicsWindowItr = std::find_if(  std::begin(graphicsWindows), std::end(graphicsWindows), 
                                                    [id](GraphicsWidget* const x) { return x->getID() == id; });

            if (graphicsWindowItr == std::end(graphicsWindows))
            {
                return;
            }

            (*graphicsWindowItr)->setWGeometry(top, left, width, height);
        }

        void GraphicsManager::getGraphicsWindowGeometry(unsigned long id,
                                                        int *top, int *left,
                                                        int *width, int *height) const
        {
            auto graphicsWindowItr = std::find_if(  std::begin(graphicsWindows), std::end(graphicsWindows), 
                                                    [id](GraphicsWidget* const x) { return x->getID() == id; });

            if (graphicsWindowItr == std::end(graphicsWindows))
            {
                return;
            }

            (*graphicsWindowItr)->getWGeometry(top, left, width, height);
        }

        ////// DRAWOBJECTS

        unsigned long GraphicsManager::findCoreObject(unsigned long draw_id) const
        {
            const auto it = DrawCoreIds.find(draw_id);
            if (it == DrawCoreIds.end())
            {
                return 0;
            }
            else
            {
                return it->second;
            }
        }


        OSGNodeStruct* GraphicsManager::findDrawObject(unsigned long id) const
        {
            auto needle = drawObjects_.find(id);
            if(needle == drawObjects_.end())
            {
                return nullptr;
            }

            return needle->second.get();
        }

        unsigned long GraphicsManager::addDrawObject(const mars::interfaces::NodeData &snode,
                                                     bool activated)
        {
            const auto drawObjectItr = std::find_if(std::begin(drawObjects_), std::end(drawObjects_),
                                                    [&snode](const std::pair<unsigned long, osg::ref_ptr<OSGNodeStruct>>& x)
                                                    { return x.second->name() == snode.name; });
            if (drawObjectItr != std::end(drawObjects_))
            {
                return drawObjectItr->first;
            }
            unsigned long id = next_draw_object_id++;
            int mask = 0;

            vector<mars::interfaces::LightData*> lightList;
            getLights(&lightList);
            if(lightList.size() == 0)
            {
                lightList.push_back(&defaultLight.lStruct);
            }
            auto drawObject = osg::ref_ptr<OSGNodeStruct>{new OSGNodeStruct{this, snode, false, id}};
            auto* const transform = drawObject->object()->getPosTransform();

            DrawCoreIds.insert(std::make_pair(id, snode.index));
            drawObjects_[id] = drawObject;
            auto* const configPtr = const_cast<ConfigMap*>(&(snode.map));
            if(configPtr->hasKey("createFrame") && static_cast<bool>((*configPtr)["createFrame"]) == true)
            {
                drawObject->object()->frame = framesFactory->createFrame();
                drawObject->object()->frame->setScale(scaleFramesProp.dValue);
                if(showFramesProp.bValue)
                {
                    scene->addChild(static_cast<osg::PositionAttitudeTransform*>(drawObject->object()->frame->getOSGNode()));
                }
            }
            if(snode.isShadowCaster)
            {
                mask |= CastsShadowTraversalMask;
            }
            if(snode.isShadowReceiver)
            {
                mask |= ReceivesShadowTraversalMask;
            }
            transform->setNodeMask(transform->getNodeMask() | mask);

            // import an .STL file : we have to insert an additional transformation
            // in order to add the additional rotation by 90 degrees around the
            // x-axis (adding the rotation to "transform" does not help at all,
            // because the values of "transform" are constantly resetted by MARS
            // itself)
            if((snode.filename.substr(snode.filename.size()-4, 4) == ".STL") ||
               (snode.filename.substr(snode.filename.size()-4, 4) == ".stl"))
            {
                // create the new transformation to be added
                auto transformSTL = osg::ref_ptr<osg::PositionAttitudeTransform>{
                    new osg::PositionAttitudeTransform()};

                // remove all child nodes from "transform" and add them to
                // "transformSTL"
                osg::Node* node = nullptr;
                while (transform->getNumChildren() > 0)
                {
                    node = transform->getChild(0);
                    transformSTL->addChild(node);
                    transform->removeChild(node);
                }

                // add "transformSTL" as child to "transform"
                transform->addChild(transformSTL);

                // calulate the quaternion for the rotation of 90 degrees around the
                // x-axis
                // mars::utils::Quaternion offset =
                //     mars::utils::eulerToQuaternion(mars::utils::Vector(90.0, 0.0, 0.0));

                // // set the orientation to the newly added transformation
                // transformSTL->setAttitude(osg::Quat(offset.x(),
                //                                     offset.y(),
                //                                     offset.z(),
                //                                     offset.w()));
            }

            setDrawObjectMaterial(id, snode.material);
            if(activated)
            {
                drawObject->object()->show();

                if(shadowMap.valid() && snode.map.find("shadowCenterRadius") != snode.map.end())
                {
                    shadowMap->setCenterObject(drawObject->object());
                    auto* const configPtr = const_cast<ConfigMap*>(&(snode.map));
                    shadowMap->setRadius((*configPtr)["shadowCenterRadius"][0]);
                }
            }
            //osgUtil::Optimizer optimizer;
            //optimizer.optimize(shadowedScene.get());

            // TODO: handle preview mode
            // TODO: this will not work if material.exists = false
            return id;
        }

        unsigned long GraphicsManager::getDrawID(const std::string &name) const
        {
            auto drawObjectItr = std::find_if(  std::begin(drawObjects_), std::end(drawObjects_), 
                                                [&name](const std::pair<unsigned long, osg::ref_ptr<OSGNodeStruct>>& x) { return x.second->name() == name; });
            if (drawObjectItr == std::end(drawObjects_))
            {
                return 0;
            }

            return drawObjectItr->first;
        }

        void GraphicsManager::removeDrawObject(unsigned long id)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr) 
            {
                return;
            }

            auto* const drawObject = ns->object();
            if (drawObject)
            {
                drawObject->hide();
                scene->removeChild(drawObject->getPosTransform());
                shadowedScene->removeChild(drawObject->getPosTransform());
            }
            drawObjects_.erase(id);
        }

        void GraphicsManager::exportDrawObject(unsigned long id,
                                               const std::string &name) const
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                return;
            }
            ns->object()->exportModel(name);
        }

        void GraphicsManager::removeLayerFromDrawObjects(unsigned long window_id)
        {
            unsigned int bit = 1 << (window_id-1);

            for(auto drawObject: drawObjects_)
            {
                drawObject.second->object()->removeBits(bit);
            }
        }

        void GraphicsManager::setDrawObjectSelected(unsigned long id, bool val)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                return;
            }

            ns->object()->setSelected(val);

            if(!val)
            {
                auto selectedObjectItr = std::find_if(std::begin(selectedObjects_), std::end(selectedObjects_), [ns](const osg::ref_ptr<OSGNodeStruct>& x) { return x.get() == ns; });
                if (selectedObjectItr != std::end(selectedObjects_))
                {
                    selectedObjects_.erase(selectedObjectItr);
                }
            }

            for(auto graphicsEventClient: graphicsEventClientList)
            {
                graphicsEventClient->selectEvent(findCoreObject(id), val);
            }
        }

        void GraphicsManager::setDrawObjectPos(unsigned long id, const Vector &pos)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr) 
            {
                return;
            }

            ns->object()->setPosition(pos);
        }

        void GraphicsManager::setDrawObjectRot(unsigned long id, const Quaternion &q)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                return;
            }

            ns->object()->setQuaternion(q);
        }

        void GraphicsManager::setDrawObjectScale(unsigned long id, const Vector &scale)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                return;
            }

            ns->object()->setScale(scale);
        }

        void GraphicsManager::setDrawObjectScaledSize(unsigned long id, const Vector &ext)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                return;
            }

            ns->object()->setScaledSize(ext);
        }

        void GraphicsManager::setDrawObjectMaterial(unsigned long id,
                                                    const mars::interfaces::MaterialData &material)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                return;
            }

            if(materialManager)
            {
                auto m = material;
                configmaps::ConfigMap map;
                m.toConfigMap(&map);
                // the material is not overridden if it already exists
                materialManager->createMaterial(material.name, map);
                ns->object()->setMaterial(material.name);
            }
        }

        std::vector<interfaces::MaterialData> GraphicsManager::getMaterialList() const
        {
            if(!materialManager)
            {
                return std::vector<interfaces::MaterialData>{};
            }

            std::vector<interfaces::MaterialData> materialList;
            for(auto& materialConfig: materialManager->getMaterialList())
            {
                interfaces::MaterialData md;
                md.fromConfigMap(&materialConfig, "");
                materialList.push_back(md);
            }
            return materialList;
        }

        void GraphicsManager::editMaterial(std::string materialName,
                                           std::string key,
                                           std::string value)
        {
            if(!materialManager)
            {
                return;
            }

            materialManager->editMaterial(materialName, key, value);
        }

        void GraphicsManager::setDrawObjectNodeMask(unsigned long id, unsigned int bits)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                return;
            }

            ns->object()->setBits(bits);
        }

        void GraphicsManager::setDrawObjectBrightness(unsigned long id, double v)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                return;
            }

            ns->object()->setBrightness(v);
        }

        void GraphicsManager::setBlending(unsigned long id, bool mode)
        {
            fprintf(stderr, "setBlending is deprecated\n\n");
            assert(false);
        }

        void GraphicsManager::setBumpMap(unsigned long id, const std::string &bumpMap)
        {
            fprintf(stderr, "setBumpMap is deprecated\n\n");
            assert(false);
        }

        void GraphicsManager::setSelectable(unsigned long id, bool val)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                return;
            }

            ns->object()->setSelectable(val);
        }

        void GraphicsManager::setDrawObjectRBN(unsigned long id, int val)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                return;
            }

            ns->object()->setRenderBinNumber(val);
        }

        void GraphicsManager::setDrawObjectShow(unsigned long id, bool val)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                return;
            }

            if(val)
            {
                ns->object()->show();
                //shadowedScene->addChild(ns->object()->getPosTransform());
            } else
            {
                ns->object()->hide();
                scene->removeChild(ns->object()->getPosTransform());
                shadowedScene->removeChild(ns->object()->getPosTransform());
            }
        }

        ///// DRAWITEMS

        /**
         * adds drawStruct items to the scene
         * @param drawStruct draw
         */
        void GraphicsManager::addDrawItems(drawStruct *draw)
        {
            //create a mapper
            drawMapper myMapper;
            myMapper.ds = *draw;
            draws.push_back(myMapper);
        }

        void GraphicsManager::removeDrawItems(DrawInterface *iface)
        {
            auto drawMapperItr = std::find_if(  std::begin(draws), std::end(draws), 
                                                [iface](const drawMapper& x) { return x.ds.ptr_draw == iface; });
            if (drawMapperItr == std::end(draws))
            {
                return;
            }

            for(auto& osgNode: drawMapperItr->nodes)
            {
                scene->removeChild(osgNode);
            }
            drawMapperItr->nodes.clear();
            drawMapperItr->ds.drawItems.clear();
            draws.erase(drawMapperItr);
        }

        void GraphicsManager::clearDrawItems(void)
        {
            //clear the list of draw items
            for(const auto& drawMapper: draws)
            {
                removeDrawItems(drawMapper.ds.ptr_draw);
            }
            draws.clear();
        }

        ///// LIGHT

        void GraphicsManager::addLight(mars::interfaces::LightData &ls)
        {
            bool freeOne = false;
            unsigned int lightIndex = 0;

            // find a light unit, OpenGL has 8 available in fixed function pipeline
            for (size_t i =0; i<myLights.size(); i++)
            {
                if (myLights[i].free)
                {
                    lightIndex = i;
                    freeOne = true;
                    break;
                }
            }
            // add light only if we found a free slot
            if (freeOne)
            {
                lightmanager lm;

                // set the free index
                ls.index = lightIndex;

                auto myLightSource = osg::ref_ptr<osg::LightSource>{new OSGLightStruct{ls}};

                //add to lightmanager for later editing possibility
                lm.light = myLightSource->getLight();
                lm.lightSource = myLightSource;
                lm.lStruct = ls;
                lm.lStruct.index = lightIndex;
                lm.free = false;
                if(ls.map.find("produceShadow") != ls.map.end())
                {
                    if(static_cast<bool>(ls.map["produceShadow"]))
                    {
                        if(shadowMap.valid())
                        {
                            shadowMap->setLight(lm.light.get());
                        }
                        if(pssm.valid())
                        {
                            pssm->setLight(lm.light.get());
                        }
                    }
                }
                if(ls.map.hasKey("nodeName"))
                {
                    // TODO: if we don't find the node name the camera should ask for
                    // it on update
                    lm.lStruct.node << ls.map["nodeName"];
                    const auto& nodeName = lm.lStruct.node;
                    auto drawObjectItr = std::find_if(  std::begin(drawObjects_), std::end(drawObjects_), 
                                                        [&nodeName](const std::pair<unsigned long, osg::ref_ptr<OSGNodeStruct>>& x) 
                                                        { return x.second->name() == nodeName; });
                    if (drawObjectItr != std::end(drawObjects_))
                    {
                        lm.lStruct.drawID = drawObjectItr->first;
                    }
                }
                lightGroup->addChild( myLightSource.get() );
                globalStateset->setMode(GL_LIGHT0+lightIndex, osg::StateAttribute::ON);
                myLightSource->setStateSetModes(*globalStateset, osg::StateAttribute::ON);

                myLights[lm.lStruct.index] = lm;
            }

            // TODO else make a message (should be handled in another way, will be done later)
            else
            {
                cerr << "Light couldn't be added: No free lights available" << endl;
            }
            lightGroup->removeChild(defaultLight.lightSource.get());
        }

        void GraphicsManager::removeLight(unsigned int index)
        {
            if (index < myLights.size() && !myLights[index].free)
            {
                globalStateset->setMode(GL_LIGHT0+index, osg::StateAttribute::OFF);
                lightGroup->removeChild(myLights[index].lightSource.get());

                lightmanager temp;
                temp.free = true;
                myLights[index] = temp;

                vector<mars::interfaces::LightData*> lightList;
                getLights(&lightList);
                if(lightList.size() == 0)
                {
                    lightGroup->addChild(defaultLight.lightSource.get());
                    lightList.push_back(&defaultLight.lStruct);
                }
                // TODO: do we have to remove the light from the shadow implementation?
                //for(iter=drawObjects_.begin(); iter!=drawObjects_.end(); ++iter)
                //iter->second->object()->updateShader(lightList, true);
            }
        }

        void GraphicsManager::setColor(utils::Color *c, const std::string &key,
                                       const std::string &value)
        {
            double v = atof(value.c_str());
            if(key[key.size()-1] == 'a')
            {
                c->a = v;
            }
            else if(key[key.size()-1] == 'r')
            {
                c->r = v;
            }
            else if(key[key.size()-1] == 'g')
            {
                c->g = v;
            }
            else if(key[key.size()-1] == 'b')
            {
                c->b = v;
            }
        }

        void GraphicsManager::editLight(unsigned long id, const std::string &key,
                                        const std::string &value)
        {
            for(size_t i=0; i<myLights.size(); ++i)
            {
                if(myLights[i].lStruct.index == id)
                {
                    if(utils::matchPattern("*/ambient/*", key))
                    {
                        setColor(&(myLights[i].lStruct.ambient), key, value);
                    }
                    else if(utils::matchPattern("*/diffuse/*", key))
                    {
                        setColor(&(myLights[i].lStruct.diffuse), key, value);
                    }
                    else if(utils::matchPattern("*/specular/*", key))
                    {
                        setColor(&(myLights[i].lStruct.specular), key, value);
                    }
                    else if(utils::matchPattern("*/position/x", key))
                    {
                        myLights[i].lStruct.pos.x() = atof(value.c_str());
                    }
                    else if(utils::matchPattern("*/position/y", key))
                    {
                        myLights[i].lStruct.pos.y() = atof(value.c_str());
                    }
                    else if(utils::matchPattern("*/position/z", key))
                    {
                        myLights[i].lStruct.pos.z() = atof(value.c_str());
                    }
                    else if(utils::matchPattern("*/lookat/x", key))
                    {
                        myLights[i].lStruct.lookAt.x() = atof(value.c_str());
                    }
                    else if(utils::matchPattern("*/lookat/y", key))
                    {
                        myLights[i].lStruct.lookAt.y() = atof(value.c_str());
                    }
                    else if(utils::matchPattern("*/lookat/z", key))
                    {
                        myLights[i].lStruct.lookAt.z() = atof(value.c_str());
                    }
                    else if(utils::matchPattern("*/constantAttenuation", key))
                    {
                        myLights[i].lStruct.constantAttenuation = atof(value.c_str());
                    }
                    else if(utils::matchPattern("*/linearAttenuation", key))
                    {
                        myLights[i].lStruct.linearAttenuation = atof(value.c_str());
                    }
                    else if(utils::matchPattern("*/quadraticAttenuation", key))
                    {
                        myLights[i].lStruct.quadraticAttenuation = atof(value.c_str());
                    }
                    else if(utils::matchPattern("*/type", key))
                    {
                        myLights[i].lStruct.type = atoi(value.c_str());
                    }
                    else if(utils::matchPattern("*/angle", key))
                    {
                        myLights[i].lStruct.angle = atof(value.c_str());
                    }
                    else if(utils::matchPattern("*/exponent", key))
                    {
                        myLights[i].lStruct.exponent = atof(value.c_str());
                    }
                    else if(utils::matchPattern("*/directional", key))
                    {
                        ConfigItem b;
                        b = value;
                        myLights[i].lStruct.directional << b;
                        fprintf(stderr, "directional: %d", myLights[i].lStruct.directional);
                    }
                    else if(utils::matchPattern("*/nodeName", key))
                    {
                        myLights[i].lStruct.node = value;
                        auto drawObjectItr = std::find_if(  std::begin(drawObjects_), std::end(drawObjects_), 
                                                            [&value](const std::pair<unsigned long, osg::ref_ptr<OSGNodeStruct>>& x) 
                                                            { return x.second->name() == value; });
                        if (drawObjectItr != std::end(drawObjects_))
                        {
                            myLights[i].lStruct.drawID = drawObjectItr->first;
                            break;
                        }
                    }
                    updateLight(i);
                    break;
                }
            }
        }

        void GraphicsManager::updateLight(unsigned int i, bool recompileShader)
        {
            auto* const osgLight = dynamic_cast<OSGLightStruct*>(myLights[i].lightSource.get());
            if(osgLight == nullptr)
            {
                fprintf(stderr, "GraphicsManager::updateLight -> no Light %u\n", i);
                return;
            }

            osgLight->update(myLights[i].lStruct);
            /*
              if(recompileShader) {
              vector<mars::interfaces::LightData*> lightList;
              map<unsigned long, osg::ref_ptr<OSGNodeStruct> >::iterator iter;
              getLights(&lightList);
              for(iter=drawObjects_.begin(); iter!=drawObjects_.end(); ++iter)
              iter->second->object()->updateShader(lightList, true);
              }
            */
        }

        void GraphicsManager::getLights(vector<mars::interfaces::LightData*> *lightList)
        {
            lightList->clear();
            for (size_t i=0; i<myLights.size(); ++i)
            {
                //return only the used lights
                if (!myLights[i].free)
                {
                    lightList->push_back(&(myLights[i].lStruct));
                }
            }
        }

        void GraphicsManager::getLights(vector<mars::interfaces::LightData> *lightList) const
        {
            lightList->clear();
            for(size_t i=0; i<myLights.size(); ++i)
            {
                //return only the used lights
                if(!myLights[i].free)
                {
                    lightList->push_back(myLights[i].lStruct);
                }
            }
        }

        int GraphicsManager::getLightCount() const
        {
            assert(myLights.size() < static_cast<size_t>(std::numeric_limits<int>::max()));

            // count used lights only
            int size = 0;
            for(size_t i=0; i<myLights.size(); i++)
            {
                if(!myLights[i].free)
                {
                    size++;
                }
            }
            return size;
        }



        ///// DEFAULT 3D OBJECTS

        /** removes actual coordination frame from the scene*/
        void GraphicsManager::hideCoords(const Vector &pos)
        {
            (void) pos;
            if(positionedCoords.get()!=nullptr)
            {
                transformCoords->removeChild(positionedCoords.get());
            }
            if(transformCoords.get()!=nullptr)
            {
                scene->removeChild(transformCoords.get());
            }
        }

        /** removes the main coordination frame from the scene */
        void GraphicsManager::hideCoords()
        {
            scene->removeChild(coords.get());
            show_coords = false;
        }

        /** adds the main coordination frame to the scene */
        void GraphicsManager::showCoords(const Vector &pos, const Quaternion &rot,
                                         const Vector &size)
        {
            hideCoords(pos);

            string resPath = resources_path.sValue;

            positionedCoords = new CoordsPrimitive{osgWidget, size, resPath, true};
            transformCoords = new osg::PositionAttitudeTransform{};
            transformCoords->addChild(positionedCoords.get());
            transformCoords->setAttitude(osg::Quat{rot.x(), rot.y(), rot.z(), rot.w()});
            transformCoords->setPosition(osg::Vec3(pos.x(), pos.y(), pos.z()));
            scene->addChild(transformCoords.get());

            show_coords = true;
        }

        /** adds a local coordination frame to the scene */
        void GraphicsManager::showCoords()
        {
            string resPath = resources_path.sValue;

            coords = new CoordsPrimitive(osgWidget, resPath);
            scene->addChild(coords.get());
            show_coords = true;
        }

        /**
         * closes the joint axis view
         */
        void GraphicsManager::closeAxis()
        {
            scene->removeChild(axisDrawer.get());
        }

        /**
         * draw joint axis, one red line from first node to anchor, one red line from
         * anchor to second node, 2 blue axis lines
         *
         * @param first the first node
         * @param second the anchor
         * @param third the second node
         * @param axis1 the first joint axis
         * @param axis2 the second joint axis
         */
        void GraphicsManager::drawAxis(
            const Vector &first, const Vector &second, const Vector &third,
            const Vector &axis1, const Vector &axis2)
        {
            //remove old axis
            if(axisDrawer!=nullptr)
            {
                scene->removeChild(axisDrawer.get());
            }
            axisDrawer = new AxisPrimitive(first, second, third, axis1, axis2);
            scene->addChild(axisDrawer.get());
        }

        void GraphicsManager::showGrid(void)
        {
            if(!show_grid)
            {
                scene->addChild(grid.get());
            }
            show_grid = true;
        }

        void GraphicsManager::hideGrid(void)
        {
            if(show_grid)
            {
                scene->removeChild(grid.get());
            }
            show_grid = false;
        }

        /* this function is deprecated */
        void GraphicsManager::showClouds()
        {
            showClouds_ = true;
        }

        /* this function is deprecated */
        void GraphicsManager::hideClouds()
        {
            showClouds_ = false;
        }

        ///// PREVIEW NODES

        int GraphicsManager::createPreviewNode(const vector<mars::interfaces::NodeData> &allNodes)
        {
            vector<mars::interfaces::LightData*> lightList;
            getLights(&lightList);

            if(allNodes[0].filename=="PRIMITIVE")
            {
                auto drawObject = osg::ref_ptr<OSGNodeStruct>{new OSGNodeStruct{this, allNodes[0], true, nextPreviewID}};
                previewNodes_[nextPreviewID] = drawObject;
                scene->addChild(drawObject->object()->getPosTransform());
            }
            else
            {
                // TODO: Explain what is happening here.
                for(size_t i=1; i<=previewNodes_.size();++i)
                {
                    auto drawObject = osg::ref_ptr<OSGNodeStruct>{new OSGNodeStruct{this, allNodes[i], true, nextPreviewID}};
                    previewNodes_[nextPreviewID] = drawObject;
                    scene->addChild(drawObject->object()->getPosTransform());
                }
            }
            return nextPreviewID++;
        }

        void GraphicsManager::removePreviewNode(unsigned long id)
        {
            auto needle = previewNodes_.find(id);
            if(needle == std::end(previewNodes_))
            {
                return;
            }

            scene->removeChild(needle->second->object()->getTransform().get());
            previewNodes_.erase(needle);
        }

        /**
         * \throw std::runtime_error if action is \c PREVIEW_COLOR and mat is \c nullptr.
         */
        void GraphicsManager::preview(int action, bool resize,
                                      const vector<mars::interfaces::NodeData> &allNodes,
                                      unsigned int num,
                                      const mars::interfaces::MaterialData *mat)
        {
            switch (action)
            {
                case mars::interfaces::PREVIEW_CREATE:
                    createPreviewNode(allNodes);
                    break;
                case mars::interfaces::PREVIEW_EDIT:
                {
                    size_t i=0;
                    for(auto& previewNode: previewNodes_)
                    {
                        previewNode.second->edit(allNodes[++i], resize);
                    }
                    break;
                }
                case mars::interfaces::PREVIEW_CLOSE:
                    for(const auto& previewNode: previewNodes_)
                    {
                        scene->removeChild(previewNode.second->object()->getTransform().get());
                    }
                    previewNodes_.clear();
                    break;
                case mars::interfaces::PREVIEW_COLOR:
                    for(auto& previewNode: previewNodes_)
                    {
                        if (mat == nullptr)
                        {
                            throw std::runtime_error{"ERROR: Got null-pointer in "
                                                    "GraphicsManager::preview(PREVIEW_COLOR)"};
                        }
                        auto material = osg::ref_ptr<osg::Material>{new OSGMaterialStruct{*mat}};
                        material->setTransparency(osg::Material::FRONT_AND_BACK, 0.8);
                        previewNode.second->getOrCreateStateSet()->setAttributeAndModes(material.get(), osg::StateAttribute::ON);
                    }
                    break;
                default:
                    break;
            }
        }

        ////// HUD

        unsigned long GraphicsManager::addHUDElement(hudElementStruct *he)
        {
            const unsigned long id = next_hud_id++;
            auto elem = osg::ref_ptr<OSGHudElementStruct>(new OSGHudElementStruct(*he, resources_path.sValue, id));
            if(!elem)
            {
                return 0;
            }


            hudElements.push_back(elem);
            for(auto& graphicsWindow: graphicsWindows)
            {
                graphicsWindow->addHUDElement(elem->getHUDElement());
            }
            return id;
        }

        void GraphicsManager::removeHUDElement(unsigned long id)
        {
            auto* const elem = findHUDElement(id);
            if(!elem)
            {
                return;
            }

            for(auto& graphicsWindow: graphicsWindows)
            {
                graphicsWindow->removeHUDElement(elem);
            }

            auto hudElementItr = std::find_if(  std::begin(hudElements), std::end(hudElements),
                                                [&elem](const osg::ref_ptr<OSGHudElementStruct>& x) 
                                                { return x->getHUDElement() == elem; });
            if (hudElementItr != std::end(hudElements))
            {
                hudElements.erase(hudElementItr);
            }
        }

        unsigned long GraphicsManager::addHUDOSGNode(void* node)
        {
            const unsigned long id = next_hud_id++;
            hudElementStruct he;
            he.type = HUD_ELEMENT_OSGNODE;
            auto elem = osg::ref_ptr<OSGHudElementStruct>(new OSGHudElementStruct(he, resources_path.sValue, id, static_cast<osg::Node*>(node)));
            if(!elem)
            {
                return 0;
            }
            
            hudElements.push_back(elem);
            for (auto& graphicsWindow: graphicsWindows)
            {
                graphicsWindow->addHUDElement(elem->getHUDElement());
            }
            return id;
        }

        HUDElement* GraphicsManager::findHUDElement(unsigned long id) const
        {
            auto hudElementItr = std::find_if(  std::begin(hudElements), std::end(hudElements), 
                                                [&id](const osg::ref_ptr<OSGHudElementStruct>& x) 
                                                { return x->getHUDElement()->getID() == id; });
            if (hudElementItr == std::end(hudElements))
            {
                return nullptr;
            }

            return (*hudElementItr)->getHUDElement();
        }

        void GraphicsManager::switchHUDElementVis(unsigned long id)
        {
            auto* const elem = dynamic_cast<HUDTexture*>(findHUDElement(id));
            if(elem==nullptr)
            {
                return;
            }

            elem->switchCullMask();
        }

        void GraphicsManager::setHUDElementPos(unsigned long id, double x, double y)
        {
            auto* const elem = dynamic_cast<HUDTexture*>(findHUDElement(id));
            if(elem==nullptr)
            {
                return;
            }

            elem->setPos(x, y);
        }

        void GraphicsManager::setHUDElementTexture(unsigned long id,
                                                   std::string texturename)
        {
            auto* const elem = dynamic_cast<HUDTexture*>(findHUDElement(id));
            if(elem==nullptr)
            {
                return;
            }

            elem->setTexture(GuiHelper::loadTexture(texturename).get());
        }

        void GraphicsManager::setHUDElementTextureData(unsigned long id,
                                                       void* data)
        {
            auto* const elem = dynamic_cast<HUDTexture*>(findHUDElement(id));
            if(elem==nullptr)
            {
                return;
            }

            elem->setImageData(data);
        }

        void GraphicsManager::setHUDElementTextureRTT(unsigned long id,
                                                      unsigned long window_id,
                                                      bool depthComponent)
        {
            auto* const elem = dynamic_cast<HUDTexture*>(findHUDElement(id));
            if(elem==nullptr)
            {
                return;
            }

            auto graphicsWindowItr = std::find_if(std::begin(graphicsWindows), std::end(graphicsWindows),
                                                    [window_id](GraphicsWidget* const x) { return x->getID() == window_id; });
            if(graphicsWindowItr == std::end(graphicsWindows))
            {
                return;
            }

            elem->setTexture(depthComponent ? (*graphicsWindowItr)->getRTTDepthTexture() : (*graphicsWindowItr)->getRTTTexture());
        }

        void GraphicsManager::setHUDElementLabel(unsigned long id,
                                                 std::string text,
                                                 double text_color[4])
        {
            auto* const elem = dynamic_cast<HUDLabel*>(findHUDElement(id));
            if(elem==nullptr)
            {
                return;
            }

            elem->setText(text, text_color);
        }

        void GraphicsManager::setHUDElementLines(unsigned long id,
                                                 std::vector<double> *v,
                                                 double color[4])
        {
            auto* const elem = dynamic_cast<HUDLines*>(findHUDElement(id));
            if(elem==nullptr)
            {
                return;
            }

            elem->setLines(v, color);
        }

        ////// EVENTS

        void GraphicsManager::addGuiEventHandler(GuiEventInterface *_guiEventHandler)
        {
            auto guiHandlerItr = std::find(std::begin(guiHandlerList), std::end(guiHandlerList), _guiEventHandler);
            if (guiHandlerItr != std::end(guiHandlerList))
            {
                // nothing to add - already in list
                return;
            }

            guiHandlerList.push_back(_guiEventHandler);
        }

        void GraphicsManager::removeGuiEventHandler(GuiEventInterface *_guiEventHandler)
        {
            auto guiHandlerItr = std::find(std::begin(guiHandlerList), std::end(guiHandlerList), _guiEventHandler);
            if (guiHandlerItr == std::end(guiHandlerList))
            {
                return;
            }

            guiHandlerList.erase(guiHandlerItr);
        }

        void GraphicsManager::emitKeyDownEvent(int key, unsigned int modKey,
                                               unsigned long win_id)
        {
            for(auto& guiHandler: guiHandlerList)
            {
                guiHandler->keyDownEvent(key, modKey, win_id);
            }
        }

        void GraphicsManager::emitKeyUpEvent(int key, unsigned int modKey,
                                             unsigned long win_id)
        {
            for(auto& guiHandler: guiHandlerList)
            {
                guiHandler->keyUpEvent(key, modKey, win_id);
            }
        }

        void GraphicsManager::emitQuitEvent(unsigned long win_id)
        {
            if(win_id < 1)
            {
                return;
            }

            for(auto& guiHandler: guiHandlerList)
            {
                guiHandler->quitEvent(win_id);
            }
        }

        void GraphicsManager::emitSetAppActive(unsigned long win_id)
        {
            if(win_id < 1)
            {
                return;
            }

            for(auto& guiHandler: guiHandlerList)
            {
                guiHandler->setAppActive(win_id);
            }
        }

        void GraphicsManager::addEventClient(GraphicsEventClient* theClient)
        {
            auto eventClientItr = std::find(std::begin(graphicsEventClientList), std::end(graphicsEventClientList), theClient);
            if (eventClientItr != std::end(graphicsEventClientList))
            {
                // already in list - nothing to add
                return;
            }

            graphicsEventClientList.push_back(theClient);
        }

        void GraphicsManager::removeEventClient(GraphicsEventClient* theClient)
        {
            auto eventClientItr = std::find(std::begin(graphicsEventClientList), std::end(graphicsEventClientList), theClient);
            if (eventClientItr == std::end(graphicsEventClientList))
            {
                return;
            }

            graphicsEventClientList.erase(eventClientItr);
        }

        void GraphicsManager::emitNodeSelectionChange(unsigned long win_id, int mode)
        {
            // TODO: Refactor this method
            if(win_id < 1 || mode == 0)
            {
                return;
            }

            std::vector<GraphicsEventClient*>::iterator jter;

            GraphicsWidget* gw = getGraphicsWindow(win_id);

            std::vector<osg::Node*> selectednodes = gw->getPickedObjects();
            if(selectednodes.empty())
            {
                return;
            }

            DrawObjectList::iterator drawListIt;
            DrawObjects::iterator drawIt;
            std::vector<osg::Node*>::iterator nodeit;

            bool selection = true;

            switch(mode)
            {
            case 0:
                break;
                // Pickmode STANDARD
            case 1:
                /* Before attempting to add the object as newly selected, check
                 *  if already selected. If yes, remove it from the list.
                 */
                if(!selectedObjects_.empty())
                {
                    // scan for objects to potentially remove
                    for(nodeit=selectednodes.begin(); nodeit!=selectednodes.end(); ++nodeit)
                    {
                        // and try to find them in the list of already selected objects
                        for(drawListIt=selectedObjects_.begin(); drawListIt!=selectedObjects_.end();
                            ++drawListIt)
                        {
                            /* In case we find the object, we have to remove the object
                             *  from the list of selected objects
                             */
                            if((*drawListIt)->object()->containsNode((*nodeit)))
                            {

                                for(jter=graphicsEventClientList.begin();
                                    jter!=graphicsEventClientList.end();
                                    ++jter)
                                    (*jter)->selectEvent(findCoreObject((*drawListIt)->object()->getID()), false);

                                (*drawListIt)->object()->setSelected(false);
                                selectedObjects_.erase(drawListIt);

                                selection = false;

                                if(selectedObjects_.empty())
                                    break;
                                else
                                    drawListIt = selectedObjects_.begin();

                            }
                        }
                        /* in case we previously erased all objects from the selectedobjects
                         *  list, we want to stop searching
                         */
                        if(selectedObjects_.empty())
                            break;
                    }
                }
                /* If we didn't find our picked objects in the list of selected
                 * objects, we can add them to our selections - if they are valid,
                 * of course.
                 */
                if(selection)
                {
                    // scan for objects to potentially add
                    for(nodeit=selectednodes.begin(); nodeit!=selectednodes.end(); ++nodeit)
                    {
                        // and try to verify their existance as drawobject
                        for(drawIt=drawObjects_.begin(); drawIt!=drawObjects_.end(); ++drawIt)
                        {
                            /* In case we find the corresponding drawobject, we have to:
                             * add them to the list of selected objects.
                             */
                            if(drawIt->second->object()->containsNode((*nodeit)))
                            {
                                drawIt->second->object()->setSelected(true);
                                selectedObjects_.push_back(drawIt->second.get());

                                for(jter=graphicsEventClientList.begin();
                                    jter!=graphicsEventClientList.end();
                                    ++jter)
                                    (*jter)->selectEvent(findCoreObject(drawIt->second->object()->getID()), true);

                                // increase nodeit to make sure we do not add this node again.
                                //nodeit++;
                                break;
                            }
                        }
                    }
                }
                break;

                // Pickmode FORCE_ADD
            case 2:
                break;

                // Pickmode FORCE_REMOVE
            case 3:
                break;
            case 4:
            {
                while((drawListIt=selectedObjects_.begin()) != selectedObjects_.end())
                {
                    for(jter=graphicsEventClientList.begin();
                        jter!=graphicsEventClientList.end();
                        ++jter)
                    {
                        (*jter)->selectEvent(findCoreObject((*drawListIt)->object()->getID()), false);
                    }
                    (*drawListIt)->object()->setSelected(false);
                    selectedObjects_.erase(drawListIt);
                }
                // scan for objects to potentially add
                for(nodeit=selectednodes.begin(); nodeit!=selectednodes.end(); ++nodeit)
                {
                    // and try to verify their existance as drawobject
                    for(drawIt=drawObjects_.begin(); drawIt!=drawObjects_.end(); ++drawIt)
                    {
                        /* In case we find the corresponding drawobject, we have to:
                         * add them to the list of selected objects.
                         */
                        if(drawIt->second->object()->containsNode((*nodeit)))
                        {
                            drawIt->second->object()->setSelected(true);
                            selectedObjects_.push_back(drawIt->second.get());

                            for(jter=graphicsEventClientList.begin();
                                jter!=graphicsEventClientList.end();
                                ++jter)
                                (*jter)->selectEvent(findCoreObject(drawIt->second->object()->getID()), true);

                            // increase nodeit to make sure we do not add this node again.
                            //nodeit++;
                            break;
                        }
                    }
                }
                break;
            }
            }
            gw->clearSelectionVectors();

        }

        void GraphicsManager::showNormals(bool val)
        {
            for(auto& drawObject: drawObjects_)
            {
                drawObject.second->object()->showNormals(val);
            }
        }

        void GraphicsManager::showRain(bool val)
        {
            if(val)
            {
                rain = new osgParticle::PrecipitationEffect;
                rain->setWind(osg::Vec3{1, 0, 0});
                rain->setParticleSpeed(0.4);
                rain->rain(0.6); // alternatively, use rain
                scene->addChild(rain.get());
            }
            else
            {
                scene->removeChild(rain.get());
            }
        }

        void GraphicsManager::showSnow(bool val)
        {
            if(val)
            {
                snow = new osgParticle::PrecipitationEffect;
                snow->setWind(osg::Vec3{1, 0, 0});
                snow->setParticleSpeed(0.4);
                snow->snow(0.4); // alternatively, use rain
                scene->addChild(snow.get());
            }
            else
            {
                scene->removeChild(snow.get());
            }
        }

        void GraphicsManager::setupCFG(void)
        {
            auto* const cfgClient = static_cast<cfg_manager::CFGClient*>(this);
            cfgW_top = cfg->getOrCreateProperty("Graphics", "window1Top", (int)40,
                                                cfgClient);

            cfgW_left = cfg->getOrCreateProperty("Graphics", "window1Left", (int)700,
                                                 cfgClient);

            cfgW_width = cfg->getOrCreateProperty("Graphics", "window1Width", (int)720,
                                                  cfgClient);

            cfgW_height = cfg->getOrCreateProperty("Graphics", "window1Height", (int)405,
                                                   cfgClient);

            draw_normals = cfg->getOrCreateProperty("Graphics", "draw normals", false,
                                                    cfgClient);

            brightness = cfg->getOrCreateProperty("Graphics", "brightness", 1.0,
                                                  cfgClient);

            noiseAmmount = cfg->getOrCreateProperty("Graphics", "noiseAmmount", 0.05,
                                                    cfgClient);

            grab_frames = cfg->getOrCreateProperty("Graphics", "make movie", false,
                                                   cfgClient);

            marsShader = cfg->getOrCreateProperty("Graphics", "marsShader", true,
                                                  cfgClient);

            drawRain = cfg->getOrCreateProperty("Graphics", "drawRain", false,
                                                cfgClient);

            drawSnow = cfg->getOrCreateProperty("Graphics", "drawSnow", false,
                                                cfgClient);

            showFramesProp = cfg->getOrCreateProperty("Graphics", "showFrames", false,
                                                      cfgClient);
            scaleFramesProp = cfg->getOrCreateProperty("Graphics", "scaleFrames", 0.1,
                                                       cfgClient);

            drawMainCamera = cfg->getOrCreateProperty("Graphics", "drawMainCamera", true,
                                                      cfgClient);

            backfaceCulling = cfg->getOrCreateProperty("Graphics", "backfaceCulling",
                                                       true, cfgClient);

            setGraphicsWindowGeometry(1, cfgW_top.iValue, cfgW_left.iValue,
                                      cfgW_width.iValue, cfgW_height.iValue);
            if(drawRain.bValue)
            {
                showRain(true);
            }
            if(drawSnow.bValue)
            {
                showSnow(true);
            }
            if(showFramesProp.bValue)
            {
                showFrames(true);
            }
            scaleFrames(scaleFramesProp.dValue);
            if(!drawMainCamera.bValue)
            {
                deactivate3DWindow(1);
            }

        }

        void GraphicsManager::cfgUpdateProperty(cfg_manager::cfgPropertyStruct _property)
        {
            bool change_view = 0;

            if(set_window_prop)
            {
                return;
            }

            if(_property.paramId == cfgW_top.paramId)
            {
                cfgW_top.iValue = _property.iValue;
                change_view = 1;
            }
            else if(_property.paramId == cfgW_left.paramId)
            {
                cfgW_left.iValue = _property.iValue;
                change_view = 1;
            }
            else if(_property.paramId == cfgW_width.paramId)
            {
                cfgW_width.iValue = _property.iValue;
                change_view = 1;
            }
            else if(_property.paramId == cfgW_height.paramId)
            {
                cfgW_height.iValue = _property.iValue;
                change_view = 1;
            }

            if(change_view)
            {
                // we get four callback on the resize that we want to ignore
#ifdef QT_OSG_MIX
                ignore_next_resize += 1;
#else
                ignore_next_resize += 4;
#endif
                setGraphicsWindowGeometry(1, cfgW_top.iValue, cfgW_left.iValue,
                                          cfgW_width.iValue, cfgW_height.iValue);
                return;
            }

            if(_property.paramId == draw_normals.paramId)
            {
                showNormals(_property.bValue);
                return;
            }

            if(_property.paramId == drawRain.paramId)
            {
                showRain(_property.bValue);
                return;
            }

            if(_property.paramId == drawSnow.paramId)
            {
                showSnow(_property.bValue);
                return;
            }

            if(_property.paramId == showFramesProp.paramId)
            {
                showFramesProp.bValue = _property.bValue;
                showFrames(_property.bValue);
                return;
            }

            if(_property.paramId == scaleFramesProp.paramId)
            {
                scaleFramesProp.dValue = _property.dValue;
                scaleFrames(_property.dValue);
                return;
            }

            if(_property.paramId == drawMainCamera.paramId)
            {
                drawMainCamera.bValue = _property.bValue;
                if(drawMainCamera.bValue)
                {
                    activate3DWindow(1);
                }
                else
                {
                    deactivate3DWindow(1);
                }
                return;
            }

            if(_property.paramId == multisamples.paramId)
            {
                setMultisampling(_property.iValue);
                return;
            }

            if(_property.paramId == noiseProp.paramId)
            {
                setUseNoise(_property.bValue);
                return;
            }

            if(_property.paramId == drawLineLaserProp.paramId)
            {
                setDrawLineLaser(_property.bValue);
                return;
            }

            if(_property.paramId == brightness.paramId)
            {
                setBrightness(_property.dValue);
                return;
            }

            if(_property.paramId == noiseAmmount.paramId)
            {
                setNoiseAmmount(_property.dValue);
                return;
            }

            if(_property.paramId == marsShader.paramId)
            {
                setUseShader(_property.bValue);
                return;
            }

            if(_property.paramId == marsShadow.paramId)
            {
                setUseShadow(_property.bValue);
                return;
            }

            if(_property.paramId == shadowTechnique.paramId)
            {
                setShadowTechnique(_property.sValue);
                return;
            }

            if(_property.paramId == shadowSamples.paramId)
            {
                setShadowSamples(_property.iValue);
                return;
            }

            if(_property.paramId == backfaceCulling.paramId)
            {
                if((backfaceCulling.bValue = _property.bValue))
                {
                    globalStateset->setAttributeAndModes(cull, osg::StateAttribute::ON);
                }
                else
                {
                    globalStateset->setAttributeAndModes(cull, osg::StateAttribute::OFF);
                }
                return;
            }

            if(_property.paramId == grab_frames.paramId)
            {
                setGrabFrames(_property.bValue);
                return;
            }

            if(_property.paramId == showGridProp.paramId)
            {
                showGridProp.bValue = _property.bValue;
                if(showGridProp.bValue)
                {
                    showGrid();
                }
                else
                {
                    hideGrid();
                }
                return;
            }

            if(_property.paramId == showCoordsProp.paramId)
            {
                showCoordsProp.bValue = _property.bValue;
                if(showCoordsProp.bValue){
                    showCoords();
                }
                else
                {
                    hideCoords();
                }
                return;
            }

            if(_property.paramId == showSelectionProp.paramId)
            {
                showSelectionProp.bValue = _property.bValue;
                for(auto& drawObject: drawObjects_)
                {
                    drawObject.second->object()->setShowSelected(showSelectionProp.bValue);
                }
                return;
            }
        }

        void GraphicsManager::emitGeometryChange(unsigned long win_id, int left,
                                                 int top, int width, int height)
        {
            bool update_cfg = false;
            if(win_id==1)
            {
                if(ignore_next_resize>0)
                {
                    --ignore_next_resize;
                    return;
                }

                if(top != cfgW_top.iValue)
                {
                    cfgW_top.iValue = top;
                    update_cfg = true;
                }
                if(left != cfgW_left.iValue)
                {
                    cfgW_left.iValue = left;
                    update_cfg = true;
                }
                if(width != cfgW_width.iValue)
                {
                    cfgW_width.iValue = width;
                    update_cfg = true;
                }
                if(height != cfgW_height.iValue)
                {
                    cfgW_height.iValue = height;
                    update_cfg = true;
                }
                if(update_cfg && cfg)
                {
                    set_window_prop = true;
                    cfg->setProperty(cfgW_top);
                    cfg->setProperty(cfgW_left);
                    cfg->setProperty(cfgW_width);
                    cfg->setProperty(cfgW_height);
                    set_window_prop = false;
                }
            }
        }

        void GraphicsManager::setMultisampling(int num_samples)
        {
            //Antialiasing
            if(num_samples < 0)
            {
                printf("\"num multisamples\" have to be a positiv number!");
                num_samples = 0;
            }
            osg::DisplaySettings::instance()->setNumMultiSamples(num_samples);
        }

        void GraphicsManager::setBrightness(double val)
        {
            if(materialManager)
            {
                materialManager->setBrightness(static_cast<float>(val));
            }
        }

        void GraphicsManager::setNoiseAmmount(double val)
        {
            if(materialManager)
            {
                materialManager->setNoiseAmmount(static_cast<float>(val));
            }
        }

        void GraphicsManager::setUseNoise(bool val)
        {
            useNoise = noiseProp.bValue = val;
            if(materialManager)
            {
                materialManager->setUseNoise(val);
            }
        }

        void GraphicsManager::setDrawLineLaser(bool val)
        {
            drawLineLaser = drawLineLaserProp.bValue = val;
            if(materialManager)
            {
                materialManager->setDrawLineLaser(val);
            }
        }

        void GraphicsManager::setUseShader(bool val)
        {
            if(materialManager)
            {
                materialManager->setUseShader(val);
            }
            if(val)
            {
                if(shadowMap.valid())
                {
                    shadowMap->addTexture(shadowStateset.get());
                }
                if(pssm.valid())
                {
                    pssm->addTexture(shadowStateset.get());
                }
            }
            else
            {
                if(shadowMap.valid())
                {
                    shadowMap->removeTexture(shadowStateset.get());
                }
                if(pssm.valid())
                {
                    pssm->removeTexture(shadowStateset.get());
                }
            }
            /*
              map<unsigned long, osg::ref_ptr<OSGNodeStruct> >::iterator iter;

              for(iter=drawObjects_.begin(); iter!=drawObjects_.end(); ++iter)
              iter->second->object()->setUseMARSShader(val);
            */
        }

        void GraphicsManager::setShadowSamples(int v)
        {
            shadowSamples.iValue = v;
            if(materialManager)
            {
                materialManager->setShadowSamples(v);
                materialManager->updateShadowSamples();
            }
        }

        void GraphicsManager::initDefaultLight()
        {
            defaultLight.lStruct.pos = Vector{2.0, 2.0, 10.0};
            defaultLight.lStruct.ambient = mars::utils::Color{0.5, 0.5, 0.5, 1.0};
            defaultLight.lStruct.diffuse = mars::utils::Color{1., 1., 1., 1.0};
            defaultLight.lStruct.specular = mars::utils::Color{1.0, 1.0, 1.0, 1.0};
            defaultLight.lStruct.constantAttenuation = 1.0;
            defaultLight.lStruct.linearAttenuation = 0.0;
            defaultLight.lStruct.quadraticAttenuation = 0.00002;
            defaultLight.lStruct.directional = true;
            defaultLight.lStruct.type = mars::interfaces::OMNILIGHT;
            defaultLight.lStruct.index = 0;
            defaultLight.lStruct.angle = 0;
            defaultLight.lStruct.exponent = 0;

            auto myLightSource = osg::ref_ptr<osg::LightSource>{new OSGLightStruct{defaultLight.lStruct}};

            //add to lightmanager for later editing possibility
            defaultLight.light = myLightSource->getLight();
            defaultLight.lightSource = myLightSource;
            defaultLight.free = false;

            lightGroup->addChild( myLightSource.get() );
            globalStateset->setMode(GL_LIGHT0, osg::StateAttribute::ON);
            myLightSource->setStateSetModes(*globalStateset, osg::StateAttribute::ON);
        }

        void* GraphicsManager::getWindowManager(int id)
        {
            auto* const gw = getGraphicsWindow(id);
            if(gw == nullptr)
            {
                std::cerr<<"window does not exist!"<<std::endl;
                return gw;
            }

            return static_cast<void*>(gw->getOrCreateWindowManager());
        }

        bool GraphicsManager::coordsVisible(void) const
        {
            return show_coords;
        }

        bool GraphicsManager::gridVisible(void) const
        {
            return show_grid;
        }

        bool GraphicsManager::cloudsVisible(void) const
        {
            return showClouds_;
        }

        void GraphicsManager::collideSphere(unsigned long id, Vector pos,
                                            mars::interfaces::sReal radius)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                return;
            }

            ns->object()->collideSphere(pos, radius);
        }

        const Vector& GraphicsManager::getDrawObjectPosition(unsigned long id)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                static Vector dummy;
                return dummy;
            }

            return ns->object()->getPosition();
        }

        const Quaternion& GraphicsManager::getDrawObjectQuaternion(unsigned long id)
        {
            auto* const ns = findDrawObject(id);
            if(ns == nullptr)
            {
                static Quaternion dummy;
                return dummy;
            }
            return ns->object()->getQuaternion();
        }

        interfaces::LoadMeshInterface* GraphicsManager::getLoadMeshInterface(void)
        {
            return guiHelper;
        }

        interfaces::LoadHeightmapInterface* GraphicsManager::getLoadHeightmapInterface(void)
        {
            return guiHelper;
        }

        void GraphicsManager::makeChild(unsigned long parentId,
                                        unsigned long childId)
        {
            auto* const parent = findDrawObject(parentId);
            auto* const child = findDrawObject(childId);
            osg::PositionAttitudeTransform *parentTransform;
            osg::PositionAttitudeTransform *childTransform;

            if(!parent || !child)
            {
                return;
            }
            parentTransform = parent->object()->getPosTransform();
            childTransform = child->object()->getPosTransform();
            parent->object()->addSelectionChild(child->object());
            parentTransform->addChild(childTransform);
            child->object()->seperateMaterial();
            //scene->removeChild(childTransform);
            //shadowedScene->removeChild(childTransform);
        }

        void GraphicsManager::attacheCamToNode(unsigned long winId, unsigned long drawId)
        {
            const auto* const gw = getGraphicsWindow(winId);
            auto* const gc = dynamic_cast<GraphicsCamera*>(gw->getCameraInterface());

            auto* const parent = findDrawObject(drawId);
            if(parent)
            {
                const auto& parentTransform = parent->object()->getPosTransform();
                gc->setTrakingTransform(parentTransform);
            }
            else
            {
                gc->setTrakingTransform(nullptr);
            }
        }

        void GraphicsManager::setExperimentalLineLaser(utils::Vector pos, utils::Vector normal, utils::Vector color, utils::Vector laserAngle, float openingAngle)
        {
            if(materialManager)
            {
                materialManager->setExperimentalLineLaser(pos, normal, color,
                                                          laserAngle, openingAngle);
            }
        }

        void GraphicsManager::addOSGNode(void* node)
        {
            scene->addChild(static_cast<osg::Node*>(node));
        }

        void GraphicsManager::removeOSGNode(void* node)
        {
            scene->removeChild(static_cast<osg::Node*>(node));
        }

        osg_material_manager::MaterialNode* GraphicsManager::getMaterialNode(const std::string &name)
        {
            if(!materialManager)
            {
                return nullptr;
            }

            return materialManager->getNewMaterialGroup(name);
        }

        void GraphicsManager::addMaterial(const interfaces::MaterialData &material)
        {
            if(!materialManager)
            {
                return;
            }

            configmaps::ConfigMap map;
            auto m = material;
            m.toConfigMap(&map);
            materialManager->createMaterial(material.name, map);
        }

        osg_material_manager::MaterialNode* GraphicsManager::getSharedStateGroup(unsigned long id)
        {
            auto iter = drawObjects_.find(id);
            if(iter==std::end(drawObjects_))
            {
                return nullptr;
            }

            return iter->second->object()->getStateGroup();
        }

        void GraphicsManager::setUseShadow(bool v)
        {
            marsShadow.bValue = v;

            if(v)
            {
                if(pssm.valid())
                {
                    shadowedScene->setShadowTechnique(pssm.get());
                }
                else if(shadowMap.valid())
                {
                    shadowedScene->setShadowTechnique(shadowMap.get());
                }
            }
            else
            {
                shadowedScene->setShadowTechnique(nullptr);
            }
            if(materialManager)
            {
                materialManager->setUseShadow(v);
            }
        }

        void GraphicsManager::initShadowMap()
        {
            if(!shadowMap.valid())
            {
                shadowMap = new ShadowMap;
                shadowMap->setShadowTextureSize(shadowTextureSize.iValue);
                shadowMap->initTexture();
                shadowMap->applyState(shadowStateset.get());
            }
            if(marsShadow.bValue)
            {
                shadowedScene->setShadowTechnique(shadowMap.get());
            }
            for(auto& lm: myLights)
            {
                if(lm.lStruct.map.hasKey("produceShadow"))
                {
                    if(static_cast<bool>(lm.lStruct.map["produceShadow"]))
                    {
                        shadowMap->setLight(lm.light.get());
                    }
                }
            }
            if(materialManager && materialManager->getUseShader())
            {
                shadowMap->addTexture(shadowStateset.get());
            }
        }

        void GraphicsManager::initShadowPSSM()
        {
            bool useShader = false;
            if(!pssm.valid())
            {
                pssm = new ParallelSplitShadowMap{nullptr, NUM_PSSM_SPLITS, shadowSubTextureSize.iValue};

                //pssm->enableShadowGLSLFiltering(false);
                pssm->setTextureResolution(shadowTextureSize.iValue);
                pssm->setMinNearDistanceForSplits(0);
                pssm->setMaxFarDistance(500);
                pssm->setMoveVCamBehindRCamFactor(0);
                pssm->setPolygonOffset(osg::Vec2{1.2,1.2});
                //pssm->applyState(shadowStateset.get());
                pssm->applyState(scene->getOrCreateStateSet());
                if(materialManager)
                {
                    useShader = materialManager->getUseShader();
                    pssm->applyState(materialManager->getMainStateGroup()->getOrCreateStateSet());
                }
            }
            if(marsShadow.bValue)
            {
                shadowedScene->setShadowTechnique(pssm.get());
            }

            for(auto& lm: myLights)
            {
                if(lm.lStruct.map.hasKey("produceShadow"))
                {
                    if(static_cast<bool>(lm.lStruct.map["produceShadow"]))
                    {
                        pssm->setLight(lm.light.get());
                    }
                }
            }

            if(useShader)
            {
                pssm->addTexture(shadowStateset.get());
            }
        }

        void GraphicsManager::setShadowTechnique(const std::string& s)
        {
            if(shadowTechnique.sValue != s)
            {
                shadowedScene->setShadowTechnique(nullptr);
                if(shadowMap.valid())
                {
                    shadowMap->removeState(scene->getOrCreateStateSet());
                    shadowMap->removeTexture(shadowStateset.get());
                    shadowMap = nullptr;
                }
                if(pssm.valid())
                {
                    pssm->removeState(scene->getOrCreateStateSet());
                    if(materialManager)
                    {
                        pssm->removeState(materialManager->getMainStateGroup()->getOrCreateStateSet());
                    }
                    pssm->removeTexture(shadowStateset.get());
                    pssm = nullptr;
                }

                shadowTechnique.sValue = "none";
                if(s=="pssm")
                {
                    initShadowPSSM();
                    shadowTechnique.sValue = s;
                }
                else if(s=="sm")
                {
                    initShadowMap();
                    shadowTechnique.sValue = s;
                }
                else if(s=="none")
                {
                    // nothing to do here
                }
                else
                {
                    fprintf(stderr, "Selected shadow technique *%s* unknown, set to \"none\". Please select from [none, sm, pssm]\n", s.c_str());
                }

                if(materialManager)
                {
                    materialManager->setShadowTechnique(s);
                }
            }
        }

        void GraphicsManager::setCameraDefaultView(int view)
        {
            if(!activeWindow)
            {
                return;
            }
            auto* const cam = activeWindow->getCameraInterface();
            switch(view)
            {
            case 1:
                cam->context_setCamPredefTop();
                break;
            case 2:
                cam->context_setCamPredefFront();
                break;
            case 3:
                cam->context_setCamPredefRight();
                break;
            case 4:
                cam->context_setCamPredefRear();
                break;
            case 5:
                cam->context_setCamPredefLeft();
                break;
            case 6:
                cam->context_setCamPredefBottom();
                break;
            default:
                break;
            }
        }

        void GraphicsManager::edit(const std::string &key,
                                   const std::string &value)
        {
            fprintf(stderr, "GraphicsManager::edit(%s, %s)\n", key.c_str(), value.c_str());
            if(matchPattern("*/fogEnable", key))
            {
                if(tolower(value) == "true" || value == "1")
                {
                    graphicOptions.fogEnabled = true;
                }
                else if(tolower(value) == "false" || value == "0")
                {
                    graphicOptions.fogEnabled = false;
                }
            }
            else if(matchPattern("*/fogColor", key))
            {
                double v = atof(value.c_str());
                if(key[key.size()-1] == 'a')
                {
                    graphicOptions.fogColor.a = v;
                }
                else if(key[key.size()-1] == 'r')
                {
                    graphicOptions.fogColor.r = v;
                }
                else if(key[key.size()-1] == 'g')
                {
                    graphicOptions.fogColor.g = v;
                }
                else if(key[key.size()-1] == 'b')
                {
                    graphicOptions.fogColor.b = v;
                }
            }
            else if(matchPattern("*/fogStart", key))
            {
                graphicOptions.fogStart = atof(value.c_str());
            }
            else if(matchPattern("*/fogEnd", key))
            {
                graphicOptions.fogEnd = atof(value.c_str());
            }
            else if(matchPattern("*/fogDensity", key))
            {
                graphicOptions.fogDensity = atof(value.c_str());
            }
            setGraphicOptions(graphicOptions);
        }

        void GraphicsManager::edit(unsigned long widgetID, const std::string &key,
                                   const std::string &value)
        {
            GraphicsWindowInterface *win;
            if(widgetID == 0)
            {
                if(!activeWindow)
                {
                    return;
                }
                win = activeWindow;
            }
            else
            {
                win = get3DWindow(widgetID);
            }
            auto* const cam = win->getCameraInterface();
            if(matchPattern("*/projection", key))
            {
                if(value == "perspective")
                {
                    cam->changeCameraTypeToPerspective();
                }
                else if(value == "orthogonal")
                {
                    cam->changeCameraTypeToOrtho();
                }
            }
            else if(matchPattern("*/mouse", key))
            {
                if(value == "default")
                {
                    cam->setCamera(ODE_CAM);
                }
                else if(value == "invert")
                {
                    cam->setCamera(MICHA_CAM);
                }
                else if(value == "osg")
                {
                    cam->setCamera(OSG_CAM);
                }
                else if(value == "iso")
                {
                    cam->setCamera(ISO_CAM);
                }
                else if(value == "trackball")
                {
                    cam->setCamera(TRACKBALL);
                }
                else if(value == "toggle_trackball")
                {
                    cam->toggleTrackball();
                }
            }
            else if(matchPattern("*/clearColor", key))
            {
                double v = atof(value.c_str());
                auto clearColor = win->getClearColor();
                if(key[key.size()-1] == 'a')
                {
                    clearColor.a = v;
                }
                if(key[key.size()-1] == 'r')
                {
                    clearColor.r = v;
                }
                if(key[key.size()-1] == 'g')
                {
                    clearColor.g = v;
                }
                if(key[key.size()-1] == 'b')
                {
                    clearColor.b = v;
                }
                win->setClearColor(clearColor);
            }
            else if(matchPattern("*/orthoH", key))
            {
                double v = atof(value.c_str());
                cam->setOrthoH(v);
            }
            else if(matchPattern("*/position", key))
            {
                double d = atof(value.c_str());
                double v[7];
                if(cam->isTracking())
                {
                    cam->getOffsetQuat(v, v+1, v+2, v+3, v+4, v+5, v+6);
                }
                else
                {
                    cam->getViewportQuat(v, v+1, v+2, v+3, v+4, v+5, v+6);
                }

                if(key[key.size()-1] == 'x')
                {
                    v[0] = d;
                }
                else if(key[key.size()-1] == 'y')
                {
                    v[1] = d;
                }
                else if(key[key.size()-1] == 'z')
                {
                    v[2] = d;
                }

                if(cam->isTracking())
                {
                    cam->setOffsetQuat(v[0], v[1], v[2], v[3], v[4], v[5], v[6]);
                }
                else
                {
                    cam->updateViewportQuat(v[0], v[1], v[2], v[3], v[4], v[5], v[6]);
                }
            }
            else if(matchPattern("*/euler", key))
            {
                double d = atof(value.c_str());
                double v[7];
                Quaternion q;
                if(cam->isTracking())
                {
                    cam->getOffsetQuat(v, v+1, v+2, v+3, v+4, v+5, v+6);
                }
                else
                {
                    cam->getViewportQuat(v, v+1, v+2, v+3, v+4, v+5, v+6);
                }
                q.x() = v[3];
                q.y() = v[4];
                q.z() = v[5];
                q.w() = v[6];
                auto r = quaternionTosRotation(q);
                if(key.find("alpha") != string::npos)
                {
                    r.alpha = d;
                }
                else if(key.find("beta") != string::npos)
                {
                    r.beta = d;
                }
                else if(key.find("gamma") != string::npos)
                {
                    r.gamma = d;
                }
                q = eulerToQuaternion(r);
                if(cam->isTracking())
                {
                    cam->setOffsetQuat(v[0], v[1], v[2], q.x(), q.y(), q.z(), q.w());
                }
                else
                {
                    cam->updateViewportQuat(v[0], v[1], v[2], q.x(), q.y(), q.z(), q.w());
                }
            }
            else if(matchPattern("*/moveSpeed", key))
            {
                cam->setMoveSpeed(atof(value.c_str()));
            }
            else if(matchPattern("*/logTrackingRotation", key))
            {
                cam->setTrackingLogRotation(atoi(value.c_str()));
            }
        }

        osg::Vec3f GraphicsManager::getSelectedPos()
        {
            if (selectedObjects_.size() == 0)
            {
                return osg::Vec3f{0, 0, 0};
            }

            for(auto& selectedObject: selectedObjects_)
            {
                const auto& p = selectedObject->object()->getPosition();
                return osg::Vec3(p.x(), p.y(), p.z());
                // TODO: multiple selections?
            }
        }

        void GraphicsManager::showFrames(bool val)
        {
            for(auto& drawObject: drawObjects_)
            {
                auto& frame = drawObject.second->object()->frame;
                if(frame)
                {
                    auto node = static_cast<osg::PositionAttitudeTransform*>(frame->getOSGNode());
                    if(val)
                    {
                        scene->addChild(node);
                    }
                    else
                    {
                        scene->removeChild(node);
                    }
                }
            }
        }

        void GraphicsManager::scaleFrames(double x)
        {
            for(auto& drawObject: drawObjects_)
            {
                auto& frame = drawObject.second->object()->frame;
                if(frame)
                {
                    frame->setScale(x);
                }
            }
        }

        void GraphicsManager::lock()
        {
            mutex.lock();
        }

        void GraphicsManager::unlock()
        {
            mutex.unlock();
        }

        void GraphicsManager::produceData(const data_broker::DataInfo &info,
                                          data_broker::DataPackage *dbPackage,
                                          int callbackParam)
        {
            dbPackageMapping.writePackage(dbPackage);
        }

    } // end of namespace graphics
} // end of namespace mars

DESTROY_LIB(mars::graphics::GraphicsManager);
CREATE_LIB(mars::graphics::GraphicsManager);
