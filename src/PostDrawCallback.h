#pragma once
#include <osgViewer/Viewer>

#include <pthread.h>


namespace mars
{
    namespace graphics
    {

        class PostDrawCallback : public osg::Camera::Camera::DrawCallback
        {
        public:
            PostDrawCallback(osg::Image* image);

            ~PostDrawCallback();

            virtual void operator () (osg::RenderInfo& renderInfo) const;

            void setSize(int width, int height);

            void setGrab(bool grab);
            void setSaveGrab(bool grab);

            void getImageData(void **data, int &width, int &height);

        private:
            osg::Image* _image;
            int _width;
            int _height;
            bool _grab, _save_grab;
            unsigned long *image_id;
            pthread_mutex_t *imageMutex;
        };

    } // end of namespace graphics
} // end of namespace mars
