#include "FlickerLight.h"
#include "Core/util/Time.h"

FlickerLight::FlickerLight() {
    this->lastIntensityFlickerTime = Core::Time::getTime();
    this->lastPositionFlickerTime = Core::Time::getTime();
    this->lastIntensityAdjuster = 1.0f;
    this->nextIntensityAdjuster = 1.0f;
    this->intensity = 1.0f;
}

FlickerLight::FlickerLight(const FlickerLight& src) {
    this->owner = src.owner;
    this->light = src.light;
    this->intensity = src.intensity;
    this->lastIntensityFlickerTime = src.lastIntensityFlickerTime;
    this->lastPositionFlickerTime = src.lastPositionFlickerTime;
    this->lastIntensityAdjuster = src.lastIntensityAdjuster;
    this->nextIntensityAdjuster = src.nextIntensityAdjuster;
    this->lastPositionAdjuster = src.lastPositionAdjuster;
}

FlickerLight& FlickerLight::operator = (const FlickerLight& other) {
    if (&other == this) return *this;
    this->owner = other.owner;
    this->light = other.light;
    this->intensity = other.intensity;
    this->lastIntensityFlickerTime = other.lastIntensityFlickerTime;
    this->lastPositionFlickerTime = other.lastPositionFlickerTime;
    this->lastIntensityAdjuster = other.lastIntensityAdjuster;
    this->nextIntensityAdjuster = other.nextIntensityAdjuster;
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

    Core::Real elapsedTimeSinceLastIntensityFlicker = time - this->lastIntensityFlickerTime;
    Core::Real flickerIntensityIntervalsPerSecond = 8.0f;
    Core::Real flickerIntensityIntervalLength = 1.0f / flickerIntensityIntervalsPerSecond;
    Core::Real perUpdateIntervalIntensityFluxRange = 2.0f;
    Core::Real intensityFactorRangeLowerBound = 0.25f;
    Core::Real intensityFactorRangeUpperBound = 1.5f;

    if (elapsedTimeSinceLastIntensityFlicker > flickerIntensityIntervalLength) {
        this->lastIntensityFlickerTime = time;

        Core::Real intensityDiff = (Core::Math::random() - 0.5f) * 2.0f * perUpdateIntervalIntensityFluxRange * flickerIntensityIntervalLength;

        Core::Real intensityAdjuster = 1.0f + intensityDiff;
        Core::Real diff = (intensityAdjuster - this->lastIntensityAdjuster);
        intensityAdjuster = this->lastIntensityAdjuster + diff;

        this->lastIntensityAdjuster = this->nextIntensityAdjuster;
        this->nextIntensityAdjuster = Core::Math::clamp(intensityAdjuster, intensityFactorRangeLowerBound, intensityFactorRangeUpperBound);

    } else {
        Core::Real elapsedFlickerIntensityT = elapsedTimeSinceLastIntensityFlicker / flickerIntensityIntervalLength;
        Core::Real intensityAdjuster = (1.0f - elapsedFlickerIntensityT) * this->lastIntensityAdjuster + elapsedFlickerIntensityT * this->nextIntensityAdjuster;
        this->light->setIntensity(intensityAdjuster * this->intensity);
    }


    Core::Real elapsedTimeSinceLastPositionFlicker = time - this->lastPositionFlickerTime;
    Core::Real flickerPositionIntervalsPerSecond = 16.0f;
    Core::Real flickerPositionIntervalLength = 1.0f / flickerPositionIntervalsPerSecond;

    if (elapsedTimeSinceLastPositionFlicker > flickerPositionIntervalLength) {
        this->lastPositionFlickerTime = time;

        Core::Real deltaTime = Core::Time::getDeltaTime();
        Core::Vector3r positionAdjuster(Core::Math::random() - 0.5f, Core::Math::random() - 0.5f, Core::Math::random() - 0.5f);
        positionAdjuster.scale(deltaTime);

        this->owner->getTransform().getLocalMatrix().setIdentity();
        //this->owner->getTransform().translate(campFireLightLocalOffset, true);

        positionAdjuster.add(this->lastPositionAdjuster);
        positionAdjuster.scale(0.5f);

        this->owner->getTransform().translate(positionAdjuster);

        this->lastPositionAdjuster = positionAdjuster;
    }
}
