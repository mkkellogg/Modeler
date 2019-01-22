#pragma once

#include <QMainWindow>
#include "MainGUI.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    MainGUI* getMainGUI();
private:
    void setupMainGUI();

    MainGUI * mainGUI;
};
