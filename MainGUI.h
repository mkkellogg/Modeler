#pragma once

#include <functional>
#include <unordered_map>

#include <QTreeWidgetItem>

#include "SceneTreeWidget.h"

#include "Core/Engine.h"

class QSlider;
class QPushButton;
class QTreeWidgetItem;
class QTreeWidget;
class QVBoxLayout;
class QGroupBox;
class QItemSelection;
class QCheckBox;

class RenderWindow;
class MainWindow;
class ModelerApp;

class MainGUI : public QWidget
{
    Q_OBJECT

public:
    using LoadModelClickedCallback = std::function<void(const std::string&, const std::string&, const std::string&, bool)>;

    MainGUI(MainWindow *mw);
    void setModelerApp(ModelerApp* modelerApp);
    void setQtApp(QApplication* qtApp);
    RenderWindow* getRenderWindow();

protected slots:
    void sceneTreeSelectionChanged(const QItemSelection& oldItem,const QItemSelection& newItem);
    void browseForModel();
    void loadModel();
    void showImportSettings();
    void saveImportSettings();

private:
    class SceneTreeWidgetItem: public QTreeWidgetItem {
    public:
        Core::WeakPointer<Core::Object3D> sceneObject;
    };

    void setupStyles();
    void setUpGUI();
    QGroupBox* buildLoadModelGUI();
    QVBoxLayout* buildLeftLayout();
    QVBoxLayout* buildRightLayout();
    void updateSelectedSceneObjects(Core::WeakPointer<Core::Object3D> object, bool added);
    void updateSelectedSceneObjectsProperties();
    void populateSceneTree(QTreeWidget* sceneObjectTree, QTreeWidgetItem* parentItem, Core::WeakPointer<Core::Object3D> parentObject);
    void refreshSceneTree();
    void expandAllAbove(SceneTreeWidgetItem* item);
    void buildModelImportSettingsDialog();

    ModelerApp* modelerApp;
    QApplication* qtApp;

    RenderWindow *renderWindow;
    QPushButton *dockBtn;
    MainWindow *mainWindow;
    SceneTreeWidget* sceneObjectTree;
    QLineEdit* modelNameEdit;
    QDialog* modelImportSettingsDialog;

    QGroupBox* propertiesPlaceHolderFrame;
    QGroupBox* transformFrame;
    QLineEdit* positionX;
    QLineEdit* positionY;
    QLineEdit* positionZ;
    QLineEdit* rotationX;
    QLineEdit* rotationY;
    QLineEdit* rotationZ;
    QLineEdit* scaleX;
    QLineEdit* scaleY;
    QLineEdit* scaleZ;

    std::unordered_map<Core::UInt64, SceneTreeWidgetItem*> sceneObjectTreeMap;

    QString appStyle;
    QString titleLabelStyle;

    float modelImportScale;
    unsigned int modelImportSmoothingThreshold;
    bool modelImportZUp;
    bool modelImportPhsicalMaterial;

    QLineEdit* modelImportScaleEdit;
    QLineEdit* modelImportSmoothingThresholdEdit;
    QCheckBox* modelImportZUpCheckBox;
    QCheckBox* modelImportphysicalMaterialCheckBox;

};
