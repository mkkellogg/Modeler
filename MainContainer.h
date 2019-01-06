#pragma once

#include <functional>

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>

class QSlider;
class QPushButton;

class RenderWindow;
class MainWindow;
class ModelerApp;

class MainContainer : public QWidget
{
    Q_OBJECT

public:
    using LoadModelClickedCallback = std::function<void(const std::string&, const std::string&, const std::string&, bool)>;

    MainContainer(MainWindow *mw);
    void setApp(ModelerApp* app);
    RenderWindow* getRenderWindow();

    void setModelScaleEditText(float scale);
    void setModelSmoothingThresholdEditText(float angle);
    void setModelZUpCheck(bool checked);

protected slots:
    void browseForModel();
    void loadModel();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QSlider *createSlider();

    ModelerApp* app;
    RenderWindow *renderWindow;
    QPushButton *dockBtn;
    MainWindow *mainWindow;

    QLineEdit* modelNameEdit;
    QLineEdit* modelScaleEdit;
    QLineEdit* modelSmoothingThresholdEdit;
    QCheckBox* modelZUpCheckBox;

};
