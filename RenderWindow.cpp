#include "RenderWindow.h"

#include <math.h>
#include <QTimer>
#include <QMouseEvent>
#include <QOpenGLShaderProgram>
#include <QCoreApplication>
#include <QApplication>
#include <QDesktopWidget>

bool RenderWindow::m_transparent = false;

RenderWindow::RenderWindow(QWidget *parent): OpenGLMouseAdapterWidget(parent),
                           initialized(false), engineInitialized(false), engine(nullptr)
{
    m_core = QSurfaceFormat::defaultFormat().profile() == QSurfaceFormat::CoreProfile;
    // --transparent causes the clear color to be transparent. Therefore, on systems that
    // support it, the widget will become transparent apart from the logo.
   /* if (m_transparent) {
        QSurfaceFormat fmt = format();
        fmt.setAlphaBufferSize(8);
        setFormat(fmt);
    }
    */
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
    return QSize(1024, 768);
}

void RenderWindow::cleanup()
{

}

void RenderWindow::start() {
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(mainLoop()));
    timer->start(10);
}

void RenderWindow::mainLoop() {
    this->update();
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
    this->initializeOpenGLFunctions();

    // Create a vertex array object. In OpenGL ES 2.0 and OpenGL 2.x
    // implementations this is optional and support may not be present
    // at all. Nonetheless the below code works in all cases and makes
    // sure there is a VAO when one is needed.
    m_vao.create();
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);

    this->init();
    this->start();
}

void RenderWindow::paintGL()
{
    QMutexLocker ml(&this->updateMutex);
    QOpenGLVertexArrayObject::Binder vaoBinder(&m_vao);
    static bool _inited = false;
    if (!_inited) {
        this->engine->setDefaultRenderTargetToCurrent();
        _inited = true;
    }

    this->engineUpdate();
    this->engineRender();

}

void RenderWindow::resizeGL(int w, int h) {
    float dpr = QApplication::desktop()->devicePixelRatio();
    Core::UInt32 scaledW = static_cast<unsigned int>(static_cast<float>(w) * dpr);
    Core::UInt32 scaledH = static_cast<unsigned int>(static_cast<float>(h) * dpr);
    engine->setRenderSize(scaledW, scaledH, true);
}

void RenderWindow::init() {
  if (!engineInitialized) {
    engine = Core::Engine::instance();
    engineInitialized = true;
  }
  resolveOnInits();
}

void RenderWindow::engineUpdate() {
  engine->update();
}

void RenderWindow::engineRender() {
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

void RenderWindow::resolveOnInits() {
    for(std::vector<LifeCycleEventCallback>::iterator itr = onInits.begin(); itr != onInits.end(); ++itr) {
        LifeCycleEventCallback func = *itr;
        resolveOnInit(func);
    }
}

void RenderWindow::resolveOnInit(LifeCycleEventCallback callback) {
    callback(this);
}
