#include <sstream>

#include "RenderWindow.h"
#include "MainContainer.h"
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

MainContainer::MainContainer(MainWindow *mw): app(nullptr), mainWindow(mw), sceneObjectTree(nullptr),
  modelNameEdit(nullptr), modelScaleEdit(nullptr), modelSmoothingThresholdEdit(nullptr), modelZUpCheckBox(nullptr) {
    this->renderWindow = new RenderWindow;
    setWindowTitle(tr("Modeler"));
    this->setAutoFillBackground(true);
    this->setUpGUI();
}

void MainContainer::setApp(ModelerApp* app) {
    if (this->app == nullptr) {
        this->app = app;
        this->app->getCoreScene().onSceneUpdated([this](Core::WeakPointer<Core::Object3D> object){
            this->refreshSceneTree();
        });
        this->app->getCoreScene().onObjectSelected([this](Core::WeakPointer<Core::Object3D> object){
            this->selecScenetObject(object);
        });
    }
    else {
        throw Exception("MainContainer::setApp() -> Tried to set 'app' multiple times!");
    }
}

void MainContainer::setUpGUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QFrame *loadModelFrame = this->buildLoadModelGUI();
    QHBoxLayout *lowerLayout = new QHBoxLayout;
    mainLayout->addWidget(loadModelFrame);

    this->sceneObjectTree = new QTreeWidget;
    this->sceneObjectTree->setColumnCount(1);
    this-> sceneObjectTree->setHeaderHidden(true);
    this->sceneObjectTree->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    this->sceneObjectTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    this->sceneObjectTree->header()->setStretchLastSection(false);
    this->sceneObjectTree->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    connect(this->sceneObjectTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)), this, SLOT(sceneTreeSelectionChanged(const QItemSelection&,const QItemSelection&)));

    lowerLayout->addWidget(sceneObjectTree);
    lowerLayout->addWidget(this->renderWindow);
    mainLayout->addLayout(lowerLayout);
    setLayout(mainLayout);

    this->setModelScaleEditText(0.05);
    this->setModelSmoothingThresholdEditText(80);
    this->setModelZUpCheck(true);
}

QFrame* MainContainer::buildLoadModelGUI() {
    QFrame* loadModelFrame = new QFrame(this);
    loadModelFrame->setObjectName("loadModelFrame");
    QString hFrameStyle = QString("#loadModelFrame {border: 1px solid #aaa;}");
    loadModelFrame->setStyleSheet(hFrameStyle);

    this->modelNameEdit = new QLineEdit;
    QString modelNameEditStyle = QString("QLineEdit {width: 300px;}");
    this->modelNameEdit->setStyleSheet(modelNameEditStyle);

    //TODO: Remove this debug code
    this->modelNameEdit->setText("/home/mark/Development/GTE/resources/models/toonwarrior/character/warrior.fbx");

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

    QPushButton *browseButton = new QPushButton(this);
    connect(browseButton, SIGNAL(clicked()), SLOT(browseForModel()));
    browseButton->setText(tr("Browse"));

    QPushButton *loadButton = new QPushButton(this);
    connect(loadButton, SIGNAL(clicked()), SLOT(loadModel()));
    loadButton->setText(tr("Load"));

    this->modelZUpCheckBox = new QCheckBox;

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
    loadModelHLayout->addWidget(loadButton);
    loadModelFrame->setLayout(loadModelHLayout);
    return loadModelFrame;
}

void MainContainer::sceneTreeSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    QList<QTreeWidgetItem*> selectedItems = this->sceneObjectTree->selectedItems();
    for (const QTreeWidgetItem* item : selectedItems) {
        SceneTreeWidgetItem* sceneTreeItem = const_cast<SceneTreeWidgetItem*>(dynamic_cast<const SceneTreeWidgetItem*>(item));
        this->app->getCoreScene().setSelectedObject(sceneTreeItem->sceneObject);
    }
}

void MainContainer::browseForModel() {
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

void MainContainer::loadModel() {
    QString nameQStr = this->modelNameEdit->text();
    QString scaleQStr = this->modelScaleEdit->text();
    QString smoothingThresholdQStr = this->modelSmoothingThresholdEdit->text();
    bool zUp = this->modelZUpCheckBox->isChecked();

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

    this->app->loadModel(nameQStr.toStdString(), scale, smoothingThreshold, zUp);
}

RenderWindow* MainContainer::getRenderWindow() {
    return this->renderWindow;
}

void MainContainer::setModelScaleEditText(float scale) {
    std::ostringstream ss;
    ss << scale;
    std::string scaleText(ss.str());
    this->modelScaleEdit->setText(QString::fromStdString(scaleText));
}

void MainContainer::selecScenetObject(Core::WeakPointer<Core::Object3D> object) {
    Core::UInt64 objectID = object->getID();
    if (this->sceneObjectTreeMap.find(objectID) != this->sceneObjectTreeMap.end()) {
        this->sceneObjectTree->clearSelection();
        SceneTreeWidgetItem* mappedItem = this->sceneObjectTreeMap[objectID];
        this->expandAllAbove(mappedItem);
        mappedItem->setSelected(true);
    }
}

void MainContainer::setModelSmoothingThresholdEditText(float angle) {
    std::ostringstream ss;
    ss << angle;
    std::string scaleAngle(ss.str());
    this->modelSmoothingThresholdEdit->setText(QString::fromStdString(scaleAngle));
}

void MainContainer::setModelZUpCheck(bool checked) {
    this->modelZUpCheckBox->setChecked(checked);
}

void MainContainer::populateSceneTree(QTreeWidget* sceneTree, QTreeWidgetItem* parentItem, Core::WeakPointer<Core::Object3D> object) {
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

void MainContainer::refreshSceneTree() {
    Core::WeakPointer<Core::Object3D> sceneRoot = this->app->getCoreScene().getSceneRoot();
    this->sceneObjectTree->clear();
    this->sceneObjectTreeMap.clear();
    for (Core::SceneObjectIterator<Core::Object3D> itr = sceneRoot->beginIterateChildren(); itr != sceneRoot->endIterateChildren(); ++itr) {
        Core::WeakPointer<Core::Object3D> childObject = *itr;
        this->populateSceneTree(this->sceneObjectTree, nullptr, childObject);
    }
}

void MainContainer::expandAllAbove(SceneTreeWidgetItem* item) {
    QTreeWidgetItem* curItem = item;
    while(curItem != nullptr) {
        curItem->setExpanded(true);
        curItem = curItem->parent();
    }
}
