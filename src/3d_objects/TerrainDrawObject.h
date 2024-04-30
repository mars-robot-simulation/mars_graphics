#pragma once

//#include "DrawObject.h"
#include "LoadDrawObject.h"

#include <mars_interfaces/MARSDefs.h>
#include <mars_utils/Vector.h>
#include <mars_interfaces/terrainStruct.h>

#include <configmaps/ConfigMap.hpp>

#include <string>
#include <vector>
#include <list>
#include <map>

namespace mars
{
    namespace graphics
    {

#ifdef USE_VERTEX_BUFFER
        class VertexBufferTerrain;
#endif

        struct TerrainDrawObject2Info
        {
            // texture scaling - a value of 0 will
            // set the texture coordinates to fit the terrain
            double texScale;
            std::string fileName;
            std::string objectName;
        }; // end of struct TerrainDrawObject2Info

        struct SubTile
        {
            double xPos, yPos;
            double **heightData;
            double xRes, yRes;
            double xSize, ySize;
            int xCount, yCount;
            double vStart;
            double vCount;
            int where;
            int frame;
            int mapIndex;
            int x, y;
            osg::ref_ptr<osg::DrawElementsUInt> pSet;
            osg::ref_ptr<osg::Vec3Array> vertices;
            osg::ref_ptr<osg::Vec4Array> tangents;
            osg::ref_ptr<osg::Vec2Array> texcoords;
            osg::ref_ptr<osg::Vec3Array> normals;
            osg::ref_ptr<osg::Vec3Array> normal_debug;
            osg::ref_ptr<osg::Geometry> normalGeom;
            osg::ref_ptr<osg::Geometry> geom;
            osg::ref_ptr<osg::Geode> geode;
        }; // end of struct SubTile

        class TerrainDrawObject : public DrawObject
        {

        public:
            TerrainDrawObject(GraphicsManager *g,
                              const mars::interfaces::terrainStruct *ts,
                              std::string gridFile = "");
            virtual ~TerrainDrawObject(void);
            virtual void generateTangents();
            virtual void collideSphere(mars::utils::Vector pos,
                                       mars::interfaces::sReal radius);
            static int countSubTiles;

#ifdef USE_VERTEX_BUFFER
            virtual void setSelected(bool val);
#endif
            void createObject(unsigned long id, const utils::Vector &pivot,
                              unsigned long sharedID);
            void setData(configmaps::ConfigMap map);

        private:

#ifdef USE_VERTEX_BUFFER
            osg::ref_ptr<VertexBufferTerrain> vbt;
#endif

            mars::interfaces::terrainStruct info;
            osg::ref_ptr<osg::Vec4Array> tangents;
            osg::ref_ptr<osg::Geometry> geom;

            osg::ref_ptr<osg::Vec3Array> vertices;
            osg::ref_ptr<osg::Vec2Array> texcoords;
            osg::ref_ptr<osg::Vec3Array> normals;
            osg::ref_ptr<osg::Vec3Array> normal_debug;
            osg::ref_ptr<osg::Geometry> normal_geom;

            double **height_data;

            int tangentUnit;
            int num_y, num_x;
            double x_step, y_step;
            double x_step2, y_step2;
            double tex_scale_x, tex_scale_y;
            std::string gridFile;
            std::vector<std::vector<LoadDrawObjectPSetBox*>*> gridPSets;
            virtual std::list< osg::ref_ptr< osg::Geode > > createGeometry();

            std::map<int, SubTile*> subTiles;
            std::vector<SubTile*> vSubTiles;
            configmaps::ConfigMap map;

            void createNewSubTile(SubTile *newSubTile, mars::utils::Vector pos,
                                  double radius);
            mars::utils::Vector getNormal(int x, int y, int mx, int my,
                                          double x_step, double y_step,
                                          double **height_data, osg::Vec3d* t,
                                          bool skipBorder = false);

            int rectangleIntersect(const osg::Vec3& first_leftup,
                                   const osg::Vec3& first_downright,
                                   const osg::Vec3& second_leftup,
                                   const osg::Vec3& second_downright);
            osg::Vec3d intersectReplacementSphere(osg::Vec3 center, double radius,
                                                  osg::Vec3 vertex);
            void cutHole(int x1, int x2, int y1, int y2);
            bool adaptSubtile(SubTile* tile, mars::utils::Vector pos, double radius);
            void removeSubTile(SubTile *tile);
            void updateShadowObject(mars::utils::Vector pos);
            void drawSubTile(SubTile *tile);
            double getHeight(int x, int y, SubTile *tile);

        }; // end of class TerrainDrawObject

    } // end of namespace graphics
} // end of namespace mars

