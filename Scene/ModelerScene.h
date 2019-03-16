#pragma once

#include "Core/Engine.h"
#include "Core/render/Camera.h"

class ModelerApp;
class CoreScene;

class ModelerScene {
public:
    ModelerScene(ModelerApp& modelerApp);
    virtual void load();
    virtual void update() = 0;

protected:
    ModelerApp& modelerApp;

private:
    bool loadComplete;
};
