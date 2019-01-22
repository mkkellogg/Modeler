#pragma once

#include <functional>
#include <unordered_map>

#include <QTreeWidgetItem>

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

    void setupStyles();
    void setUpGUI();
    QGroupBox* buildLoadModelGUI();
    QVBoxLayout* buildLeftLayout();
    QVBoxLayout* buildRightLayout();
    void selectSceneObject(Core::WeakPointer<Core::Object3D> object);
    void populateSceneTree(QTreeWidget* sceneObjectTree, QTreeWidgetItem* parentItem, Core::WeakPointer<Core::Object3D> parentObject);
    void refreshSceneTree();
    void expandAllAbove(SceneTreeWidgetItem* item);

    ModelerApp* modelerApp;
    QApplication* qtApp;

    RenderWindow *renderWindow;
    QPushButton *dockBtn;
    MainWindow *mainWindow;
    QTreeWidget* sceneObjectTree;
    QLineEdit* modelNameEdit;
    QLineEdit* modelScaleEdit;
    QLineEdit* modelSmoothingThresholdEdit;
    QCheckBox* modelZUpCheckBox;
    std::unordered_map<Core::UInt64, SceneTreeWidgetItem*> sceneObjectTreeMap;

    QString appStyle;
    QString titleLabelStyle;
};
