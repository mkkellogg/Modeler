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
    QString modelScaleEditStyle = QString("QLineEdit {width: 25px;}");
    this->modelScaleEdit->setStyleSheet(modelScaleEditStyle);

    this->modelSmoothingThresholdEdit = new QLineEdit;
    this->modelSmoothingThresholdEdit->setObjectName("modelSmoothingThresholdEdit");
    QString modelSmoothingThresholdEditStyle = QString("QLineEdit {width: 25px;}");
    this->modelSmoothingThresholdEdit->setStyleSheet(modelSmoothingThresholdEditStyle);

    QLabel* loadLabel = new QLabel("Load model: ");
    QLabel* scaleLabel = new QLabel("Model scale: ");
    QLabel* smoothingLabel = new QLabel("Smoothing threshhold: ");
    QPushButton *browseButton = new QPushButton(this);
    connect(browseButton, SIGNAL(clicked()), SLOT(browseForModel()));
    browseButton->setText(tr("Browse"));
    QPushButton *loadButton = new QPushButton(this);
    loadButton->setText(tr("Load"));

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(loadLabel);
    horizontalLayout->addWidget(this->modelNameEdit);
    horizontalLayout->addWidget(browseButton);
    horizontalLayout->addWidget(scaleLabel);
    horizontalLayout->addWidget(this->modelScaleEdit);
    horizontalLayout->addWidget(smoothingLabel);
    horizontalLayout->addWidget(this->modelSmoothingThresholdEdit);
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
        //if (directoryComboBox->findText(directory) == -1)
        //    directoryComboBox->addItem(directory);
        //directoryComboBox->setCurrentIndex(directoryComboBox->findText(directory));
        this->modelNameEdit->setText(selectedFiles.at(0));
    }
}

RenderWindow* MainContainer::getRenderWindow() {
    return this->renderWindow;
}

void MainContainer::keyPressEvent(QKeyEvent *e) {
    if (e->key() == Qt::Key_Escape)
        close();
    else
        QWidget::keyPressEvent(e);
}
