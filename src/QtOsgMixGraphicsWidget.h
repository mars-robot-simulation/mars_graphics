#pragma once

#include "GraphicsWidget.h"

#include <QWidget>

namespace mars
{
    namespace graphics
    {

        class GraphicsManager;

        /**
         * A GraphicsWidget using QWidget
         */
        class QtOsgMixGraphicsWidget: public QWidget, public GraphicsWidget
        {
            Q_OBJECT; // with this we need a generated moc file
        public:
            QtOsgMixGraphicsWidget(void *parent, osg::Group *scene,
                                   unsigned long id, bool rtt_widget = 0,
                                   GraphicsManager *gm = 0)
                : QWidget((QWidget*) parent),
                  GraphicsWidget(parent, scene, id, rtt_widget, gm), childWidget(NULL) {}
            // Prevent flicker on Windows Qt
            QPaintEngine* paintEngine () const { return 0; }

            virtual void initialize();
            virtual osg::ref_ptr<osg::GraphicsContext> createWidgetContext(void *parent, osg::ref_ptr<osg::GraphicsContext::Traits> traits);

            virtual void setWGeometry(int top, int left, int width, int height);
            virtual void getWGeometry(int *top, int *left, int *width, int *height) const;
            virtual void setWidgetFullscreen(bool val);

            virtual void* getWidget();
            virtual void showWidget();

            virtual void updateView();

            virtual void hideEvent(QHideEvent *event);
            virtual void closeEvent(QCloseEvent *event);
            virtual void showEvent(QShowEvent *event);
            virtual void mousePressEvent(QMouseEvent *e);
            virtual bool eventFilter(QObject *obj, QEvent *event);
            virtual void focusInEvent(QFocusEvent *event);

            static QtOsgMixGraphicsWidget* createInstance(void *parent,
                                                          osg::Group *scene,
                                                          unsigned long id,
                                                          bool rtt_widget = 0,
                                                          GraphicsManager *gm = 0);

        protected:
            virtual ~QtOsgMixGraphicsWidget() {fprintf(stderr, "destructor\n");};

        private:
            static QtOsgMixGraphicsWidget *activeWindow, *eventInWindow;
            QWidget *childWidget;
            QObject *eventChild;
            double retinaScale;
        }; // end of class QtOsgMixGraphicsWidget

    } // end of namespace graphics
} // end of namespace mars

