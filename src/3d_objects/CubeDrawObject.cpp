#include <osg/Geometry>
#include "CubeDrawObject.h"

namespace mars
{
    namespace graphics
    {
    
        using mars::utils::Vector;

        osg::ref_ptr<osg::Geode> CubeDrawObject::sharedCube = nullptr;

        CubeDrawObject::CubeDrawObject(GraphicsManager *g)
            : DrawObject(g)
        {
        }

        std::list< osg::ref_ptr< osg::Geode > > CubeDrawObject::createGeometry()
        {
            std::list< osg::ref_ptr< osg::Geode > > geodes;

            if(!sharedCube.valid())
            {
                osg::ref_ptr<osg::Vec3Array> vertices(new osg::Vec3Array());
                osg::ref_ptr<osg::Vec2Array> texcoords(new osg::Vec2Array());
                osg::ref_ptr<osg::Vec3Array> normals(new osg::Vec3Array());
                osg::Geometry *geom = new osg::Geometry();
                float x2=.5, y2=.5, z2=.5;
                // Top Face
                texcoords->push_back(osg::Vec2(0.0f, 0.0f));
                vertices->push_back(osg::Vec3(-x2, -y2,  z2));
                normals->push_back(osg::Vec3(0.0, 0.0, 1.0));
                texcoords->push_back(osg::Vec2(1.0f, 0.0f));
                vertices->push_back(osg::Vec3( x2, -y2,  z2));
                normals->push_back(osg::Vec3(0.0, 0.0, 1.0));
                texcoords->push_back(osg::Vec2(1.0f, 1.0f));
                vertices->push_back(osg::Vec3( x2,  y2,  z2));
                normals->push_back(osg::Vec3(0.0, 0.0, 1.0));
                texcoords->push_back(osg::Vec2(0.0f, 1.0f));
                vertices->push_back(osg::Vec3(-x2,  y2,  z2));
                normals->push_back(osg::Vec3(0.0, 0.0, 1.0));
                // Bottom Face
                texcoords->push_back(osg::Vec2(1.0f, 0.0f));
                vertices->push_back(osg::Vec3(-x2, -y2, -z2));
                normals->push_back(osg::Vec3(0.0, 0.0, -1.0));
                texcoords->push_back(osg::Vec2(1.0f, 1.0f));
                vertices->push_back(osg::Vec3(-x2,  y2, -z2));
                normals->push_back(osg::Vec3(0.0, 0.0, -1.0));
                texcoords->push_back(osg::Vec2(0.0f, 1.0f));
                vertices->push_back(osg::Vec3( x2,  y2, -z2));
                normals->push_back(osg::Vec3(0.0, 0.0, -1.0));
                texcoords->push_back(osg::Vec2(0.0f, 0.0f));
                vertices->push_back(osg::Vec3( x2, -y2, -z2));
                normals->push_back(osg::Vec3(0.0, 0.0, -1.0));
                // Back Face
                texcoords->push_back(osg::Vec2(0.0f, 1.0f));
                vertices->push_back(osg::Vec3(-x2,  y2, -z2));
                normals->push_back(osg::Vec3(0.0, 1.0, 0.0));
                texcoords->push_back(osg::Vec2(0.0f, 0.0f));
                vertices->push_back(osg::Vec3(-x2,  y2,  z2));
                normals->push_back(osg::Vec3(0.0, 1.0, 0.0));
                texcoords->push_back(osg::Vec2(1.0f, 0.0f));
                vertices->push_back(osg::Vec3( x2,  y2,  z2));
                normals->push_back(osg::Vec3(0.0, 1.0, 0.0));
                texcoords->push_back(osg::Vec2(1.0f, 1.0f));
                vertices->push_back(osg::Vec3( x2,  y2, -z2));
                normals->push_back(osg::Vec3(0.0, 1.0, 0.0));
                // Front Face
                texcoords->push_back(osg::Vec2(1.0f, 1.0f));
                vertices->push_back(osg::Vec3(-x2, -y2, -z2));
                normals->push_back(osg::Vec3(0.0, -1.0, 0.0));
                texcoords->push_back(osg::Vec2(0.0f, 1.0f));
                vertices->push_back(osg::Vec3( x2, -y2, -z2));
                normals->push_back(osg::Vec3(0.0, -1.0, 0.0));
                texcoords->push_back(osg::Vec2(0.0f, 0.0f));
                vertices->push_back(osg::Vec3( x2, -y2,  z2));
                normals->push_back(osg::Vec3(0.0, -1.0, 0.0));
                texcoords->push_back(osg::Vec2(1.0f, 0.0f));
                vertices->push_back(osg::Vec3(-x2, -y2,  z2));
                normals->push_back(osg::Vec3(0.0, -1.0, 0.0));
                // Right face
                texcoords->push_back(osg::Vec2(1.0f, 0.0f));
                vertices->push_back(osg::Vec3( x2, -y2, -z2));
                normals->push_back(osg::Vec3(1.0, 0.0, 0.0));
                texcoords->push_back(osg::Vec2(1.0f, 1.0f));
                vertices->push_back(osg::Vec3( x2,  y2, -z2));
                normals->push_back(osg::Vec3(1.0, 0.0, 0.0));
                texcoords->push_back(osg::Vec2(0.0f, 1.0f));
                vertices->push_back(osg::Vec3( x2,  y2,  z2));
                normals->push_back(osg::Vec3(1.0, 0.0, 0.0));
                texcoords->push_back(osg::Vec2(0.0f, 0.0f));
                vertices->push_back(osg::Vec3( x2, -y2,  z2));
                normals->push_back(osg::Vec3(1.0, 0.0, 0.0));
                // Left Face
                texcoords->push_back(osg::Vec2(0.0f, 0.0f));
                vertices->push_back(osg::Vec3(-x2, -y2, -z2));
                normals->push_back(osg::Vec3(-1.0, 0.0, 0.0));
                texcoords->push_back(osg::Vec2(1.0f, 0.0f));
                vertices->push_back(osg::Vec3(-x2, -y2,  z2));
                normals->push_back(osg::Vec3(-1.0, 0.0, 0.0));
                texcoords->push_back(osg::Vec2(1.0f, 1.0f));
                vertices->push_back(osg::Vec3(-x2,  y2,  z2));
                normals->push_back(osg::Vec3(-1.0, 0.0, 0.0));
                texcoords->push_back(osg::Vec2(0.0f, 1.0f));
                vertices->push_back(osg::Vec3(-x2,  y2, -z2));
                normals->push_back(osg::Vec3(-1.0, 0.0, 0.0));
                geom->setVertexArray(vertices.get());
                geom->setTexCoordArray(DEFAULT_UV_UNIT,texcoords.get());
                geom->setNormalArray(normals.get());
                geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

                geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,
                                                          0, // index of first vertex
                                                          vertices->size()));
                geom->setUseDisplayList(false);
                geom->setUseVertexBufferObjects(true);
                sharedCube = new osg::Geode();
                sharedCube->addDrawable(geom);
            }
            geodes.push_back(sharedCube.get());
            return geodes;
        }

    } // end of namespace graphics
} // end of namespace mars
