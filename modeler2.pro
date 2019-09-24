# Set these to the appropriate directories
CORE_BUILD_DIR=$$PWD/../core/build
ASSIMP_BUILD_DIR=$$PWD/../assimp/build
ASSIMP_SRC_DIR=$$PWD/../assimp/build
DEVIL_BUILD_DIR=$$PWD/../devil/devil-src/DevIL/build

HEADERS       = \
    ModelerApp.h \
    RenderWindow.h \
    OpenGLMouseAdapterWidget.h \
    MouseAdapter.h \
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
    Scene/CornfieldScene.h \
    Scene/RedSkyScene.h \
    Scene/ModelerScene.h \
    Scene/SceneHelper.h \
    Util/FileUtil.h
SOURCES       = \
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
    Scene/CornfieldScene.cpp \
    Scene/RedSkyScene.cpp \
    Scene/ModelerScene.cpp \
    Scene/SceneHelper.cpp \
    Util/FileUtil.cpp

QT           += widgets


DEFINES += GL_GLEXT_PROTOTYPES
CONFIG += c++11

win32:CONFIG(release, debug|release): LIBS += -L$$CORE_BUILD_DIR/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$CORE_BUILD_DIR/debug/ -lcore
else:unix: LIBS += -L$$CORE_BUILD_DIR -lcore

INCLUDEPATH += $$CORE_BUILD_DIR/include/
DEPENDPATH += $$CORE_BUILD_DIR/include/

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$CORE_BUILD_DIR/release/libcore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$CORE_BUILD_DIR/debug/libcore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$CORE_BUILD_DIR/release/core.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$CORE_BUILD_DIR/debug/core.lib
else:unix: PRE_TARGETDEPS += $$CORE_BUILD_DIR/libcore.a

win32:CONFIG(release, debug|release): LIBS += -L$$ASSIMP_BUILD_DIR/code/release/ -lassimp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$ASSIMP_BUILD_DIR/code/ -lassimp
else:unix: LIBS += -L$$ASSIMP_BUILD_DIR/code/ -lassimp

INCLUDEPATH += $$ASSIMP_SRC_DIR/include
DEPENDPATH += $ASSIMP_SRC_DIR/include

win32:CONFIG(release, debug|release): LIBS += -L$$DEVIL_BUILD_DIR/lib/x64/ -lIL
else:win32:CONFIG(debug, debug|release): LIBS += -L$$DEVIL_BUILD_DIR/lib/x64/ -lIL
else:unix: LIBS += -L$$DEVIL_BUILD_DIR/lib/x64/ -lIL



