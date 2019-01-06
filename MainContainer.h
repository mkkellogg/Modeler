#pragma once

#include <functional>

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>

class QSlider;
class QPushButton;

class RenderWindow;
class MainWindow;

class MainContainer : public QWidget
{
    Q_OBJECT

public:
    using LoadModelClickedCallback = std::function<void(const std::string&, const std::string&, const std::string&, bool)>;

    MainContainer(MainWindow *mw);
    RenderWindow* getRenderWindow();

    void setModelScaleEditText(float scale);
    void setModelSmoothingThresholdEditText(float angle);
    void setModelZUpCheck(bool checked);

    void onLoadModelClicked(LoadModelClickedCallback callback);

protected slots:
    void browseForModel();
    void loadModel();

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
    QCheckBox* modelZUpCheckBox;

    LoadModelClickedCallback loadModelClickedCallback;
};
