#pragma once

#include <vector>

#include "Scene/ModelerScene.h"
#include "CoreScene.h"
#include "ModelerApp.h"
#include "FlickerLight.h"

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
    void setupCommonSceneElements();
    void setupUniqueSceneElements();
     void setupBaseLights();
    FlickerLight createTorchFlame(Core::WeakPointer<Core::Engine> engine, CoreScene& coreScene, Core::Atlas& emberAtlas, Core::Atlas& fire2Atlas, Core::Atlas& fire4FlatAtlas,
                                  float x, float y, float z, float scale, float lightRadius, float lightIntensity);
    void createEmberParticleSystem(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Object3D> parent, Core::Atlas& atlas, float scale = 1.0f);
    void createFire2ParticleSystem(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Object3D> parent, Core::Atlas& atlas, float scale = 1.0f);
    void createFire4ParticleSystem(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Object3D> parent, Core::Atlas& atlas, float scale = 1.0f);

    unsigned int frameCount;
    Core::WeakPointer<Core::Object3D> ambientLightObject;
    Core::WeakPointer<Core::Object3D> directionalLightObject;
    Core::WeakPointer<Core::Object3D>  pointLightObject;
    std::vector<FlickerLight> flickerLights;
};
