#pragma once

#include "GraphicsWidget.h"

#include <mars_interfaces/MARSDefs.h>
#include <mars_utils/Vector.h>
#include <mars_utils/Color.h>
#include <mars_utils/mathUtils.h>
#include <mars_interfaces/graphics/draw_structs.h>

#include <string>
#include <vector>
#include <osg/Group>

namespace mars
{
    namespace graphics
    {

        /**
         * Wraps draw_item in osg::Group.
         */
        class OSGDrawItem : public osg::Group
        {
        public:
            /**
             * Constructor, does not create geometry.
             */
            OSGDrawItem(GraphicsWidget *gw);
            /**
             * Constructor, creates geometry from draw_item instance.
             */
            OSGDrawItem(GraphicsWidget *gw, const interfaces::draw_item &di,
                        std::string font_path);

            /**
             * update this OSGDrawItem with given draw_item instance.
             */
            void update(interfaces::draw_item &di);

            /////// create and update functions for draw items

            static void createLabel(OSGDrawItem *di,
                                    mars::utils::Vector pos,
                                    mars::interfaces::sReal size,
                                    std::string label, std::string fontPath,
                                    double r = 1.0, double g = 1.0, double b = 1.0,
                                    double a = 1.0, int align_to_view = 1);

            static void updateLabel(OSGDrawItem *di,
                                    mars::utils::Vector pos,
                                    mars::interfaces::sReal size,
                                    std::string label,
                                    double r, double g, double b, double a,
                                    int align_to_view);

            static void createLine(OSGDrawItem *di,
                                   mars::utils::Vector start,
                                   mars::utils::Vector end,
                                   mars::utils::Color myColor);

            static void createLines(OSGDrawItem *osgNode,
                                    std::vector<float> _vertices,
                                    mars::utils::Color myColor);

            static void createArrow(OSGDrawItem *di,
                                    mars::utils::Vector start,
                                    mars::utils::Vector end,
                                    mars::utils::Color myColor);

            static void createRectangle(OSGDrawItem *di,
                                        mars::utils::Vector a, mars::utils::Vector b,
                                        mars::utils::Vector q,
                                        mars::utils::Color myColor,
                                        mars::utils::Color borderColor,
                                        int align_to_view = 0);

            static void createCircle(OSGDrawItem *di,
                                     mars::utils::Vector a, mars::utils::Vector b,
                                     mars::utils::Vector q,
                                     mars::utils::Color myColor,
                                     mars::utils::Color borderColor,
                                     int resolution);

            static void createHUDRectangle(OSGDrawItem *di,
                                           mars::interfaces::sReal height,
                                           mars::interfaces::sReal width,
                                           mars::utils::Vector myoff,
                                           mars::utils::Color myColor);
            static void updateHUDRectangle(OSGDrawItem *di,
                                           mars::interfaces::sReal height,
                                           mars::interfaces::sReal width,
                                           mars::utils::Vector myoff,
                                           mars::utils::Color myColor);

            static void createPoints(OSGDrawItem *di,
                                     std::vector<float> vertices,
                                     mars::utils::Color myColor);
            static void updatePoints(OSGDrawItem *di, std::vector<float> vertices);

            static void createTriangleSet(OSGDrawItem *di,
                                          std::vector<float> vertices,
                                          mars::utils::Color myColor);
            static void updateTriangleSet(OSGDrawItem *di,
                                          std::vector<float> vertices);
            static void addToTriangleSet(OSGDrawItem *di, std::vector<float> vertices,
                                         mars::utils::Color myColor);

            static void createPlane(OSGDrawItem *di,
                                    double w, double l, double h, double res);

            static void createSphere(OSGDrawItem *di, double rad);

            GraphicsWidget* gw()
                {
                    return gw_;
                }

        private:
            interfaces::DrawType type_;
            GraphicsWidget *gw_;
            osg::ref_ptr<osg::Texture2D> texture_;
        }; // end of class OSGDrawItem

    } // end of namespace graphics
} // end of namespace mars
