#include <osg/Geometry>

#include "PlaneDrawObject.h"

namespace mars
{
    namespace graphics
    {

        PlaneDrawObject::PlaneDrawObject(GraphicsManager *g,
                                         const mars::utils::Vector &ext)
            : DrawObject(g), extent_(ext)
        {
            geometrySize_.x() = extent_.x();
            geometrySize_.y() = extent_.y();
        }

        std::list< osg::ref_ptr< osg::Geode > > PlaneDrawObject::createGeometry()
        {
            osg::ref_ptr<osg::Vec3Array> vertices(new osg::Vec3Array());
            osg::ref_ptr<osg::Vec2Array> texcoords(new osg::Vec2Array());
            osg::ref_ptr<osg::Vec3Array> normals(new osg::Vec3Array());
            osg::Geometry *geom = new osg::Geometry();
            std::list< osg::ref_ptr< osg::Geode > > geodes;

            { // NOTE: we have to bind the normals per vertex for the stupid tangent generator of osg :/
                float x1 = extent_.x()/2.0, x2 = -x1;
                float y1 = extent_.y()/2.0, y2 = -x1;
                /*
                  vertices->push_back(osg::Vec3(x2, y1, 0.0f));
                  texcoords->push_back(osg::Vec2(0.0f, extent_.z()));
                  normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
                  vertices->push_back(osg::Vec3(x1, y1, 0.0f));
                  texcoords->push_back(osg::Vec2(extent_.z(), extent_.z()));
                  normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
                  vertices->push_back(osg::Vec3(x1, y2, 0.0f));
                  texcoords->push_back(osg::Vec2(extent_.z(), 0.0f));
                  normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
                  vertices->push_back(osg::Vec3(x2, y2, 0.0f));
                  texcoords->push_back(osg::Vec2(0.0f, 0.0f));
                  normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
                */
                vertices->push_back(osg::Vec3(x2, y2, 0.0f));
                texcoords->push_back(osg::Vec2(x2, y2));
                normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
                vertices->push_back(osg::Vec3(x1, y2, 0.0f));
                texcoords->push_back(osg::Vec2(x1, y2));
                normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
                vertices->push_back(osg::Vec3(x1, y1, 0.0f));
                texcoords->push_back(osg::Vec2(x1, y1));
                normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
                vertices->push_back(osg::Vec3(x2, y1, 0.0f));
                texcoords->push_back(osg::Vec2(x2, y1));
                normals->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));

            }

            geom->setVertexArray(vertices.get());
            geom->setTexCoordArray(DEFAULT_UV_UNIT,texcoords.get());
            geom->setNormalArray(normals.get());
            geom->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);

            // TODO: use addPrimitiveSet(DrawElementsUInt)
            geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,
                                                      0, // index of first vertex
                                                      vertices->size()));

            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            geode->addDrawable(geom);
            geodes.push_back(geode);

            return geodes;
        }

        void PlaneDrawObject::setScaledSize(const mars::utils::Vector &scaledSize)
        {
            setScale(mars::utils::Vector(scaledSize.x() / geometrySize_.x(),
                                         scaledSize.y() / geometrySize_.y(),
                                         1.0));
        }

    } // end of namespace graphics
} // end of namespace mars
