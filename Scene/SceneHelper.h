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
    void loadWarrior(bool usePhysicalMaterial, float rotation, float x, float y, float z);
    void createBasePlatform();
    void createDemoSpheres();

private:
    ModelerApp& modelerApp;
};
