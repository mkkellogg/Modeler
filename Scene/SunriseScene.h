#pragma once

#include "Scene/ModelerScene.h"
#include "CoreScene.h"
#include "ModelerApp.h"

#include "Core/common/types.h"
#include "Core/util/WeakPointer.h"
#include "Core/Engine.h"
#include "Core/render/Camera.h"
#include "Core/scene/Scene.h"

class SunriseScene : public ModelerScene
{
public:
    SunriseScene(ModelerApp& modelerApp);
    void load() override;
    void update() override;

private:
    void setupSkyboxes();
    void setupDefaultObjects();
    void setupLights();

    unsigned int frameCount;
    Core::WeakPointer<Core::Object3D> ambientLightObject;
    Core::WeakPointer<Core::Object3D> directionalLightObject;
    Core::WeakPointer<Core::Object3D>  pointLightObject;
};
