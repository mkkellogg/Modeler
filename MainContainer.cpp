#include "RenderWindow.h"
#include "MainContainer.h"
#include "ManWindow.h"
#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QDesktopWidget>
#include <QApplication>
#include <QMessageBox>

MainContainer::MainContainer(MainWindow *mw): mainWindow(mw) {
    this->renderWindow = new RenderWindow;

    xSlider = createSlider();
    ySlider = createSlider();
    zSlider = createSlider();

    connect(xSlider, &QSlider::valueChanged, this->renderWindow, &RenderWindow::setXRotation);
    connect(this->renderWindow, &RenderWindow::xRotationChanged, xSlider, &QSlider::setValue);
    connect(ySlider, &QSlider::valueChanged, this->renderWindow, &RenderWindow::setYRotation);
    connect(this->renderWindow, &RenderWindow::yRotationChanged, ySlider, &QSlider::setValue);
    connect(zSlider, &QSlider::valueChanged, this->renderWindow, &RenderWindow::setZRotation);
    connect(this->renderWindow, &RenderWindow::zRotationChanged, zSlider, &QSlider::setValue);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    QHBoxLayout *container = new QHBoxLayout;
    container->addWidget(this->renderWindow);
    container->addWidget(xSlider);
    container->addWidget(ySlider);
    container->addWidget(zSlider);

    QWidget *w = new QWidget;
    w->setLayout(container);
    mainLayout->addWidget(w);
    setLayout(mainLayout);

    xSlider->setValue(15 * 16);
    ySlider->setValue(345 * 16);
    zSlider->setValue(0 * 16);

    setWindowTitle(tr("Hello GL"));

    this->setAutoFillBackground(false);
}

RenderWindow* MainContainer::getRenderWindow() {
    return this->renderWindow;
}

QSlider *MainContainer::createSlider() {
    QSlider *slider = new QSlider(Qt::Vertical);
    slider->setRange(0, 360 * 16);
    slider->setSingleStep(16);
    slider->setPageStep(15 * 16);
    slider->setTickInterval(15 * 16);
    slider->setTickPosition(QSlider::TicksRight);
    return slider;
}

void MainContainer::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(e);
}
