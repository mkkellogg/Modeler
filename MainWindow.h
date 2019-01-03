#pragma once

#include <QMainWindow>
#include "MainContainer.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    MainContainer* getMainContainer();
private:
    void setupMainContainer();

    MainContainer * mainContainer;
};
