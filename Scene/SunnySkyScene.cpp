#include "SunnySkyScene.h"

#include "Core/image/TextureUtils.h"
#include "Core/image/Texture2D.h"
#include "Core/material/StandardPhysicalMaterial.h"
#include "Core/geometry/GeometryUtils.h"
#include "Core/geometry/Mesh.h"
#include "Core/render/RenderableContainer.h"
#include "Core/render/MeshRenderer.h"
#include "Core/render/RenderTargetCube.h"
#include "Core/render/RenderTarget2D.h"
#include "Core/light/AmbientIBLLight.h"
#include "Core/light/AmbientLight.h"
#include "Core/material/BasicMaterial.h"
#include "Core/material/BasicCubeMaterial.h"
#include "Core/util/Time.h"

#include <QDir>

SunnySkyScene::SunnySkyScene(ModelerApp& modelerApp): ModelerScene(modelerApp) {
    this->frameCount = 0;
    this->envSubType = EnvironmentSubType::Standard;
}

void SunnySkyScene::load() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();

    switch (this->envSubType) {
        case EnvironmentSubType::Standard:
            renderCamera->setHDREnabled(true);
            //renderCamera->setHDRToneMapTypeReinhard();
            renderCamera->setHDRToneMapTypeExposure(2.0f);
            renderCamera->setHDRGamma(2.0f);
        break;
        case EnvironmentSubType::Alps:
            renderCamera->setHDREnabled(true);
            renderCamera->setHDRToneMapTypeExposure(3.0f);
            renderCamera->setHDRGamma(2.2);
        break;
    }

    Core::WeakPointer<Core::Object3D> cameraObj = renderCamera->getOwner();
    cameraObj->getTransform().translate(-20, 15, -30, Core::TransformationSpace::World);
    cameraObj->getTransform().updateWorldMatrix();
    cameraObj->getTransform().lookAt(Core::Point3r(0, 0, 0));

    this->setupSkyboxes();
    this->setupDefaultObjects();
    this->setupLights();
}

void SunnySkyScene::update() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    this->frameCount++;
}

void SunnySkyScene::setupSkyboxes() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();

    Core::TextureAttributes skyboxTextureAttributes;
    skyboxTextureAttributes.FilterMode = Core::TextureFilter::Linear;
    skyboxTextureAttributes.MipLevels = 2;
    Core::WeakPointer<Core::CubeTexture> skyboxTexture = engine->createCubeTexture(skyboxTextureAttributes);

    Core::WeakPointer<Core::CubeTexture> skyTexture;
    switch (this->envSubType) {
        case EnvironmentSubType::Standard:
            skyTexture = Core::TextureUtils::loadFromEquirectangularImage("assets/skyboxes/HDR/puresky1_4k.hdr", false);
            renderCamera->getSkybox().build(skyTexture, true, 2.0f);
        break;
        case EnvironmentSubType::Alps:
            skyTexture = Core::TextureUtils::loadFromEquirectangularImage("assets/skyboxes/HDR/alps_field_4k.hdr", true, Core::Math::PI * -0.85f);
            renderCamera->getSkybox().build(skyTexture, true, 3.0f);
        break;
    }

    renderCamera->setSkyboxEnabled(true);
}

void SunnySkyScene::setupDefaultObjects() {
    this->sceneHelper.setupCommonSceneElements(false, true);
}

void SunnySkyScene::setupLights() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    CoreScene& coreScene = this->modelerApp.getCoreScene();

    this->ambientLightObject = engine->createObject3D();
    this->ambientLightObject->setName("Ambient light");
    coreScene.addObjectToScene(ambientLightObject);
    Core::WeakPointer<Core::AmbientLight> ambientLight = engine->createLight<Core::AmbientLight>(ambientLightObject);
    ambientLight->setColor(0.25f, 0.25f, 0.25f, 1.0f);

    this->directionalLightObject = engine->createObject3D();
    this->directionalLightObject->setName("Directonal light");
    coreScene.addObjectToScene(directionalLightObject);
    Core::WeakPointer<Core::DirectionalLight> directionalLight = engine->createDirectionalLight<Core::DirectionalLight>(directionalLightObject, 3, true, 4096, 0.0001, 0.0005);
    switch (this->envSubType) {
        case EnvironmentSubType::Standard:
            directionalLight->setIntensity(5.0f);
        break;
        case EnvironmentSubType::Alps:
            directionalLight->setIntensity(5.0f);
        break;
    }

    directionalLight->setColor(1.0, 1.0, 0.75, 1.0f);
    directionalLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);


    Core::Vector3r lightVector;
    Core::WeakPointer<Core::CubeTexture> skyTexture;
    switch (this->envSubType) {
        case EnvironmentSubType::Standard:
            lightVector.set(-0.6f, -1.0f, -0.4);
        break;
        case EnvironmentSubType::Alps:
            lightVector.set(0.9f, -1.3f, 0.1);
        break;
    }

    Core::Vector3r offsetVector = lightVector;
    offsetVector = offsetVector * -1000.0f;
    this->directionalLightObject->getTransform().translate(offsetVector, Core::TransformationSpace::World);
    this->directionalLightObject->getTransform().lookAt(Core::Point3r(lightVector.x, lightVector.y, lightVector.z));
}
