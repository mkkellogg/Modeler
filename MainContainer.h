#pragma once

#include <functional>
#include <unordered_map>

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

private:
    class SceneTreeWidgetItem: public QTreeWidgetItem {
    public:
        Core::WeakPointer<Core::Object3D> sceneObject;
    };

    void setUpGUI();
    QFrame* buildLoadModelGUI();
    void selecScenetObject(Core::WeakPointer<Core::Object3D> object);
    void populateSceneTree(QTreeWidget* sceneObjectTree, QTreeWidgetItem* parentItem, Core::WeakPointer<Core::Object3D> parentObject);
    void refreshSceneTree();
    void expandAllAbove(SceneTreeWidgetItem* item);

    ModelerApp* app;
    RenderWindow *renderWindow;
    QPushButton *dockBtn;
    MainWindow *mainWindow;
    QTreeWidget* sceneObjectTree;
    QLineEdit* modelNameEdit;
    QLineEdit* modelScaleEdit;
    QLineEdit* modelSmoothingThresholdEdit;
    QCheckBox* modelZUpCheckBox;
    std::unordered_map<Core::UInt64, SceneTreeWidgetItem*> sceneObjectTreeMap;
};
