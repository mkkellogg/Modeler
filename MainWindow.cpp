#include "MainWindow.h"

#include <iostream>

#include <QMenuBar>
#include <QMenu>
#include <QMessageBox>

MainWindow::MainWindow(): mainGUI(nullptr) {
    setupMainGUI();
}

void MainWindow::setupMainGUI() {
    if (!centralWidget()) {
        this->mainGUI = new MainGUI(this);
        setCentralWidget(mainGUI);
    }
}

MainGUI* MainWindow::getMainGUI() {
    return this->mainGUI;
}
