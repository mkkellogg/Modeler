#include "RenderWindow.h"

#include <math.h>

#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <QTimer>

bool RenderWindow::m_transparent = false;

RenderWindow::RenderWindow(QWidget *parent): OpenGLMouseAdapterWidget(parent), m_xRot(0), m_yRot(0), m_zRot(0),
                           initialized(false), engineInitialized(false), engine(nullptr)
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

    QMutexLocker ml(&this->updateMutex);
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

void RenderWindow::onPreRender(LifeCycleEventCallback func, bool oneTime) {
    QMutexLocker ml(&this->onPreRenderMutex);
    if (oneTime) this->onSingleUpdates.push_back(func);
    else this->onPreRenders.push_back(func);
}

void RenderWindow::onUpdate(LifeCycleEventCallback func, bool oneTime) {
    QMutexLocker ml(&this->onUpdateMutex);
    if (oneTime) this->onSingleUpdates.push_back(func);
    else this->onUpdates.push_back(func);
}

QMutex& RenderWindow::getUpdateMutex() {
    return this->updateMutex;
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
    std::vector<LifeCycleEventCallback> arrays[] = {this->onSingleUpdates, this->onUpdates};
    for (unsigned int i = 0; i < 2; i++) {
        std::vector<LifeCycleEventCallback>& array = arrays[i];
        QMutexLocker ml(&this->onUpdateMutex);
        for(std::vector<LifeCycleEventCallback>::iterator itr = array.begin(); itr != array.end(); ++itr) {
            LifeCycleEventCallback func = *itr;
            func(this);
        }
    }
    this->onSingleUpdates.clear();
}

void RenderWindow::resolveOnPreRenders() {
    std::vector<LifeCycleEventCallback> arrays[] = {this->onSinglePreRenders, this->onPreRenders};
    for (unsigned int i = 0; i < 2; i++) {
        std::vector<LifeCycleEventCallback>& array = arrays[i];
        QMutexLocker ml(&this->onPreRenderMutex);
        for(std::vector<LifeCycleEventCallback>::iterator itr = array.begin(); itr != array.end(); ++itr) {
            LifeCycleEventCallback func = *itr;
            func(this);
        }
    }
    this->onSingleUpdates.clear();
}
