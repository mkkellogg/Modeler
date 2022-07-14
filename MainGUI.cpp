#include <sstream>

#include "RenderWindow.h"
#include "MainGUI.h"
#include "MainWindow.h"
#include "ModelerApp.h"
#include "Exception.h"

#include <QSlider>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPushButton>
#include <QScreen>
#include <QApplication>
#include <QMessageBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QTreeWidget>
#include <QHeaderView>
#include <QCheckBox>

#include "Core/math/Quaternion.h"
#include "Core/scene/Scene.h"
#include "Core/material/StandardPhysicalMaterial.h"
#include "Core/render/MeshRenderer.h"

const float MainGUI::OBJECT_PROPERTY_GUI_UPDATE_EPSILON = 0.000005f;

MainGUI::MainGUI(MainWindow *mw): modelerApp(nullptr), qtApp(nullptr), mainWindow(mw), sceneObjectTree(nullptr),
    modelImportScaleEdit(nullptr), modelImportSmoothingThresholdEdit(nullptr), modelImportZUpCheckBox(nullptr), modelImportphysicalMaterialCheckBox(nullptr) {
    this->modelImportScale = 0.1f;
    this->modelImportSmoothingThreshold = 80;
    this->modelImportZUp = true;
    this->modelImportPhysicalMaterial = true;

    this->renderWindow = new RenderWindow;
    this->renderWindow->setObjectName("renderWindow");
    setWindowTitle(tr("Modeler"));
    this->setupStyles();
    this->setAutoFillBackground(true);
    this->setUpGUI();

    this->modelNameEdit->setText("/home/mark/Development/Qt/build-modeler2-Desktop_Qt_5_12_5_GCC_64bit-Debug/assets/models/tree_00/tree_00.fbx");
}

void MainGUI::setModelerApp(ModelerApp* modelerApp) {
    if (this->modelerApp == nullptr) {
        this->modelerApp = modelerApp;
        this->modelerApp->getCoreScene().onSceneUpdated([this](Core::WeakPointer<Core::Object3D> object){
            this->refreshSceneTree();
        });
        this->modelerApp->getCoreScene().onSelectedObjectAdded([this](Core::WeakPointer<Core::Object3D> object){
            this->updateGUIWithSelectedSceneObjects(object, true);
            this->updateGUIWithSelectedSceneObjectsProperties(true);
        });
        this->modelerApp->getCoreScene().onSelectedObjectRemoved([this](Core::WeakPointer<Core::Object3D> object){
            this->updateGUIWithSelectedSceneObjects(object, false);
            this->updateGUIWithSelectedSceneObjectsProperties(true);
        });
        this->modelerApp->onUpdate([this]() {
            this->updateGUIWithSelectedSceneObjectsProperties(false);
        });
    }
    else {
        throw Exception("MainGUI::setModelerApp() -> Tried to set 'modelerApp' multiple times!");
    }
}

void MainGUI::setQtApp(QApplication* qtApp) {
    if (this->qtApp == nullptr) {
        this->qtApp = qtApp;
    }
    else {
        throw Exception("MainGUI::setQtApp() -> Tried to set 'qtApp' multiple times!");
    }
}

void MainGUI::setupStyles() {
    this->appStyle = QString(
        "QLabel {"
        "   font-weight:none; font-size: 10pt;"
        "}"
        "QLineEdit {"
        "   font-weight:none; font-size: 10pt;"
        "}"
        "QPushButton {"
        "   font-weight:none; font-size: 10pt;"
        "}"
        "QTreeWidget {"
        "   font-weight:none; font-size: 10pt;"
        "}"
        "QGroupBox {"
        "   font-weight:bold; padding-top:0px; margin-top: 10px; border: 1px solid #aaa;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   subcontrol-position: top left;"
        "   top:0px;"
        "   left: 10px;"
        "}"
    );

    this->titleLabelStyle = QString(
        "QLabel {"
        "   font-weight:bold; font-size: 11pt;"
        "}"
    );

    this->qtApp->setStyleSheet(this->appStyle);
}

void MainGUI::setUpGUI() {

    QGroupBox *loadModelFrame = this->buildLoadModelGUI();

    QHBoxLayout * lowerLayout = new QHBoxLayout;
    QVBoxLayout * leftLayout = this->buildLeftLayout();
    QVBoxLayout * rightLayout = this->buildRightLayout();
    QHBoxLayout * sceneToolsLayout = this->buildSceneToolsLayout();
    QVBoxLayout * centerLayout = new QVBoxLayout;
    centerLayout->addWidget(this->renderWindow);
    lowerLayout->addLayout(leftLayout);
    lowerLayout->addLayout(centerLayout);
    lowerLayout->addLayout(rightLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(loadModelFrame);
    mainLayout->addLayout(sceneToolsLayout);
    mainLayout->addLayout(lowerLayout);
    setLayout(mainLayout);

    this->buildModelImportSettingsDialog();
}

QGroupBox* MainGUI::buildLoadModelGUI() {
    QGroupBox* loadModelFrame = new QGroupBox(" Quick load ", this);
    loadModelFrame->setObjectName("loadModelFrame");
    QString hFrameStyle = QString("#loadModelFrame {border: 1px solid #aaa;}");
    loadModelFrame->setStyleSheet(hFrameStyle);

    this->modelNameEdit = new QLineEdit;
    QString modelNameEditStyle = QString("QLineEdit {width: 300px;}");
    this->modelNameEdit->setStyleSheet(modelNameEditStyle);
    QLabel* loadLabel = new QLabel("Path: ");

    QPushButton *browseButton = new QPushButton(this);
    connect(browseButton, SIGNAL(clicked()), SLOT(browseForModel()));
    browseButton->setText(tr("Browse"));

    QPushButton *loadButton = new QPushButton(this);
    connect(loadButton, SIGNAL(clicked()), SLOT(loadModel()));
    loadButton->setText(tr("Load"));

    QPushButton *importSettingsButton = new QPushButton(this);
    connect(importSettingsButton, SIGNAL(clicked()), SLOT(showImportSettings()));
    importSettingsButton->setText(tr("Import Settings..."));

    QHBoxLayout *loadModelHLayout = new QHBoxLayout;
    loadModelHLayout->addWidget(loadLabel);
    loadModelHLayout->addWidget(this->modelNameEdit);
    loadModelHLayout->addWidget(browseButton);
    loadModelHLayout->addWidget(loadButton);
    loadModelHLayout->addWidget(importSettingsButton);
    loadModelFrame->setLayout(loadModelHLayout);
    return loadModelFrame;
}

QVBoxLayout* MainGUI::buildLeftLayout() {

    this->sceneObjectTree = new SceneTreeWidget;
    this->sceneObjectTree->setStyleSheet("QTreeWidget {margin: 0px; border:none; background-color: transparent;}");
    this->sceneObjectTree->setColumnCount(1);
    this->sceneObjectTree->setHeaderHidden(true);
    this->sceneObjectTree->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    this->sceneObjectTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->sceneObjectTree->header()->setStretchLastSection(false);
    this->sceneObjectTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->sceneObjectTree->setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(this->sceneObjectTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(sceneTreeSelectionChanged(const QItemSelection&,const QItemSelection&)));

    QGroupBox* sceneObjectTreeFrame = new QGroupBox(" Scene Tree ", this);
    sceneObjectTreeFrame->setStyleSheet("QGroupBox {background-color: #ffffff;}");
    sceneObjectTreeFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    QVBoxLayout* sceneObjectTreeFrameLayout = new QVBoxLayout;
    sceneObjectTreeFrame->setLayout(sceneObjectTreeFrameLayout);
    sceneObjectTreeFrameLayout->addWidget(this->sceneObjectTree);

    QVBoxLayout* leftLayout = new QVBoxLayout;
    leftLayout->addWidget(sceneObjectTreeFrame);
    return leftLayout;
}

QVBoxLayout* MainGUI::buildRightLayout() {

    this->propertiesPlaceHolderFrame = new QGroupBox(this);
    this->propertiesPlaceHolderFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    QString placeHolderFrameStyle = QString("QGroupBox {width: 300px; border:none;}");
    this->propertiesPlaceHolderFrame->setStyleSheet(placeHolderFrameStyle);

    this->transformFrame = new QGroupBox (" Transform ", this);
    transformFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    transformFrame->setObjectName("transformFrame");
    QGridLayout *transformFrameLayout = new QGridLayout;
    transformFrame->setLayout(transformFrameLayout);

    QString transformComponentStyle = QString("QLineEdit {width: 55px;}");

    QLabel* positionLabel = new QLabel("Position: ");
    this->positionXLineEdit = new QLineEdit;
    this->positionXLineEdit->setObjectName("positionX");
    this->positionXLineEdit->setStyleSheet(transformComponentStyle);
    this->positionXLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->positionYLineEdit = new QLineEdit;
    this->positionYLineEdit->setObjectName("positionY");
    this->positionYLineEdit->setStyleSheet(transformComponentStyle);
    this->positionYLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->positionZLineEdit = new QLineEdit;
    this->positionZLineEdit->setObjectName("positionZ");
    this->positionZLineEdit->setStyleSheet(transformComponentStyle);
    this->positionZLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel* rotationLabel = new QLabel("Rotation: ");
    this->eulerXLineEdit = new QLineEdit;
    this->eulerXLineEdit->setObjectName("eulerX");
    this->eulerXLineEdit->setStyleSheet(transformComponentStyle);
    this->eulerXLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->eulerYLineEdit = new QLineEdit;
    this->eulerYLineEdit->setObjectName("eulerY");
    this->eulerYLineEdit->setStyleSheet(transformComponentStyle);
    this->eulerYLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->eulerZLineEdit = new QLineEdit;
    this->eulerZLineEdit->setObjectName("eulerZ");
    this->eulerZLineEdit->setStyleSheet(transformComponentStyle);
    this->eulerZLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel* scaleLabel = new QLabel("Scale: ");
    this->scaleXLineEdit = new QLineEdit;
    this->scaleXLineEdit->setObjectName("scaleX");
    this->scaleXLineEdit->setStyleSheet(transformComponentStyle);
    this->scaleXLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->scaleYLineEdit = new QLineEdit;
    this->scaleYLineEdit->setObjectName("scaleY");
    this->scaleYLineEdit->setStyleSheet(transformComponentStyle);
    this->scaleYLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->scaleZLineEdit = new QLineEdit;
    this->scaleZLineEdit->setObjectName("scaleZ");
    this->scaleZLineEdit->setStyleSheet(transformComponentStyle);
    this->scaleZLineEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    transformFrameLayout->addWidget (positionLabel, 0, 0);
    transformFrameLayout->addWidget (this->positionXLineEdit, 0, 1);
    transformFrameLayout->addWidget (this->positionYLineEdit, 0, 2);
    transformFrameLayout->addWidget (this->positionZLineEdit, 0, 3);
    connect(this->positionXLineEdit, SIGNAL(editingFinished()), SLOT(selectedObjectPositionXChanged()));
    connect(this->positionYLineEdit, SIGNAL(editingFinished()), SLOT(selectedObjectPositionYChanged()));
    connect(this->positionZLineEdit, SIGNAL(editingFinished()), SLOT(selectedObjectPositionZChanged()));

    transformFrameLayout->addWidget (rotationLabel, 1, 0);
    transformFrameLayout->addWidget (this->eulerXLineEdit, 1, 1);
    transformFrameLayout->addWidget (this->eulerYLineEdit, 1, 2);
    transformFrameLayout->addWidget (this->eulerZLineEdit, 1, 3);
    connect(this->eulerXLineEdit, SIGNAL(editingFinished()), SLOT(selectedObjectEulerXChanged()));
    connect(this->eulerYLineEdit, SIGNAL(editingFinished()), SLOT(selectedObjectEulerYChanged()));
    connect(this->eulerZLineEdit, SIGNAL(editingFinished()), SLOT(selectedObjectEulerZChanged()));


    transformFrameLayout->addWidget (scaleLabel, 2, 0);
    transformFrameLayout->addWidget (this->scaleXLineEdit, 2, 1);
    transformFrameLayout->addWidget (this->scaleYLineEdit, 2, 2);
    transformFrameLayout->addWidget (this->scaleZLineEdit, 2, 3);
    connect(this->scaleXLineEdit, SIGNAL(editingFinished()), SLOT(selectedObjectScaleXChanged()));
    connect(this->scaleYLineEdit, SIGNAL(editingFinished()), SLOT(selectedObjectScaleYChanged()));
    connect(this->scaleZLineEdit, SIGNAL(editingFinished()), SLOT(selectedObjectScaleZChanged()));

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setAlignment(Qt::AlignTop);
    rightLayout->addWidget(transformFrame);
    rightLayout->addWidget(propertiesPlaceHolderFrame);
    return rightLayout;
}

QHBoxLayout* MainGUI::buildSceneToolsLayout() {
    QHBoxLayout *sceneToolsLayout = new QHBoxLayout;
    sceneToolsLayout->setAlignment(Qt::AlignLeft);

    QPushButton *translationButton = new QPushButton(this);
    connect(translationButton, SIGNAL(clicked()), SLOT(setTransformModeTranslation()));
    QPushButton *rotationButton = new QPushButton(this);
    connect(rotationButton, SIGNAL(clicked()), SLOT(setTransformModeRotation()));

    QPixmap movePixmap("assets/icons/move_reduced.png");
    QIcon moveIcon(movePixmap);
    translationButton->setIcon(moveIcon);
    translationButton->setIconSize(movePixmap.rect().size());
    translationButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    translationButton->setFixedSize(movePixmap.rect().size());

    QPixmap rotationPixmap("assets/icons/rotate_reduced.png");
    QIcon rotationIcon(rotationPixmap);
    rotationButton->setIcon(rotationIcon);
    rotationButton->setIconSize(rotationPixmap.rect().size());
    rotationButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    rotationButton->setFixedSize(rotationPixmap.rect().size());

    sceneToolsLayout->addWidget(translationButton);
    sceneToolsLayout->addWidget(rotationButton);
    return sceneToolsLayout;

}

void MainGUI::updateGUIWithSelectedSceneObjectsProperties(bool force) {
    static bool initialized = false;

    std::vector<Core::WeakPointer<Core::Object3D>>& selectedObjects = this->modelerApp->getCoreScene().getSelectedObjects();
    if (selectedObjects.size() == 1) {
        Core::WeakPointer<Core::Object3D> object = selectedObjects[0];
        Core::Matrix4x4 localTransformation = object->getTransform().getLocalMatrix();
        Core::Vector3r translation;
        Core::Quaternion rotation;
        Core::Vector3r scale;
        Core::Vector3r euler;

        localTransformation.decompose(translation, rotation, scale);
        euler = rotation.euler();

        std::ostringstream ss;

        this->updateTextFieldFromNumberIfChanged(this->positionXLineEdit, translation.x, this->selectedObjectTranslation.x, !initialized || force);
        this->updateTextFieldFromNumberIfChanged(this->positionYLineEdit, translation.y, this->selectedObjectTranslation.y, !initialized || force);
        this->updateTextFieldFromNumberIfChanged(this->positionZLineEdit, translation.z, this->selectedObjectTranslation.z, !initialized || force);

        this->updateTextFieldFromNumberIfChanged(this->eulerXLineEdit, euler.x, this->selectedObjectEuler.x, !initialized || force);
        this->updateTextFieldFromNumberIfChanged(this->eulerYLineEdit, euler.y, this->selectedObjectEuler.y, !initialized || force);
        this->updateTextFieldFromNumberIfChanged(this->eulerZLineEdit, euler.z, this->selectedObjectEuler.z, !initialized || force);

        this->updateTextFieldFromNumberIfChanged(this->scaleXLineEdit, scale.x, this->selectedObjectScale.x, !initialized || force);
        this->updateTextFieldFromNumberIfChanged(this->scaleYLineEdit, scale.y, this->selectedObjectScale.y, !initialized || force);
        this->updateTextFieldFromNumberIfChanged(this->scaleZLineEdit, scale.z, this->selectedObjectScale.z, !initialized || force);

        this->transformFrame->setVisible(true);
        initialized = true;
    }
    else {
        this->transformFrame->setVisible(false);
    }
}

void MainGUI::updateSelectedSceneObjectsPropertiesFromGUI() {
    static Core::Matrix4x4 localMatrix;
    static Core::Matrix4x4 scaleMatrix;
    std::vector<Core::WeakPointer<Core::Object3D>>& selectedObjects = this->modelerApp->getCoreScene().getSelectedObjects();
    if (selectedObjects.size() == 1) {
        Core::WeakPointer<Core::Object3D> object = selectedObjects[0];
        object->getTransform().getLocalMatrix().compose(this->selectedObjectTranslation, this->selectedObjectEuler, this->selectedObjectScale);
    }
}

void MainGUI::updateTextFieldIfChanged(QLineEdit* field, QString* str) {
    if (field->text() != *str) {
        field->setText(*str);
    }
}

void MainGUI::updateTextFieldFromNumberIfChanged(QLineEdit* field, Core::Real value, Core::Real& cachedValue, bool force) {
    static std::ostringstream ss;;
    if (force || Core::Math::abs(value - cachedValue) > OBJECT_PROPERTY_GUI_UPDATE_EPSILON) {
        ss.str("");
        ss << value;
        field->setText(QString::fromStdString(ss.str()));
        cachedValue = value;
    }
}

void MainGUI::updateSelectedObjectRealPropertyFromLineEdit(QLineEdit* lineEdit, Core::Real& dest) {
    bool success;
    float temp = lineEdit->text().toFloat(&success);
    if (success) {
        if (Core::Math::abs(temp - dest) > OBJECT_PROPERTY_GUI_UPDATE_EPSILON) {
            dest = temp;
            this->updateSelectedSceneObjectsPropertiesFromGUI();
        }
    }
}

void MainGUI::selectedObjectPositionXChanged() {
    this->updateSelectedObjectRealPropertyFromLineEdit(this->positionXLineEdit, this->selectedObjectTranslation.x);
}

void MainGUI::selectedObjectPositionYChanged() {
    this->updateSelectedObjectRealPropertyFromLineEdit(this->positionYLineEdit, this->selectedObjectTranslation.y);
}

void MainGUI::selectedObjectPositionZChanged() {
    this->updateSelectedObjectRealPropertyFromLineEdit(this->positionZLineEdit, this->selectedObjectTranslation.z);
}

void MainGUI::selectedObjectEulerXChanged() {
    this->updateSelectedObjectRealPropertyFromLineEdit(this->eulerXLineEdit, this->selectedObjectEuler.x);
}

void MainGUI::selectedObjectEulerYChanged() {
    this->updateSelectedObjectRealPropertyFromLineEdit(this->eulerYLineEdit, this->selectedObjectEuler.y);
}

void MainGUI::selectedObjectEulerZChanged() {
    this->updateSelectedObjectRealPropertyFromLineEdit(this->eulerZLineEdit, this->selectedObjectEuler.z);
}

void MainGUI::selectedObjectScaleXChanged() {
    this->updateSelectedObjectRealPropertyFromLineEdit(this->scaleXLineEdit, this->selectedObjectScale.x);
}

void MainGUI::selectedObjectScaleYChanged() {
    this->updateSelectedObjectRealPropertyFromLineEdit(this->scaleYLineEdit, this->selectedObjectScale.y);
}

void MainGUI::selectedObjectScaleZChanged() {
    this->updateSelectedObjectRealPropertyFromLineEdit(this->scaleZLineEdit, this->selectedObjectScale.z);
}

void MainGUI::sceneTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    QModelIndexList selectedIndexes = selected.indexes();
    QModelIndexList deselectedIndexes = deselected.indexes();

    for (unsigned int i = 0; i < deselectedIndexes.size(); i++) {
        QTreeWidgetItem * item = this->sceneObjectTree->getItemFromIndex(deselectedIndexes[i]);
        SceneTreeWidgetItem* sceneTreeItem = const_cast<SceneTreeWidgetItem*>(dynamic_cast<const SceneTreeWidgetItem*>(item));
        this->modelerApp->getCoreScene().removeSelectedObject(sceneTreeItem->sceneObject);
    }

    for (unsigned int i = 0; i < selectedIndexes.size(); i++) {
        QTreeWidgetItem * item = this->sceneObjectTree->getItemFromIndex(selectedIndexes[i]);
        SceneTreeWidgetItem* sceneTreeItem = const_cast<SceneTreeWidgetItem*>(dynamic_cast<const SceneTreeWidgetItem*>(item));
        this->modelerApp->getCoreScene().addSelectedObject(sceneTreeItem->sceneObject);
    }

}

void MainGUI::browseForModel() {
    QFileDialog dialog(this);
    dialog.setFileMode(QFileDialog::ExistingFiles);
    QStringList selectedFiles;
    if (dialog.exec()) {
        selectedFiles = dialog.selectedFiles();
    }

    if (selectedFiles.size() > 0) {
        this->modelNameEdit->setText(selectedFiles.at(0));
    }
}

void MainGUI::loadModel() {
    QString nameQStr = this->modelNameEdit->text();
    float scale = this->modelImportScale;
    float smoothingThreshold = this->modelImportSmoothingThreshold;
    this->modelerApp->loadModel(nameQStr.toStdString(), scale, smoothingThreshold, this->modelImportZUp, true, true, this->modelImportPhysicalMaterial, [this](Core::WeakPointer<Core::Object3D> rootObject){
       Core::WeakPointer<Core::Scene> scene = this->modelerApp->getEngine()->getActiveScene();
       scene->visitScene(rootObject, [this, rootObject](Core::WeakPointer<Core::Object3D> obj){
           Core::WeakPointer<Core::BaseRenderableContainer> baseRenderableContainer = obj->getBaseRenderableContainer();
           if (baseRenderableContainer.isValid()) {
               Core::WeakPointer<Core::MeshContainer> meshContainer = Core::WeakPointer<Core::BaseRenderableContainer>::dynamicPointerCast<Core::MeshContainer>(baseRenderableContainer);
               if (meshContainer) {
                   Core::WeakPointer<Core::Object3DRenderer<Core::Mesh>> objectRenderer = Core::WeakPointer<Core::BaseObject3DRenderer>::dynamicPointerCast<Core::Object3DRenderer<Core::Mesh>>(obj->getBaseRenderer());
                   if (objectRenderer) {
                       Core::WeakPointer<Core::MeshRenderer> meshRenderer = Core::WeakPointer<Core::Object3DRenderer<Core::Mesh>>::dynamicPointerCast<Core::MeshRenderer>(objectRenderer);
                       if (meshRenderer) {
                           Core::WeakPointer<Core::Material> renderMaterial = meshRenderer->getMaterial();
                           Core::WeakPointer<Core::StandardPhysicalMaterial> physicalMaterial = Core::WeakPointer<Core::Material>::dynamicPointerCast<Core::StandardPhysicalMaterial>(renderMaterial);
                           if (physicalMaterial) {
                               physicalMaterial->setMetallic(this->modelImportPhysicalMetallic);
                               physicalMaterial->setRoughness(this->modelImportPhysicalRoughness);
                           }
                       }
                   }
               }
           }
       });
    });
}

void MainGUI::showImportSettings() {
    this->modelImportScaleEdit->setText(QString::fromStdString(std::to_string(this->modelImportScale)));
    this->modelImportSmoothingThresholdEdit->setText(QString::fromStdString(std::to_string(this->modelImportSmoothingThreshold)));
    this->modelImportZUpCheckBox->setChecked(this->modelImportZUp);
    this->modelImportphysicalMaterialCheckBox->setChecked(this->modelImportPhysicalMaterial);
    this->modelImportSettingsDialog->setVisible(true);
    this->modelImportPhysicalMetallicEdit->setText(QString::fromStdString(std::to_string(this->modelImportPhysicalMetallic)));
    this->modelImportPhysicalRoughnessEdit->setText(QString::fromStdString(std::to_string(this->modelImportPhysicalRoughness)));
}

void MainGUI::saveImportSettings() {
    QString scaleQStr = this->modelImportScaleEdit->text();
    QString smoothingThresholdQStr = this->modelImportSmoothingThresholdEdit->text();
    QString physicalMetallicStr = this->modelImportPhysicalMetallicEdit->text();
    QString physicalRoughnessStr = this->modelImportPhysicalRoughnessEdit->text();

    try {
        this->modelImportScale = std::stof(scaleQStr.toStdString());
    }
    catch (const std::invalid_argument& ia) {
        this->modelImportScale = 1.0f;
    }

    try {
        this->modelImportSmoothingThreshold = std::stof(smoothingThresholdQStr.toStdString());
    }
    catch (const std::invalid_argument& ia) {
        this->modelImportSmoothingThreshold  = 80;
    }

    try {
        this->modelImportPhysicalMetallic = std::stof(physicalMetallicStr.toStdString());
    }
    catch (const std::invalid_argument& ia) {
        this->modelImportPhysicalMetallic  = 0.0f;
    }

    try {
        this->modelImportPhysicalRoughness = std::stof(physicalRoughnessStr.toStdString());
    }
    catch (const std::invalid_argument& ia) {
        this->modelImportPhysicalRoughness  = 0.9f;
    }

    this->modelImportZUp = this->modelImportZUpCheckBox->isChecked();
    this->modelImportPhysicalMaterial = this->modelImportphysicalMaterialCheckBox->isChecked();

    this->modelImportSettingsDialog->setVisible(false);
}

RenderWindow* MainGUI::getRenderWindow() {
    return this->renderWindow;
}

void MainGUI::updateGUIWithSelectedSceneObjects(Core::WeakPointer<Core::Object3D> object, bool added) {
    Core::UInt64 objectID = object->getID();
    if (this->sceneObjectTreeMap.find(objectID) != this->sceneObjectTreeMap.end()) {
        if (added) {
            SceneTreeWidgetItem* mappedItem = this->sceneObjectTreeMap[objectID];
            this->expandAllAbove(mappedItem);
            mappedItem->setSelected(true);
        }
        else {
            SceneTreeWidgetItem* mappedItem = this->sceneObjectTreeMap[objectID];
            mappedItem->setSelected(false);
        }
    }
}

void MainGUI::populateSceneTree(QTreeWidget* sceneTree, QTreeWidgetItem* parentItem, Core::WeakPointer<Core::Object3D> object) {
    if (this->modelerApp->isSceneObjectHidden(object)) return;
    SceneTreeWidgetItem* childItem = new SceneTreeWidgetItem();
    childItem->sceneObject = object;
    childItem->setText(0, QString::fromStdString(object->getName()));
    this->sceneObjectTreeMap[object->getID()] = childItem;

    if (parentItem == nullptr) {
        sceneTree->addTopLevelItem(childItem);
    }
    else {
        parentItem->addChild(childItem);
    }

    for (Core::SceneObjectIterator<Core::Object3D> itr = object->beginIterateChildren(); itr != object->endIterateChildren(); ++itr) {
        Core::WeakPointer<Core::Object3D> childObject = *itr;
        this->populateSceneTree(sceneTree, childItem, childObject);
    }
}

void MainGUI::refreshSceneTree() {
    Core::WeakPointer<Core::Object3D> sceneRoot = this->modelerApp->getCoreScene().getSceneRoot();
    this->sceneObjectTree->clear();
    this->sceneObjectTreeMap.clear();
    for (Core::SceneObjectIterator<Core::Object3D> itr = sceneRoot->beginIterateChildren(); itr != sceneRoot->endIterateChildren(); ++itr) {
        Core::WeakPointer<Core::Object3D> childObject = *itr;
        this->populateSceneTree(this->sceneObjectTree, nullptr, childObject);
    }
}

void MainGUI::expandAllAbove(SceneTreeWidgetItem* item) {
    QTreeWidgetItem* curItem = item;
    while(curItem != nullptr) {
        curItem->setExpanded(true);
        curItem = curItem->parent();
    }
}

void MainGUI::buildModelImportSettingsDialog() {
    this->modelImportSettingsDialog = new QDialog(this);
    this->modelImportSettingsDialog->setWindowTitle("Import settings");
    QString modelImportSettingsDialogStyle = QString("QGroupBox {margin: 10px;}");
    this->modelImportSettingsDialog->setStyleSheet(modelImportSettingsDialogStyle);
    this->modelImportSettingsDialog->setWindowModality(Qt::ApplicationModal);
    this->modelImportSettingsDialog->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->modelImportSettingsDialog->setFixedSize(270, 290);

    QVBoxLayout * modelImportSettingsDialogLayout = new QVBoxLayout;
    this->modelImportSettingsDialog->setLayout(modelImportSettingsDialogLayout);

    QGroupBox* modeImportSettingsFrame = new QGroupBox ("", this );
    modeImportSettingsFrame->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    modeImportSettingsFrame->setObjectName("importSettingsFrame");
    QGridLayout* modelImportSettingsFrameLayout = new QGridLayout;
    modeImportSettingsFrame->setLayout(modelImportSettingsFrameLayout);

    QLabel* modelImportScaleLabel = new QLabel("Scale: ");
    this->modelImportScaleEdit = new QLineEdit;
    this->modelImportScaleEdit->setObjectName("modelImportScale");
    this->modelImportScaleEdit->setStyleSheet(QString("QLineEdit {width: 55px;}"));
    this->modelImportScaleEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel* modelImportSmoothingThresholdLabel = new QLabel("Smoothing threshold: ");
    this->modelImportSmoothingThresholdEdit = new QLineEdit;
    this->modelImportSmoothingThresholdEdit->setObjectName("modelImportSmoothingThreshold");
    this->modelImportSmoothingThresholdEdit->setStyleSheet(QString("QLineEdit {width: 55px;}"));
    this->modelImportSmoothingThresholdEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel* modelImportZUpLabel = new QLabel(" Z-up: ");
    QLabel* modelImportPhysicalMaterialLabel = new QLabel(" Physical material: ");
    this->modelImportZUpCheckBox = new QCheckBox;
    this->modelImportphysicalMaterialCheckBox = new QCheckBox;

    this->modeImportPhysicalSettingsFrame = new QGroupBox ("", this );
    this->modeImportPhysicalSettingsFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QGridLayout * modelImportPhysicalSettingsLayout = new QGridLayout;
    this->modeImportPhysicalSettingsFrame->setLayout(modelImportPhysicalSettingsLayout);
    QLabel* modelImportPhysicalMetallicLabel = new QLabel("    Metallic: ");
    this->modelImportPhysicalMetallicEdit = new QLineEdit;
    QLabel* modelImportPhysicalRoughnessLabel = new QLabel("    Roughness: ");
    this->modelImportPhysicalRoughnessEdit = new QLineEdit;
    modelImportPhysicalSettingsLayout->addWidget(modelImportPhysicalMetallicLabel, 0, 0);
    modelImportPhysicalSettingsLayout->addWidget(modelImportPhysicalMetallicEdit, 0, 1);
    modelImportPhysicalSettingsLayout->addWidget(modelImportPhysicalRoughnessLabel, 1, 0);
    modelImportPhysicalSettingsLayout->addWidget(modelImportPhysicalRoughnessEdit, 1, 1);
    this->modeImportPhysicalSettingsFrame->setEnabled(false);
    connect(this->modelImportphysicalMaterialCheckBox, SIGNAL(clicked(bool)), SLOT(updateModelImportPhysicalSettingsVisibility(bool)));

    modelImportSettingsFrameLayout->addWidget (modelImportScaleLabel, 0, 0);
    modelImportSettingsFrameLayout->addWidget (this->modelImportScaleEdit, 0, 1);
    modelImportSettingsFrameLayout->addWidget (modelImportSmoothingThresholdLabel, 1, 0);
    modelImportSettingsFrameLayout->addWidget (this->modelImportSmoothingThresholdEdit, 1, 1);
    modelImportSettingsFrameLayout->addWidget (modelImportZUpLabel, 2, 0);
    modelImportSettingsFrameLayout->addWidget (this->modelImportZUpCheckBox, 2, 1);
    modelImportSettingsFrameLayout->addWidget (modelImportPhysicalMaterialLabel, 3, 0);
    modelImportSettingsFrameLayout->addWidget (this->modelImportphysicalMaterialCheckBox, 3, 1);
    modelImportSettingsFrameLayout->addWidget( this->modeImportPhysicalSettingsFrame, 4, 0, 1, 2);

    QPushButton *saveButton = new QPushButton(this);
    connect(saveButton, SIGNAL(clicked()), SLOT(saveImportSettings()));
    saveButton->setText(tr("Save"));

    modelImportSettingsDialogLayout->addWidget(modeImportSettingsFrame);
    modelImportSettingsDialogLayout->addWidget(saveButton);
}

void MainGUI::updateModelImportPhysicalSettingsVisibility(bool checked) {
    this->modeImportPhysicalSettingsFrame->setEnabled(checked);
}

void MainGUI::setTransformModeTranslation() {
    this->modelerApp->setTransformModeTranslation();
}

void MainGUI::setTransformModeRotation() {
    this->modelerApp->setTransformModeRotation();
}
