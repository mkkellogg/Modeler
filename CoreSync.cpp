#include "Core/Engine.h"

#include "CoreSync.h"

CoreSync::CoreSync() {

}

CoreSync::~CoreSync() {

}

void CoreSync::run(Runnable runnable) {
    Core::Engine::LifecycleEventCallback temp = [this, runnable]() {
        runnable(Core::Engine::instance());
    };
    Core::Engine::instance()->onUpdate(temp, false);
}
