#include <sstream>

#include "RenderWindow.h"
#include "MainContainer.h"
#include "MainWindow.h"

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

MainContainer::MainContainer(MainWindow *mw): mainWindow(mw) {
    this->renderWindow = new RenderWindow;

    QFrame* hFrame = new QFrame(this);
    hFrame->setObjectName("loadModelFrame");
    QString hFrameStyle = QString("#loadModelFrame {border: 1px solid #aaa;}");
    hFrame->setStyleSheet(hFrameStyle);

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

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(loadLabel);
    horizontalLayout->addWidget(this->modelNameEdit);
    horizontalLayout->addWidget(browseButton);
    horizontalLayout->addWidget(scaleLabel);
    horizontalLayout->addWidget(this->modelScaleEdit);
    horizontalLayout->addWidget(smoothingLabel);
    horizontalLayout->addWidget(this->modelSmoothingThresholdEdit);
    horizontalLayout->addWidget(zUpLabel);
    horizontalLayout->addWidget(this->modelZUpCheckBox);
    horizontalLayout->addWidget(loadButton);
    hFrame->setLayout(horizontalLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(hFrame);
    mainLayout->addWidget(this->renderWindow);
    setLayout(mainLayout);

    setWindowTitle(tr("Modeler"));
    this->setAutoFillBackground(true);
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
    if (this->loadModelClickedCallback) {
        this->loadModelClickedCallback(nameQStr.toStdString(), scaleQStr.toStdString(), smoothingThresholdQStr.toStdString(), zUp);
    }
}

RenderWindow* MainContainer::getRenderWindow() {
    return this->renderWindow;
}

void MainContainer::setScaleEditText(float scale) {
    std::ostringstream ss;
    ss << scale;
    std::string scaleText(ss.str());
    this->modelScaleEdit->setText(QString::fromStdString(scaleText));
}

void MainContainer::setSmoothingThresholdEditText(float angle) {
    std::ostringstream ss;
    ss << angle;
    std::string scaleAngle(ss.str());
    this->modelSmoothingThresholdEdit->setText(QString::fromStdString(scaleAngle));
}

void MainContainer::setZUpCheck(bool checked) {
    this->modelZUpCheckBox->setChecked(checked);
}

void MainContainer::onLoadModelClicked(LoadModelClickedCallback callback) {
   this->loadModelClickedCallback = callback;
}

void MainContainer::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(e);
}
