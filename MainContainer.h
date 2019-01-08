#pragma once

#include <functional>

#include <QWidget>
#include <QLineEdit>
#include <QCheckBox>
#include <QTreeWidget>
#include <QItemSelection>

#include "Core/Engine.h"

class QSlider;
class QPushButton;
class QTreeWidgetItem;
class QTreeWidget;

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
    void sceneTreeSelectionChanged(const QItemSelection& oldItem,const QItemSelection& newItem);
    void browseForModel();
    void loadModel();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    void setUpGUI();
    QFrame* buildLoadModelGUI();
    void populateSceneTree(QTreeWidget* sceneTree, QTreeWidgetItem* parentItem, Core::WeakPointer<Core::Object3D> parentObject);
    void refreshSceneTree();

    ModelerApp* app;
    RenderWindow *renderWindow;
    QPushButton *dockBtn;
    MainWindow *mainWindow;
    QTreeWidget* sceneTree;
    QLineEdit* modelNameEdit;
    QLineEdit* modelScaleEdit;
    QLineEdit* modelSmoothingThresholdEdit;
    QCheckBox* modelZUpCheckBox;

    class SceneTreeWidgetItem: public QTreeWidgetItem {
    public:
        Core::WeakPointer<Core::Object3D> sceneObject;
    };

};
