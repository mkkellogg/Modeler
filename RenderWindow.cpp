#include "RenderWindow.h"

#include <math.h>

#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <QTimer>

bool RenderWindow::m_transparent = false;

RenderWindow::RenderWindow(QWidget *parent): OpenGLMouseAdapterWidget(parent), m_xRot(0), m_yRot(0), m_zRot(0),
                         m_program(0), initialized(false), engineInitialized(false), engine(nullptr)
{
    m_core = QSurfaceFormat::defaultFormat().profile() == QSurfaceFormat::CoreProfile;
    // --transparent causes the clear color to be transparent. Therefore, on systems that
    // support it, the widget will become transparent apart from the logo.
    if (m_transparent) {
        QSurfaceFormat fmt = format();
        fmt.setAlphaBufferSize(8);
        setFormat(fmt);
    }
}

RenderWindow::~RenderWindow()
{
    cleanup();
}

Core::WeakPointer<Core::Engine> RenderWindow::getEngine() {
    return engine;
}

QSize RenderWindow::minimumSizeHint() const
{
    return QSize(50, 50);
}

QSize RenderWindow::sizeHint() const
{
    return QSize(768, 768);
}

void RenderWindow::cleanup()
{

}

void RenderWindow::initializeGL()
{
    // In this example the widget's corresponding top-level window can change
    // several times during the widget's lifetime. Whenever this happens, the
    // QOpenGLWidget's associated context is destroyed and a new one is created.
    // Therefore we have to be prepared to clean up the resources on the
    // aboutToBeDestroyed() signal, instead of the destructor. The emission of
    // the signal will be followed by an invocation of initializeGL() where we
    // can recreate all resources.
    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &RenderWindow::cleanup);

    // Create a vertex array object. In OpenGL ES 2.0 and OpenGL 2.x
    // implementations this is optional and support may not be present
    // at all. Nonetheless the below code works in all cases and makes
    // sure there is a VAO when one is needed.
    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    this->init();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(16);
}

void RenderWindow::paintGL()
{
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    static bool _inited = false;
    if (!_inited) {
        this->engine->setDefaultRenderTargetToCurrent();
        _inited = true;
    }
    this->update();
    this->resolveOnUpdates();
    this->resolveOnPreRenders();
    this->render();

}

void RenderWindow::resizeGL(int w, int h) {
    engine->setRenderSize(w, h, true);
}

void RenderWindow::init() {
  if (!engineInitialized) {
    engine = Core::Engine::instance();
    engineInitialized = true;
  }
  resolveOnInits();
}

void RenderWindow::update() {
  engine->update();
}

void RenderWindow::render() {
  engine->render();
}

void RenderWindow::onInit(LifeCycleEventCallback func) {
    if (engineInitialized) {
        resolveOnInit(func);
    }
    else {
        onInits.push_back(func);
    }
}

void RenderWindow::onPreRender(LifeCycleEventCallback func) {
    QMutexLocker ml(&this->preRenderMutex);
    onPreRenders.push_back(func);
}

void RenderWindow::onUpdate(LifeCycleEventCallback func) {
    QMutexLocker ml(&this->updateMutex);
    onUpdates.push_back(func);
}

void RenderWindow::resolveOnInits() {
    for(std::vector<LifeCycleEventCallback>::iterator itr = onInits.begin(); itr != onInits.end(); ++itr) {
        LifeCycleEventCallback func = *itr;
        resolveOnInit(func);
    }
}

void RenderWindow::resolveOnInit(LifeCycleEventCallback callback) {
    callback(this);
}

void RenderWindow::resolveOnUpdates() {
    if (onUpdates.size() > 0) {
        QMutexLocker ml(&this->updateMutex);
        for(std::vector<LifeCycleEventCallback>::iterator itr = onUpdates.begin(); itr != onUpdates.end(); ++itr) {
            LifeCycleEventCallback func = *itr;
            func(this);
        }
        onUpdates.clear();
    }
}

void RenderWindow::resolveOnPreRenders() {
    if (onPreRenders.size() > 0) {
        QMutexLocker ml(&this->preRenderMutex);
        for(std::vector<LifeCycleEventCallback>::iterator itr = onPreRenders.begin(); itr != onPreRenders.end(); ++itr) {
            LifeCycleEventCallback func = *itr;
            func(this);
        }
        onPreRenders.clear();
    }
}
