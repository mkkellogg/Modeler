#include <sstream>

#include "RenderWindow.h"
#include "MainContainer.h"
#include "MainWindow.h"
#include "ModelerApp.h"

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

MainContainer::MainContainer(MainWindow *mw): app(nullptr), mainWindow(mw), sceneTree(nullptr),
  modelNameEdit(nullptr), modelScaleEdit(nullptr), modelSmoothingThresholdEdit(nullptr), modelZUpCheckBox(nullptr) {
    this->renderWindow = new RenderWindow;
    setWindowTitle(tr("Modeler"));
    this->setAutoFillBackground(true);
    this->setUpGUI();
}

void MainContainer::setApp(ModelerApp* app) {
    this->app = app;
    this->app->getCoreScene().onSceneUpdated([this](Core::WeakPointer<Core::Object3D> object){
        this->refreshSceneTree();
    });
}

void MainContainer::setUpGUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QFrame *loadModelFrame = this->buildLoadModelGUI();
    QHBoxLayout *lowerLayout = new QHBoxLayout;
    mainLayout->addWidget(loadModelFrame);

    this->sceneTree = new QTreeWidget;
    sceneTree->setColumnCount(1);
    sceneTree->setHeaderHidden(true);

    lowerLayout->addWidget(sceneTree);
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

void MainContainer::setModelSmoothingThresholdEditText(float angle) {
    std::ostringstream ss;
    ss << angle;
    std::string scaleAngle(ss.str());
    this->modelSmoothingThresholdEdit->setText(QString::fromStdString(scaleAngle));
}

void MainContainer::setModelZUpCheck(bool checked) {
    this->modelZUpCheckBox->setChecked(checked);
}

void MainContainer::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(e);
}

void MainContainer::populateSceneTree(QTreeWidget* sceneTree, QTreeWidgetItem* parentItem, Core::WeakPointer<Core::Object3D> object) {

    QTreeWidgetItem* childItem = new QTreeWidgetItem();
    childItem->setText(0, QString::fromStdString(object->getName()));

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
    this->sceneTree->clear();
    for (Core::SceneObjectIterator<Core::Object3D> itr = sceneRoot->beginIterateChildren(); itr != sceneRoot->endIterateChildren(); ++itr) {
        Core::WeakPointer<Core::Object3D> childObject = *itr;
        this->populateSceneTree(this->sceneTree, nullptr, childObject);
    }
}
