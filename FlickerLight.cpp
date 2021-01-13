#include "FlickerLight.h"
#include "Core/util/Time.h"

FlickerLight::FlickerLight() {
    this->lastFlickerTime = Core::Time::getTime();
    this->lastIntensityAdjuster = 1.0f;
    this->intensity = 1.0f;
}

FlickerLight::FlickerLight(const FlickerLight& src) {
    this->owner = src.owner;
    this->light = src.light;
    this->intensity = src.intensity;
    this->lastFlickerTime = src.lastFlickerTime;
    this->lastIntensityAdjuster = src.lastIntensityAdjuster;
    this->lastPositionAdjuster = src.lastPositionAdjuster;
}

FlickerLight& FlickerLight::operator = (const FlickerLight& other) {
    if (&other == this) return *this;
    this->owner = other.owner;
    this->light = other.light;
    this->intensity = other.intensity;
    this->lastFlickerTime = other.lastFlickerTime;
    this->lastIntensityAdjuster = other.lastIntensityAdjuster;
    this->lastPositionAdjuster = other.lastPositionAdjuster;
    return *this;
}

Core::WeakPointer<Core::PointLight> FlickerLight::create(Core::WeakPointer<Core::Object3D> parent, Core::Bool shadowsEnabled,
                                                         Core::UInt32 shadowMapSize, Core::Real constantShadowBias, Core::Real angularShadowBias) {
    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();
    this->owner = engine->createObject3D();
    parent->addChild(this->owner);
    this->owner->getTransform().setLocalPosition(0.0f, 0.0f, 0.0f);
    this->light = engine->createPointLight<Core::PointLight>(this->owner, shadowsEnabled, shadowMapSize, constantShadowBias, angularShadowBias);
    return this->light;
}

Core::WeakPointer<Core::PointLight> FlickerLight::getLight() {
    return this->light;
}

void FlickerLight::setIntensity(Core::Real intensity) {
    this->intensity = intensity;
}

void FlickerLight::update() {
    Core::Real time = Core::Time::getTime();
    Core::Real elapsedTime = time - this->lastFlickerTime;

    if (elapsedTime > 1.0f / 30.0f) {
        lastFlickerTime = time;

        Core::Real intensityAdjuster = (Core::Math::random() - 0.5f);
        Core::Vector3r positionAdjuster(Core::Math::random() - 0.5f, Core::Math::random() - 0.5f, Core::Math::random() - 0.5f);

        Core::Real deltaTime = Core::Time::getDeltaTime();
        intensityAdjuster *= deltaTime * 220.0f;
        positionAdjuster.scale(deltaTime * 1.0f);

        Core::Real diff = (intensityAdjuster - lastIntensityAdjuster) * 0.05f;
        intensityAdjuster = Core::Math::clamp(lastIntensityAdjuster + diff, .75f, 1.25f);

        this->light->setIntensity(intensityAdjuster * this->intensity);

        lastIntensityAdjuster = intensityAdjuster;

        this->owner->getTransform().getLocalMatrix().setIdentity();
        //this->owner->getTransform().translate(campFireLightLocalOffset, true);

        positionAdjuster.add(this->lastPositionAdjuster);
        positionAdjuster.scale(0.5f);

        this->owner->getTransform().translate(positionAdjuster);

        this->lastPositionAdjuster = positionAdjuster;
    }
}
