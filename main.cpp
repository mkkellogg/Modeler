#include <QApplication>
#include <QDesktopWidget>
#include <QSurfaceFormat>
#include <QCommandLineParser>
#include <QCommandLineOption>

#include "RenderWindow.h"
#include "MainWindow.h"
#include "ModelerApp.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("Modeler");
    QCoreApplication::setOrganizationName("GhostTree");
    QCoreApplication::setApplicationVersion(QT_VERSION_STR);
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::applicationName());
    parser.addHelpOption();
    parser.addVersionOption();
    QCommandLineOption multipleSampleOption("multisample", "Multisampling");
    parser.addOption(multipleSampleOption);
    QCommandLineOption coreProfileOption("coreprofile", "Use core profile");
    parser.addOption(coreProfileOption);
   // QCommandLineOption transparentOption("transparent", "Transparent window");
   // parser.addOption(transparentOption);
    parser.process(app);

    QSurfaceFormat fmt;
    fmt.setStencilBufferSize(8);
    fmt.setDepthBufferSize(32);
    if (parser.isSet(multipleSampleOption)) {
        fmt.setSamples(4);
    }
    //if (parser.isSet(coreProfileOption)) {
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    //}
    QSurfaceFormat::setDefaultFormat(fmt);

    MainWindow mainWindow;

    ModelerApp* modelerApp = new ModelerApp;
    modelerApp->init();

    MainGUI * mainGUI = mainWindow.getMainGUI();
    mainGUI->setModelerApp(modelerApp);
    mainGUI->setQtApp(&app);
    RenderWindow * renderWindow = mainGUI->getRenderWindow();
    modelerApp->setRenderWindow(renderWindow);

    //RenderWindow::setTransparent(parser.isSet(transparentOption) && false);
    if (RenderWindow::isTransparent()) {
        mainWindow.setAttribute(Qt::WA_TranslucentBackground);
        mainWindow.setAttribute(Qt::WA_NoSystemBackground, false);
    }
    mainWindow.resize(mainWindow.sizeHint());
    int desktopArea = QApplication::desktop()->width() *
                     QApplication::desktop()->height();
    int widgetArea = mainWindow.width() * mainWindow.height();
    if (((float)widgetArea / (float)desktopArea) < 0.75f)
        mainWindow.show();
    else
        mainWindow.showMaximized();
    return app.exec();
}
