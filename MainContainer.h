#pragma once

#include <QWidget>
#include <QLineEdit>

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

protected slots:
    void browseForModel();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QSlider *createSlider();

    RenderWindow *renderWindow;
    QPushButton *dockBtn;
    MainWindow *mainWindow;

    QLineEdit* modelNameEdit;
    QLineEdit* modelScaleEdit;
    QLineEdit* modelSmoothingThresholdEdit;
};
