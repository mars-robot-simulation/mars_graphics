#include <cstring>
#include <string>
#include <osgDB/WriteFile>

#include "PostDrawCallback.h"


namespace mars
{
    namespace graphics
    {

        PostDrawCallback::PostDrawCallback(osg::Image* image)
        {
            _image = image;
            _grab = false;
            _save_grab = false;
            image_id = (unsigned long*)malloc(sizeof(unsigned long));
            *image_id = 1;
            fprintf(stderr, "initialized postDrawCallback\n");
            imageMutex = new pthread_mutex_t;
            pthread_mutex_init(imageMutex, NULL);
        }

        PostDrawCallback::~PostDrawCallback()
        {
            pthread_mutex_lock(imageMutex);
            delete image_id;
            delete imageMutex;
        }

        void PostDrawCallback::operator () (osg::RenderInfo& renderInfo) const
        {
            (void) renderInfo;
            if(_grab)
            {
                pthread_mutex_lock(imageMutex);
                if(_save_grab)
                    _image->readPixels(0, 0 , _width, _height, GL_RGBA,
                                       GL_UNSIGNED_BYTE);
                else
                    _image->readPixels(0, 0 , _width, _height, GL_BGRA,
                                       GL_UNSIGNED_BYTE);
                if(_save_grab)
                {
                    char c_filename[255];
                    sprintf(c_filename, "movie/pic%.6lu.png", *image_id);
                    std::string filename = c_filename;
                    osgDB::writeImageFile(*_image, filename);
                    *image_id += 1;
                }
                pthread_mutex_unlock(imageMutex);
            }
        }

        void PostDrawCallback::setSize(int width, int height)
        {
            pthread_mutex_lock(imageMutex);
            _width = width;
            _height = height;
            pthread_mutex_unlock(imageMutex);
        }

        void PostDrawCallback::setGrab(bool grab)
        {
            _grab = grab;
        }
        void PostDrawCallback::setSaveGrab(bool grab)
        {
            _save_grab = grab;
        }

        void PostDrawCallback::getImageData(void **data, int &width, int &height)
        {
            pthread_mutex_lock(imageMutex);
            if(_image->valid())
            {
                width = _image->s();
                height = _image->t();
                // allocating width*height*3byte
                *data = malloc(width*height*4);
                memcpy(*data, _image->data(), width*height*4);
            }
            pthread_mutex_unlock(imageMutex);
        }

    } // end of namespace graphics
} // end of namespace mars
