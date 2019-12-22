#pragma once

#include "Scene/ModelerScene.h"
#include "CoreScene.h"
#include "ModelerApp.h"

#include "Core/common/types.h"
#include "Core/util/WeakPointer.h"
#include "Core/Engine.h"
#include "Core/render/Camera.h"
#include "Core/scene/Scene.h"
#include "Core/light/AmbientLight.h"

class CornfieldScene : public ModelerScene
{
public:
    CornfieldScene(ModelerApp& modelerApp);
    ~CornfieldScene();

    void load() override;
    void update() override;

private:
    void setupSkyboxes();
    void setupDefaultObjects();
    void setupLights();

    unsigned int frameCount;
    Core::WeakPointer<Core::Object3D> ambientLightObject;
    Core::WeakPointer<Core::Object3D> directionalLightObject;
    Core::WeakPointer<Core::MeshContainer>  pointLightObject;
    Core::WeakPointer<Core::ReflectionProbe> centerProbe;
};
