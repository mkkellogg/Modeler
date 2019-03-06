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
#include <QDesktopWidget>
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

MainGUI::MainGUI(MainWindow *mw): modelerApp(nullptr), qtApp(nullptr), mainWindow(mw), sceneObjectTree(nullptr),
  modelNameEdit(nullptr), modelScaleEdit(nullptr), modelSmoothingThresholdEdit(nullptr), modelZUpCheckBox(nullptr) {
    this->renderWindow = new RenderWindow;
    this->renderWindow->setObjectName("renderWindow");
    setWindowTitle(tr("Modeler"));
    this->setupStyles();
    this->setAutoFillBackground(true);
    this->setUpGUI();
}

void MainGUI::setModelerApp(ModelerApp* modelerApp) {
    if (this->modelerApp == nullptr) {
        this->modelerApp = modelerApp;
        this->modelerApp->getCoreScene().onSceneUpdated([this](Core::WeakPointer<Core::Object3D> object){
            this->refreshSceneTree();
        });
        this->modelerApp->getCoreScene().onSelectedObjectAdded([this](Core::WeakPointer<Core::Object3D> object){
            this->updateSelectedSceneObjects(object, true);
            this->updateSelectedSceneObjectsProperties();
        });
        this->modelerApp->getCoreScene().onSelectedObjectRemoved([this](Core::WeakPointer<Core::Object3D> object){
            this->updateSelectedSceneObjects(object, false);
            this->updateSelectedSceneObjectsProperties();
        });
        this->modelerApp->onUpdate([this]() {
            this->updateSelectedSceneObjectsProperties();
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
    QVBoxLayout * centerLayout = new QVBoxLayout;
    centerLayout->addWidget(this->renderWindow);
    lowerLayout->addLayout(leftLayout);
    lowerLayout->addLayout(centerLayout);
    lowerLayout->addLayout(rightLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(loadModelFrame);
    mainLayout->addLayout(lowerLayout);
    setLayout(mainLayout);

    this->setModelScaleEditText(3.0);
    this->setModelSmoothingThresholdEditText(80);
    this->setModelZUpCheck(true);
}

QGroupBox* MainGUI::buildLoadModelGUI() {
    QGroupBox* loadModelFrame = new QGroupBox(" Quick load ", this);
    loadModelFrame->setObjectName("loadModelFrame");
    QString hFrameStyle = QString("#loadModelFrame {border: 1px solid #aaa;}");
    loadModelFrame->setStyleSheet(hFrameStyle);

    this->modelNameEdit = new QLineEdit;
    QString modelNameEditStyle = QString("QLineEdit {width: 300px;}");
    this->modelNameEdit->setStyleSheet(modelNameEditStyle);

    //TODO: Remove this debug code
    this->modelNameEdit->setText("/home/mark/Development/models/Metal_Water_Tank/Water_Tank_fbx.fbx");

    this->modelScaleEdit = new QLineEdit;
    this->modelScaleEdit->setObjectName("modelScaleEdit");
    QString modelScaleEditStyle = QString("QLineEdit {width: 35px;}");
    this->modelScaleEdit->setStyleSheet(modelScaleEditStyle);
    this->modelScaleEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    this->modelSmoothingThresholdEdit = new QLineEdit;
    this->modelSmoothingThresholdEdit->setObjectName("modelSmoothingThresholdEdit");
    QString modelSmoothingThresholdEditStyle = QString("QLineEdit {width: 25px;}");
    this->modelSmoothingThresholdEdit->setStyleSheet(modelSmoothingThresholdEditStyle);
    this->modelSmoothingThresholdEdit->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel* loadLabel = new QLabel("Path: ");
    QLabel* scaleLabel = new QLabel(" Scale: ");
    QLabel* smoothingLabel = new QLabel(" Smoothing angle: ");
    QLabel* zUpLabel = new QLabel(" Z-up: ");
    QLabel* physicalMaterialLabel = new QLabel(" Physical material: ");


    QPushButton *browseButton = new QPushButton(this);
    connect(browseButton, SIGNAL(clicked()), SLOT(browseForModel()));
    browseButton->setText(tr("Browse"));

    QPushButton *loadButton = new QPushButton(this);
    connect(loadButton, SIGNAL(clicked()), SLOT(loadModel()));
    loadButton->setText(tr("Load"));

    this->modelZUpCheckBox = new QCheckBox;

    this->physicalMaterialCheckBox = new QCheckBox;

    QHBoxLayout *loadModelHLayout = new QHBoxLayout;
    loadModelHLayout->addWidget(loadLabel);
    loadModelHLayout->addWidget(this->modelNameEdit);
    loadModelHLayout->addWidget(browseButton);
    loadModelHLayout->addWidget(scaleLabel);
    loadModelHLayout->addWidget(this->modelScaleEdit);
    loadModelHLayout->addWidget(smoothingLabel);
    loadModelHLayout->addWidget(this->modelSmoothingThresholdEdit);
    loadModelHLayout->addWidget(zUpLabel);
    loadModelHLayout->addWidget(this->modelZUpCheckBox);
    loadModelHLayout->addWidget(physicalMaterialLabel);
    loadModelHLayout->addWidget(this->physicalMaterialCheckBox);
    loadModelHLayout->addWidget(loadButton);
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
    this->positionX = new QLineEdit;
    this->positionX->setObjectName("positionX");
    this->positionX->setStyleSheet(transformComponentStyle);
    this->positionX->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->positionY = new QLineEdit;
    this->positionX->setObjectName("positionY");
    this->positionY->setStyleSheet(transformComponentStyle);
    this->positionY->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->positionZ = new QLineEdit;
    this->positionZ->setObjectName("positionZ");
    this->positionZ->setStyleSheet(transformComponentStyle);
    this->positionZ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel* rotationLabel = new QLabel("Rotation: ");
    this->rotationX = new QLineEdit;
    this->rotationX->setObjectName("rotationX");
    this->rotationX->setStyleSheet(transformComponentStyle);
    this->rotationX->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->rotationY = new QLineEdit;
    this->rotationY->setObjectName("rotationY");
    this->rotationY->setStyleSheet(transformComponentStyle);
    this->rotationY->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->rotationZ = new QLineEdit;
    this->rotationZ->setObjectName("rotationZ");
    this->rotationZ->setStyleSheet(transformComponentStyle);
    this->rotationZ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    QLabel* scaleLabel = new QLabel("Scale: ");
    this->scaleX = new QLineEdit;
    this->scaleX->setObjectName("scaleX");
    this->scaleX->setStyleSheet(transformComponentStyle);
    this->scaleX->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->scaleY = new QLineEdit;
    this->scaleY->setObjectName("scaleY");
    this->scaleY->setStyleSheet(transformComponentStyle);
    this->scaleY->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    this->scaleZ = new QLineEdit;
    this->scaleZ->setObjectName("scaleZ");
    this->scaleZ->setStyleSheet(transformComponentStyle);
    this->scaleZ->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    transformFrameLayout->addWidget (positionLabel, 0, 0);
    transformFrameLayout->addWidget (positionX, 0, 1);
    transformFrameLayout->addWidget (positionY, 0, 2);
    transformFrameLayout->addWidget (positionZ, 0, 3);

    transformFrameLayout->addWidget (rotationLabel, 1, 0);
    transformFrameLayout->addWidget (rotationX, 1, 1);
    transformFrameLayout->addWidget (rotationY, 1, 2);
    transformFrameLayout->addWidget (rotationZ, 1, 3);

    transformFrameLayout->addWidget (scaleLabel, 2, 0);
    transformFrameLayout->addWidget (scaleX, 2, 1);
    transformFrameLayout->addWidget (scaleY, 2, 2);
    transformFrameLayout->addWidget (scaleZ, 2, 3);

    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->setAlignment(Qt::AlignTop);
    rightLayout->addWidget(transformFrame);
    rightLayout->addWidget(propertiesPlaceHolderFrame);
    return rightLayout;
}

void MainGUI::updateSelectedSceneObjectsProperties() {
    std::vector<Core::WeakPointer<Core::Object3D>>& selectedObjects = this->modelerApp->getCoreScene().getSelectedObjects();
    if (selectedObjects.size() == 1) {
        Core::WeakPointer<Core::Object3D> object = selectedObjects[0];
        Core::Matrix4x4 worldTransformation;
        object->getTransform().updateWorldMatrix();
        worldTransformation = object->getTransform().getWorldMatrix();
        Core::Vector3r translation;
        Core::Quaternion rotation;
        Core::Vector3r scale;
        Core::Vector3r euler;

        worldTransformation.decompose(translation, rotation, scale);
        euler = rotation.euler();

        std::ostringstream ss;

        ss << translation.x;
        this->positionX->setText(QString::fromStdString(ss.str()));
        ss.str("");
        ss << translation.y;
        this->positionY->setText(QString::fromStdString(ss.str()));
        ss.str("");
        ss << translation.z;
        this->positionZ->setText(QString::fromStdString(ss.str()));

        ss.str("");
        ss << euler.x;
        this->rotationX->setText(QString::fromStdString(ss.str()));
        ss.str("");
        ss << euler.y;
        this->rotationY->setText(QString::fromStdString(ss.str()));
        ss.str("");
        ss << euler.z;
        this->rotationZ->setText(QString::fromStdString(ss.str()));

        ss.str("");
        ss << scale.x;
        this->scaleX->setText(QString::fromStdString(ss.str()));
        ss.str("");
        ss << scale.y;
        this->scaleY->setText(QString::fromStdString(ss.str()));
        ss.str("");
        ss << scale.z;
        this->scaleZ->setText(QString::fromStdString(ss.str()));
        this->transformFrame->setVisible(true);
    }
    else {
        this->transformFrame->setVisible(false);
    }
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
    QString scaleQStr = this->modelScaleEdit->text();
    QString smoothingThresholdQStr = this->modelSmoothingThresholdEdit->text();
    bool zUp = this->modelZUpCheckBox->isChecked();
    bool usePhysicalMaterial = this->physicalMaterialCheckBox->isChecked();

    float scale = 1.0f;
    try {
        scale = std::stof(scaleQStr.toStdString());
    }
    catch (const std::invalid_argument& ia) {
        scale = 1.0f;
    }

    float smoothingThreshold = 80;
    try {
        smoothingThreshold = std::stof(smoothingThresholdQStr.toStdString());
    }
    catch (const std::invalid_argument& ia) {
        smoothingThreshold = 80;
    }

    this->modelerApp->loadModel(nameQStr.toStdString(), scale, smoothingThreshold, zUp, usePhysicalMaterial);
}

RenderWindow* MainGUI::getRenderWindow() {
    return this->renderWindow;
}

void MainGUI::setModelScaleEditText(float scale) {
    std::ostringstream ss;
    ss << scale;
    std::string scaleText(ss.str());
    this->modelScaleEdit->setText(QString::fromStdString(scaleText));
}

void MainGUI::updateSelectedSceneObjects(Core::WeakPointer<Core::Object3D> object, bool added) {
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

void MainGUI::setModelSmoothingThresholdEditText(float angle) {
    std::ostringstream ss;
    ss << angle;
    std::string scaleAngle(ss.str());
    this->modelSmoothingThresholdEdit->setText(QString::fromStdString(scaleAngle));
}

void MainGUI::setModelZUpCheck(bool checked) {
    this->modelZUpCheckBox->setChecked(checked);
}

void MainGUI::populateSceneTree(QTreeWidget* sceneTree, QTreeWidgetItem* parentItem, Core::WeakPointer<Core::Object3D> object) {
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
