#pragma once

#include <QWidget>

class QSlider;
class QPushButton;

class RenderWindow;
class MainWindow;

class MainContainer : public QWidget
{
    Q_OBJECT

public:
    MainContainer(MainWindow *mw);
    RenderWindow* getRenderWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QSlider *createSlider();

    RenderWindow *renderWindow;
    QSlider *xSlider;
    QSlider *ySlider;
    QSlider *zSlider;
    QPushButton *dockBtn;
    MainWindow *mainWindow;
};
