#pragma once

#include "CoreScene.h"

#include "Core/common/types.h"
#include "Core/util/WeakPointer.h"
#include "Core/Engine.h"
#include "Core/render/Camera.h"
#include "Core/scene/Scene.h"

class CornfieldScene
{
public:
    CornfieldScene();
    void setupScene(Core::WeakPointer<Core::Engine> engine, CoreScene& coreScene,
                    Core::WeakPointer<Core::Camera> renderCamera);
private:
    void setupSkyboxes(Core::WeakPointer<Core::Camera> renderCamera);
    void setupDefaultObjects();
    void setupLights();

    CoreScene * coreScene;
    Core::WeakPointer<Core::Engine> engine;
    Core::WeakPointer<Core::Object3D> ambientLightObject;
    Core::WeakPointer<Core::Object3D> directionalLightObject;
    Core::WeakPointer<Core::RenderableContainer<Core::Mesh>>  pointLightObject;
    Core::WeakPointer<Core::ReflectionProbe> centerProbe;
};
