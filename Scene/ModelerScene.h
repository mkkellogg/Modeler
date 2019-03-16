#pragma once

#include "Core/Engine.h"
#include "Core/render/Camera.h"

class ModelerApp;
class CoreScene;

class ModelerScene {
public:
    ModelerScene();
    virtual void load(Core::WeakPointer<Core::Engine> engine, ModelerApp& modelerApp, CoreScene& coreScene,
                      Core::WeakPointer<Core::Camera> renderCamera) = 0;
    virtual void unload() = 0;
    virtual void update() = 0;
};
