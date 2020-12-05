# Set these to the appropriate directories
CORE_BINARY_DIR=$$PWD/../../Core/build
ASSIMP_BINARY_DIR=$$PWD/../../assimp/build/bin
#DEVIL_BINARY_DIR=$$PWD/../devil/devil-src/DevIL/build

HEADERS       = \
    ModelerApp.h \
    RenderWindow.h \
    OpenGLMouseAdapterWidget.h \
    MouseAdapter.h \
    Scene/MoonlitNightScene.h \
    Scene/SunnySkyScene.h \
    Scene/SunriseScene.h \
    Scene/SunsetScene.h \
    Settings.h \
    PipedEventAdapter.h \
    CoreSync.h \
    GestureAdapter.h \
    OrbitControls.h \
    MainWindow.h \
    CoreScene.h \
    Exception.h \
    MainGUI.h \
    BasicRimShadowMaterial.h \
    TransformWidget.h \
    SceneTreeWidget.h \
    KeyboardAdapter.h \
    SceneUtils.h \
    Scene/ModelerScene.h \
    Scene/SceneHelper.h \
    Util/FileUtil.h
SOURCES       = \
    Scene/MoonlitNightScene.cpp \
    Scene/SunnySkyScene.cpp \
    Scene/SunriseScene.cpp \
    Scene/SunsetScene.cpp \
    main.cpp \
    ModelerApp.cpp \
    RenderWindow.cpp \
    MainWindow.cpp \
    OpenGLMouseAdapterWidget.cpp \
    MouseAdapter.cpp \
    Settings.cpp \
    CoreSync.cpp \
    GestureAdapter.cpp \
    OrbitControls.cpp \
    CoreScene.cpp \
    Exception.cpp \
    MainGUI.cpp \
    BasicRimShadowMaterial.cpp \
    TransformWidget.cpp \
    SceneTreeWidget.cpp \
    KeyboardAdapter.cpp \
    SceneUtils.cpp \
    Scene/ModelerScene.cpp \
    Scene/SceneHelper.cpp \
    Util/FileUtil.cpp

QT           += widgets


DEFINES += GL_GLEXT_PROTOTYPES
CONFIG += c++11

INCLUDEPATH += $$CORE_BINARY_DIR/include/
DEPENDPATH += $$CORE_BINARY_DIR/include/

LIBS += -L$$CORE_BINARY_DIR/ -lcore
PRE_TARGETDEPS += $$CORE_BINARY_DIR/libcore.a
# For windows, the follow line may need to be uncommented
# PRE_TARGETDEPS += $$CORE_BINARY_DIR/core.lib

LIBS += -L$$ASSIMP_BINARY_DIR
# If you have a custom DevIL location, uncomment the line below
#LIBS += -L$$DEVIL_BINARY_DIR/lib/x64/
LIBS += -lassimp
LIBS += -lIL



