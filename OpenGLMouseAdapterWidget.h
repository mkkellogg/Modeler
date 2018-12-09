#pragma once

#include <memory>

#include <QOpenGLWidget>

#include "MouseAdapter.h"

class OpenGLMouseAdapterWidget : public QOpenGLWidget
{
public:
    OpenGLMouseAdapterWidget(QWidget *parent);
    void setMouseAdapter(std::shared_ptr<MouseAdapter> adapter);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

    std::shared_ptr<MouseAdapter> mouseAdapter;
};
