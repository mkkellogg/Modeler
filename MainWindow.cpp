#include "MainWindow.h"

#include <iostream>

#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>

MainWindow::MainWindow(): mainContainer(nullptr) {
    setupMainContainer();
}

void MainWindow::setupMainContainer() {
    if (!centralWidget()) {
        this->mainContainer = new MainContainer(this);
        setCentralWidget(mainContainer);
    }
}

MainContainer* MainWindow::getMainContainer() {
    return this->mainContainer;
}
