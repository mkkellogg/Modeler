#pragma once

#include "SceneHelper.h"

#include "Core/Engine.h"
#include "Core/render/Camera.h"

class ModelerApp;
class CoreScene;

class ModelerScene {
public:
    ModelerScene(ModelerApp& modelerApp);
    ~ModelerScene();

    virtual void load();
    virtual void update() = 0;

protected:
    ModelerApp& modelerApp;
    SceneHelper sceneHelper;

private:
    bool loadComplete;
};
