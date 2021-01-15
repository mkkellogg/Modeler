#pragma once

#include "Core/Engine.h"
#include "Core/scene/Object3D.h"
#include "Core/light/PointLight.h"
#include "Core/geometry/Vector3.h"

class FlickerLight
{
public:
    FlickerLight();
    FlickerLight(const FlickerLight& src);
    FlickerLight& operator = (const FlickerLight& other);
    Core::WeakPointer<Core::PointLight> create(Core::WeakPointer<Core::Object3D> parent, Core::Bool shadowsEnabled,
                                               Core::UInt32 shadowMapSize, Core::Real constantShadowBias, Core::Real angularShadowBias);
    Core::WeakPointer<Core::PointLight> getLight();
    void setIntensity(Core::Real intensity);
    void update();

private:

     Core::WeakPointer<Core::Object3D> owner;
     Core::WeakPointer<Core::PointLight> light;

     Core::Real lastIntensityFlickerTime;
     Core::Real lastPositionFlickerTime;

     Core::Real lastIntensityAdjuster;
     Core::Real nextIntensityAdjuster;
     Core::Real intensity;
     Core::Vector3r lastPositionAdjuster;
};
