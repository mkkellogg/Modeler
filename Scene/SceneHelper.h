#pragma once

#include "Core/Engine.h"

class ModelerApp;

class SceneHelper
{
public:
    SceneHelper(ModelerApp& modelerApp);
    Core::WeakPointer<Core::ReflectionProbe> createSkyboxReflectionProbe(float x, float y, float z);
    void loadModelStandard(const std::string& path, bool usePhysicalMaterial, bool overrideLoadedTransform, float ex, float ey, float ez,
                           float rx, float ry, float rz, float ra, float tx, float ty, float tz, float scaleX, float scaleY, float scaleZ,
                           bool singlePassMultiLight, float metallic, float roughness, bool transparent, unsigned int enabledAlphaChannel,
                           bool doubleSided, bool customShadowRendering, bool castShadows, std::function<void(Core::WeakPointer<Core::Object3D>)> onLoad, int layer);
    void loadTerrain(bool usePhysicalMaterial, float rotation);
    void loadWarrior(bool usePhysicalMaterial, float rotation, float x, float y, float z);
    void createBasePlatform();
    void createDemoSpheres();
    void setupCommonSceneElements(bool excludeCastle, bool physicalTerain);

private:
    ModelerApp& modelerApp;
    Core::WeakPointer<Core::ReflectionProbe> centerProbe;
};
