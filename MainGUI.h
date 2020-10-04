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
class QHBoxLayout;
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
    void selectedObjectPositionXChanged();
    void selectedObjectPositionYChanged();
    void selectedObjectPositionZChanged();
    void selectedObjectEulerXChanged();
    void selectedObjectEulerYChanged();
    void selectedObjectEulerZChanged();
    void selectedObjectScaleXChanged();
    void selectedObjectScaleYChanged();
    void selectedObjectScaleZChanged();
    void sceneTreeSelectionChanged(const QItemSelection& oldItem,const QItemSelection& newItem);
    void browseForModel();
    void loadModel();
    void showImportSettings();
    void saveImportSettings();
    void updateModelImportPhysicalSettingsVisibility(bool checked);
    void setTransformModeTranslation();
    void setTransformModeRotation();

private:

    static const float OBJECT_PROPERTY_GUI_UPDATE_EPSILON;

    class SceneTreeWidgetItem: public QTreeWidgetItem {
    public:
        Core::WeakPointer<Core::Object3D> sceneObject;
    };

    void setupStyles();
    void setUpGUI();
    QGroupBox* buildLoadModelGUI();
    QVBoxLayout* buildLeftLayout();
    QVBoxLayout* buildRightLayout();
    QHBoxLayout* buildSceneToolsLayout();
    void updateGUIWithSelectedSceneObjects(Core::WeakPointer<Core::Object3D> object, bool added);
    void updateGUIWithSelectedSceneObjectsProperties(bool force = false);
    void updateSelectedSceneObjectsPropertiesFromGUI();
    void updateTextFieldIfChanged(QLineEdit* field, QString* str);
    void updateTextFieldFromNumberIfChanged(QLineEdit* field, Core::Real value, Core::Real& cachedValue, bool force = false);
    void updateSelectedObjectRealPropertyFromLineEdit(QLineEdit* lineEdit, Core::Real& dest);
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
    QLineEdit* positionXLineEdit;
    QLineEdit* positionYLineEdit;
    QLineEdit* positionZLineEdit;
    QLineEdit* eulerXLineEdit;
    QLineEdit* eulerYLineEdit;
    QLineEdit* eulerZLineEdit;
    QLineEdit* scaleXLineEdit;
    QLineEdit* scaleYLineEdit;
    QLineEdit* scaleZLineEdit;

    std::unordered_map<Core::UInt64, SceneTreeWidgetItem*> sceneObjectTreeMap;

    QString appStyle;
    QString titleLabelStyle;

    QLineEdit* modelImportScaleEdit;
    QLineEdit* modelImportSmoothingThresholdEdit;
    QCheckBox* modelImportZUpCheckBox;
    QCheckBox* modelImportphysicalMaterialCheckBox;
    QGroupBox* modeImportPhysicalSettingsFrame;
    QLineEdit* modelImportPhysicalMetallicEdit;
    QLineEdit* modelImportPhysicalRoughnessEdit;

    float modelImportScale;
    unsigned int modelImportSmoothingThreshold;
    bool modelImportZUp;
    bool modelImportPhysicalMaterial;
    float modelImportPhysicalMetallic = 0.0f;
    float modelImportPhysicalRoughness = 0.9f;

    Core::Vector3r selectedObjectTranslation;
    Core::Vector3r selectedObjectScale;
    Core::Vector3r selectedObjectEuler;
};
