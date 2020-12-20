#pragma once

#include "Scene/ModelerScene.h"
#include "CoreScene.h"
#include "ModelerApp.h"

#include "Core/common/types.h"
#include "Core/util/WeakPointer.h"
#include "Core/Engine.h"
#include "Core/render/Camera.h"
#include "Core/scene/Scene.h"
#include "Core/image/Atlas.h"

class MoonlitNightScene : public ModelerScene
{
public:
    MoonlitNightScene(ModelerApp& modelerApp);
    void load() override;
    void update() override;

private:
    void setupSkyboxes();
    void setupDefaultObjects();
    void setupLights();
    void createFire2ParticleSystem(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Object3D> parent, Core::Atlas& atlas);
    void createFire4ParticleSystem(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Object3D> parent, Core::Atlas& atlas);

    unsigned int frameCount;
    Core::WeakPointer<Core::Object3D> ambientLightObject;
    Core::WeakPointer<Core::Object3D> directionalLightObject;
    Core::WeakPointer<Core::Object3D>  pointLightObject;
};
