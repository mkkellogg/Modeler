#include "OpenGLMouseAdapterWidget.h"

OpenGLMouseAdapterWidget::OpenGLMouseAdapterWidget(QWidget *parent): QOpenGLWidget(parent)
{

}

void OpenGLMouseAdapterWidget::setMouseAdapter(std::shared_ptr<MouseAdapter> adapter) {
    this->mouseAdapter = adapter;
}

void OpenGLMouseAdapterWidget::mousePressEvent(QMouseEvent *event)
{
    if(this->mouseAdapter) {;
        this->mouseAdapter->processEvent(this, event);
    }
}

void OpenGLMouseAdapterWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(this->mouseAdapter) {;
        this->mouseAdapter->processEvent(this, event);
    }
}


void OpenGLMouseAdapterWidget::mouseMoveEvent(QMouseEvent *event)
{
    if(this->mouseAdapter) {
        this->mouseAdapter->processEvent(this, event);
    }
}

void OpenGLMouseAdapterWidget::wheelEvent(QWheelEvent *event)
{
    if(this->mouseAdapter) {;
        this->mouseAdapter->processEvent(this, event);
    }
}


