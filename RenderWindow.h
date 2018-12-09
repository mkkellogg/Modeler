#pragma once

#include <functional>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QMutex>

#include "OpenGLMouseAdapterWidget.h"

#include "Core/Engine.h"

QT_FORWARD_DECLARE_CLASS(QOpenGLShaderProgram)

using QOpenGLFunctionsBase = QOpenGLFunctions_3_3_Core;

class RenderWindow : public OpenGLMouseAdapterWidget, protected QOpenGLFunctionsBase
{
    Q_OBJECT

public:

    typedef std::function<void(RenderWindow*)> LifeCycleEventCallback;

    RenderWindow(QWidget *parent = 0);
    ~RenderWindow();

    static bool isTransparent() { return m_transparent; }
    static void setTransparent(bool t) { m_transparent = t; }

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    Core::WeakPointer<Core::Engine> getEngine();
    void onInit(LifeCycleEventCallback func);
    void onUpdate(LifeCycleEventCallback func);
    void onPreRender(LifeCycleEventCallback func);

public slots:
    void setXRotation(int angle);
    void setYRotation(int angle);
    void setZRotation(int angle);
    void cleanup();

signals:
    void xRotationChanged(int angle);
    void yRotationChanged(int angle);
    void zRotationChanged(int angle);

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

private:

    bool m_core;
    int m_xRot;
    int m_yRot;
    int m_zRot;
    QPoint m_lastPos;
    QOpenGLVertexArrayObject m_vao;
    QOpenGLBuffer m_logoVbo;
    QOpenGLShaderProgram *m_program;
    int m_projMatrixLoc;
    int m_mvMatrixLoc;
    int m_normalMatrixLoc;
    int m_lightPosLoc;
    QMatrix4x4 m_proj;
    QMatrix4x4 m_camera;
    QMatrix4x4 m_world;
    static bool m_transparent;



    void init();
    void update();
    void render();
    void resolveOnInits();
    void resolveOnInit(LifeCycleEventCallback callback);
    void resolveOnUpdates();
    void resolveOnPreRenders();

    QMutex preRenderMutex;
    QMutex updateMutex;
    bool initialized;
    bool engineInitialized;
    Core::PersistentWeakPointer<Core::Engine> engine;
    std::vector<LifeCycleEventCallback> onInits;
    std::vector<LifeCycleEventCallback> onUpdates;
    std::vector<LifeCycleEventCallback> onPreRenders;
};
