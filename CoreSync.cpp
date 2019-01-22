#include "CoreSync.h"
#include "RenderWindow.h"

CoreSync::CoreSync(RenderWindow* renderWindow): renderWindow(renderWindow) {

}

CoreSync::~CoreSync() {

}

void CoreSync::run(Runnable runnable) {
    RenderWindow::LifeCycleEventCallback temp = [this, runnable](RenderWindow* renderWindow) {
        runnable(this->renderWindow->getEngine());
    };
    renderWindow->onUpdate(temp, true);
}

