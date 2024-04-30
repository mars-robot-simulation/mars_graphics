#pragma once

#include <osgViewer/CompositeViewer>
#include <mars_interfaces/graphics/GuiEventInterface.h>

namespace mars
{
    namespace graphics
    {

        class GraphicsViewer : public osgViewer::CompositeViewer
        {

        public:
            GraphicsViewer(interfaces::GuiEventInterface *_guiEventHandler);
            ~GraphicsViewer(void);

            void eventTraversal();

        private:
            interfaces::GuiEventInterface *guiEventHandler;
        }; // end of class GraphicsViewer

    } // end of namespace graphics
} // end of namespace mars

