HEADERS       = \
    ModelerApp.h \
    MainContainer.h \
    RenderWindow.h \
    OpenGLMouseAdapterWidget.h \
    MouseAdapter.h \
    Settings.h \
    PipedEventAdapter.h \
    CoreSync.h \
    GestureAdapter.h \
    OrbitControls.h \
    MainWindow.h
SOURCES       = \
                main.cpp \
    ModelerApp.cpp \
    MainContainer.cpp \
    RenderWindow.cpp \
    MainWindow.cpp \
    OpenGLMouseAdapterWidget.cpp \
    MouseAdapter.cpp \
    Settings.cpp \
    CoreSync.cpp \
    GestureAdapter.cpp \
    OrbitControls.cpp

QT           += widgets


DEFINES += GL_GLEXT_PROTOTYPES


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../Core/build/release/ -lcore
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../Core/build/debug/ -lcore
else:unix: LIBS += -L$$PWD/../../Core/build/ -lcore

INCLUDEPATH += $$PWD/../../Core/build/include/
DEPENDPATH += $$PWD/../../Core/build/include/

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../Core/build/release/libcore.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../Core/build/debug/libcore.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$PWD/../../Core/build/release/core.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$PWD/../../Core/build/debug/core.lib
else:unix: PRE_TARGETDEPS += $$PWD/../../Core/build/libcore.a




win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../assimp_dynamic/build/code/release/ -lassimp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../assimp_dynamic/build/code/ -lassimp
else:unix: LIBS += -L/home/mark/Development/assimp_dynamic/build/code/ -lassimp

INCLUDEPATH += $$PWD/../../assimp_dynamic/include
DEPENDPATH += $$PWD/../../assimp_dynamic/include




win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../../DevIL/DevIL/build/lib/x64/ -lIL
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../../DevIL/DevIL/build/lib/x64/ -lIL
else:unix: LIBS += -L/home/mark/Development//DevIL/DevIL/build/lib/x64/ -lIL



