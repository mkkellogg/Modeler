#pragma once

#include "Core/Engine.h"

class ModelerApp;

class SceneHelper
{
public:
    SceneHelper(ModelerApp& modelerApp);
    Core::WeakPointer<Core::ReflectionProbe> createSkyboxReflectionProbe(float x, float y, float z);
    void loadGun(float rotation, float x, float y, float z);
    void loadHouse(bool usePhysicalMaterial, float rotation, float x, float y, float z);
    void loadModelStandard(const std::string& path, bool usePhysicalMaterial, float rotation, float x, float y, float , float scale,
                           bool singlePassMultiLight, float metallic, float roughness, bool transparent, bool customShadowRendering);
    void loadTerrain(bool usePhysicalMaterial, float rotation, float x, float y, float z);
    void loadWarrior(bool usePhysicalMaterial, float rotation, float x, float y, float z);
    void createBasePlatform();
    void createDemoSpheres();
    void setupCommonSceneElements();

private:
    ModelerApp& modelerApp;
    Core::WeakPointer<Core::ReflectionProbe> centerProbe;
};
