#pragma once

#include "Scene/Scene.h"
#include "CoreScene.h"
#include "ModelerApp.h"

#include "Core/common/types.h"
#include "Core/util/WeakPointer.h"
#include "Core/Engine.h"
#include "Core/render/Camera.h"
#include "Core/scene/Scene.h"

class CornfieldScene : public Scene
{
public:
    CornfieldScene();
    void setupScene(Core::WeakPointer<Core::Engine> engine, ModelerApp& modelerApp, CoreScene& coreScene,
                    Core::WeakPointer<Core::Camera> renderCamera);
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
