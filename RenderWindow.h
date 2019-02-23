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
    QMutex& getUpdateMutex();

public slots:
    void cleanup();

signals:

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int width, int height) override;

private:

    bool m_core;
    QOpenGLVertexArrayObject m_vao;
    static bool m_transparent;

    void init();
    void update();
    void render();
    void resolveOnInits();
    void resolveOnInit(LifeCycleEventCallback callback);

    QMutex onPreRenderMutex;
    QMutex onUpdateMutex;
    QMutex updateMutex;
    bool initialized;
    bool engineInitialized;
    Core::PersistentWeakPointer<Core::Engine> engine;
    std::vector<LifeCycleEventCallback> onInits;
};
