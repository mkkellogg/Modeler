#pragma once

#include "Scene/ModelerScene.h"
#include "CoreScene.h"
#include "ModelerApp.h"

#include "Core/common/types.h"
#include "Core/util/WeakPointer.h"
#include "Core/Engine.h"
#include "Core/render/Camera.h"
#include "Core/scene/Scene.h"

class CornfieldScene : public ModelerScene
{
public:
    CornfieldScene();
    void load(Core::WeakPointer<Core::Engine> engine, ModelerApp& modelerApp, CoreScene& coreScene,
              Core::WeakPointer<Core::Camera> renderCamera) override;
    void unload() override;
    void update() override;

private:
    void setupSkyboxes(Core::WeakPointer<Core::Camera> renderCamera);
    void setupDefaultObjects(Core::WeakPointer<Core::Camera> renderCamera);
    void setupLights();

    unsigned int frameCount;
    CoreScene * coreScene;
    ModelerApp * modelerApp;
    Core::WeakPointer<Core::Engine> engine;
    Core::WeakPointer<Core::Object3D> ambientLightObject;
    Core::WeakPointer<Core::Object3D> directionalLightObject;
    Core::WeakPointer<Core::RenderableContainer<Core::Mesh>>  pointLightObject;
    Core::WeakPointer<Core::ReflectionProbe> centerProbe;
};
