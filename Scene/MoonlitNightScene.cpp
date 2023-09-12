#include "MoonlitNightScene.h"

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
#include "Core/filesys/FileSystem.h"
#include "Core/particles/ParticleSystem.h"
#include "Core/particles/ParticleSequenceGroup.h"
#include "Core/particles/ParticleEmitter.h"
#include "Core/particles/initializer/BoxPositionInitializer.h"
#include "Core/particles/initializer/RandomVelocityInitializer.h"
#include "Core/particles/initializer/LifetimeInitializer.h"
#include "Core/particles/initializer/RotationInitializer.h"
#include "Core/particles/initializer/RotationalSpeedInitializer.h"
#include "Core/particles/initializer/SizeInitializer.h"
#include "Core/particles/initializer/SequenceInitializer.h"
#include "Core/particles/renderer/ParticleSystemAnimatedSpriteRenderer.h"
#include "Core/particles/renderer/ParticleSystemPointRenderer.h"
#include "Core/particles/material/ParticleStandardMaterial.h"
#include "Core/particles/operator/SequenceOperator.h"
#include "Core/particles/operator/OpacityInterpolatorOperator.h"
#include "Core/particles/operator/SizeInterpolatorOperator.h"
#include "Core/particles/operator/ColorInterpolatorOperator.h"
#include "Core/particles/operator/AccelerationOperator.h"
#include "Core/particles/util/RandomGenerator.h"
#include "Core/particles/util/SphereRandomGenerator.h"

#include <QDir>

MoonlitNightScene::MoonlitNightScene(ModelerApp& modelerApp): ModelerScene(modelerApp) {
    this->frameCount = 0;
}

void MoonlitNightScene::load() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();

    renderCamera->setHDREnabled(true);
    renderCamera->setHDRToneMapTypeExposure(1.5f);
    renderCamera->setHDRGamma(1.5f);

    Core::WeakPointer<Core::Object3D> cameraObj = renderCamera->getOwner();
    cameraObj->getTransform().translate(-20, 15, -30, Core::TransformationSpace::World);
    cameraObj->getTransform().updateWorldMatrix();
    cameraObj->getTransform().lookAt(Core::Point3r(0, 0, 0));

    this->setupSkyboxes();
    this->setupCommonSceneElements();
    this->setupUniqueSceneElements();
    this->setupBaseLights();
}

void MoonlitNightScene::update() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    for (unsigned int i = 0; i < this->flickerLights.size(); i++) {
        this->flickerLights[i].update();
    }
    this->frameCount++;
}

void MoonlitNightScene::setupSkyboxes() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();

    Core::TextureAttributes skyboxTextureAttributes;
    skyboxTextureAttributes.FilterMode = Core::TextureFilter::Linear;
    skyboxTextureAttributes.MipLevels = 2;
    Core::WeakPointer<Core::CubeTexture> skyboxTexture = engine->createCubeTexture(skyboxTextureAttributes);

   /* std::vector<std::shared_ptr<Core::StandardImage>> skyboxImages;
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/moonlit_night/nightsky_north.png", true, true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/moonlit_night/nightsky_south.png", true, true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/moonlit_night/nightsky_up.png", true, true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/moonlit_night/nightsky_down.png", true, true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/moonlit_night/nightsky_west.png", true, true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/moonlit_night/nightsky_east.png", true, true));
    skyboxTexture->buildFromImages(skyboxImages[0], skyboxImages[1], skyboxImages[2], skyboxImages[3], skyboxImages[4], skyboxImages[5]);*/

    skyboxTexture = Core::TextureUtils::loadFromEquirectangularImage("assets/skyboxes/HDR/puresky_night1_4k.hdr", true, Core::Math::PI * -2.05f);
    renderCamera->getSkybox().build(skyboxTexture, true, 1.5f);
    renderCamera->setSkyboxEnabled(true);
}

void MoonlitNightScene::setupCommonSceneElements() {
    this->sceneHelper.setupCommonSceneElements(true, true);
}

void MoonlitNightScene::setupBaseLights() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    CoreScene& coreScene = this->modelerApp.getCoreScene();

    // ambient light
    this->ambientLightObject = engine->createObject3D();
    this->ambientLightObject->setName("Ambient light");
    coreScene.addObjectToScene(ambientLightObject);
    Core::WeakPointer<Core::AmbientLight> ambientLight = engine->createLight<Core::AmbientLight>(ambientLightObject);
    ambientLight->setColor(0.25f, 0.25f, 0.25f, 1.0f);

    // directional moon light
    this->directionalLightObject = engine->createObject3D();
    this->directionalLightObject->setName("Directonal light");
    coreScene.addObjectToScene(directionalLightObject);
    Core::WeakPointer<Core::DirectionalLight> directionalLight = engine->createDirectionalLight<Core::DirectionalLight>(directionalLightObject, 3, true, 4096, 0.0001, 0.0005);
    directionalLight->setIntensity(2.5f);
    directionalLight->setColor(1.0, 1.0, 1.0, 1.0f);
    directionalLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    Core::Vector3r lightVector(-0.5f, -1.0f, -0.5f);
    Core::Vector3r offsetVector = lightVector;
    offsetVector = offsetVector * -1000.0f;
    this->directionalLightObject->getTransform().translate(offsetVector, Core::TransformationSpace::World);
    this->directionalLightObject->getTransform().lookAt(Core::Point3r(lightVector.x, lightVector.y, lightVector.z));

}

void MoonlitNightScene::setupUniqueSceneElements() {
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    CoreScene& coreScene = this->modelerApp.getCoreScene();
    std::function<void(Core::WeakPointer<Core::Object3D>)> dummyOnLoad = [](Core::WeakPointer<Core::Object3D> root){};

    // texture atlases for flame particles systems
    std::shared_ptr<Core::FileSystem> fileSystem = Core::FileSystem::getInstance();
    Core::TextureAttributes texAttributes;
    texAttributes.FilterMode = Core::TextureFilter::TriLinear;
    texAttributes.MipLevels = 4;
    texAttributes.WrapMode = Core::TextureWrap::Clamp;
    texAttributes.Format = Core::TextureFormat::RGBA8;

    // Ember atlas
    std::string emberTexturePath = fileSystem->fixupPathForLocalFilesystem("assets/textures/particle_glow_05.png");
    std::shared_ptr<Core::StandardImage> emberTextureImage = Core::ImageLoader::loadImageU(emberTexturePath, false, true);
    Core::WeakPointer<Core::Texture2D> emberTexture = Core::Engine::instance()->getGraphicsSystem()->createTexture2D(texAttributes);
    emberTexture->buildFromImage(emberTextureImage);
    Core::Atlas emberAtlas(emberTexture);
    emberAtlas.addTileArray(1, 0.0f, 0.0f, 1.0f, 1.0f);

    // Base flame atlas
    std::string baseFlameTexturePath = fileSystem->fixupPathForLocalFilesystem("assets/textures/fire_particle_2_half.png");
    std::shared_ptr<Core::StandardImage> baseFlameTextureImage;
    baseFlameTextureImage = Core::ImageLoader::loadImageU(baseFlameTexturePath, false, true);
    Core::WeakPointer<Core::Texture2D> baseFlameTexture = Core::Engine::instance()->getGraphicsSystem()->createTexture2D(texAttributes);
    baseFlameTexture->buildFromImage(baseFlameTextureImage);
    Core::Atlas baseFlameAtlas(baseFlameTexture);
    baseFlameAtlas.addTileArray(18, 0.0f, 0.0f, 128.0f / 1024.0f, 128.0f / 512.0f);

    // Bright flame atlas
    std::string brightFlameTexturePath = fileSystem->fixupPathForLocalFilesystem("assets/textures/fire_particle_4_flat_half.png");
    std::shared_ptr<Core::StandardImage> brightFlameTextureImage;
    brightFlameTextureImage = Core::ImageLoader::loadImageU(brightFlameTexturePath, false, true);
    Core::WeakPointer<Core::Texture2D> fire4Texture = Core::Engine::instance()->getGraphicsSystem()->createTexture2D(texAttributes);
    fire4Texture->buildFromImage(brightFlameTextureImage);
    Core::Atlas brightFlameAtlas(fire4Texture);
    brightFlameAtlas.addTileArray(16, 0.0f, 0.0f, 212.0f / 1024.0f, 256.0f / 1024.0f);

    Core::Real torchIntensity = 180.0f;
    const std::string torchPost("assets/models/cartoonTorch/cartoonTorch.fbx");
    const std::string campfire("assets/models/toonlevel/campfire/campfire01.fbx");

    // torch 1
    FlickerLight torch1FlickerLight = this->createTorchWithFlame(engine, coreScene, emberAtlas, baseFlameAtlas, brightFlameAtlas, 40.4505, 32.0f, -141.762f, 1.0f, 14.0f, torchIntensity, 6);
    this->flickerLights.push_back(torch1FlickerLight);
    Core::WeakPointer<Core::PointLight> torch1Light = torch1FlickerLight.getLight();
    Core::IntMask torch1LightCullingMask = torch1Light->getCullingMask();
    Core::IntMaskUtil::clearBit(&torch1LightCullingMask, 1);
    Core::IntMaskUtil::clearBit(&torch1LightCullingMask, 2);
    Core::IntMaskUtil::clearBit(&torch1LightCullingMask, 6);
    torch1Light->setCullingMask(torch1LightCullingMask);

    // torch 2
    FlickerLight torch2FlickerLight = this->createTorchWithFlame(engine, coreScene, emberAtlas, baseFlameAtlas, brightFlameAtlas, 51.0816f, 32.0f, -141.762f, 1.0f, 14.0f, torchIntensity, 6);
    this->flickerLights.push_back(torch2FlickerLight);
    Core::WeakPointer<Core::PointLight> torch2Light = torch2FlickerLight.getLight();
    Core::IntMask torch2LightCullingMask = torch2Light->getCullingMask();
    Core::IntMaskUtil::clearBit(&torch2LightCullingMask, 1);
    Core::IntMaskUtil::clearBit(&torch2LightCullingMask, 2);
    Core::IntMaskUtil::clearBit(&torch2LightCullingMask, 6);
    torch2Light->setCullingMask(torch2LightCullingMask);

    // campfire
    FlickerLight torch3FlickerLight = this->createTorchFlame(engine, coreScene, emberAtlas, baseFlameAtlas, brightFlameAtlas, 45.4915f, 28.405f, -164.412f, 2.0f, 10.0f, 230.0f);
    this->flickerLights.push_back(torch3FlickerLight);
    Core::WeakPointer<Core::PointLight> torch3Light = torch3FlickerLight.getLight();
    Core::IntMask torch3LightCullingMask = torch3Light->getCullingMask();
    Core::IntMaskUtil::clearBit(&torch3LightCullingMask, 1);
    Core::IntMaskUtil::clearBit(&torch3LightCullingMask, 4);
    Core::IntMaskUtil::clearBit(&torch3LightCullingMask, 6);
    torch3Light->setCullingMask(torch3LightCullingMask);
    torch3Light->getOwner()->getParent()->getTransform().translate(0.0f, 2.0f, 0.0f);
    this->sceneHelper.loadModelStandard(campfire, true, false, 0.0f, 0.0f, 0.0f, 0, 1, 0, 0, 45.4915f, 27.2334f, -164.412f, 0.5f, 0.5f, 0.5f, false, 0.0f, 0.85f, false, 0, false, false, false, dummyOnLoad, 2);

    // torch 4
    FlickerLight torch4FlickerLight = this->createTorchWithFlame(engine, coreScene, emberAtlas, baseFlameAtlas, brightFlameAtlas, 31.6682f, 30.9f, -169.04f, 1.0f, 14.0f, torchIntensity, 7);
    this->flickerLights.push_back(torch4FlickerLight);
    Core::WeakPointer<Core::PointLight> torch4Light = torch4FlickerLight.getLight();
    Core::IntMask torch4LightCullingMask = torch4Light->getCullingMask();
    Core::IntMaskUtil::clearBit(&torch4LightCullingMask, 1);
    Core::IntMaskUtil::clearBit(&torch4LightCullingMask, 7);
    Core::IntMaskUtil::clearBit(&torch4LightCullingMask, 6);
    torch4Light->setCullingMask(torch4LightCullingMask);

    // castle fort objects
    const std::string modularCastleTowerPath("assets/models/modular_castle_tower/modular_castle_tower.fbx");
    const std::string modularCastleWallGatePath("assets/models/modular_castle_wall_gate/modular_castle_wall_gate.fbx");
    const std::string modularCastleWallBottomPath("assets/models/modular_castle_wall_bottom/modular_castle_wall_bottom.fbx");
    const std::string modularCastleWallTopPath("assets/models/modular_castle_wall_top/modular_castle_wall_top.fbx");
    this->sceneHelper.loadModelStandard(modularCastleTowerPath, true, false, 0.0f, Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 38.7209, 27.1098f, -146.606f, 0.015f, 0.015f, 0.015f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 4);
    this->sceneHelper.loadModelStandard(modularCastleTowerPath, true, false, 0.0f, Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 52.6357, 27.1098f, -146.606f, 0.015f, 0.015f, 0.015f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 4);
    this->sceneHelper.loadModelStandard(modularCastleWallGatePath, true, false, 0.0f, 0.0f, 0.0f, 0, 1, 0, 0, 45.7019, 27.1098f, -146.371f, 0.015f, 0.015f, 0.015f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 4);

    this->sceneHelper.loadModelStandard(modularCastleWallBottomPath, true, false, 0.0f, -Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 38.7705f, 27.1098f, -153.457f, 0.015f, 0.015f, 0.015f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 2);
    this->sceneHelper.loadModelStandard(modularCastleWallTopPath, true, false, 0.0f, -Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 38.7705f, 27.1098f, -153.457f, 0.015f, 0.015f, 0.015f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 2);
    this->sceneHelper.loadModelStandard(modularCastleWallBottomPath, true, false, 0.0f, -Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 38.7705f, 27.1098f, -162.38f, 0.015f, 0.015f, 0.015f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 2);
    this->sceneHelper.loadModelStandard(modularCastleWallTopPath, true, false, 0.0f, -Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 38.7705f, 27.1098f, -162.38f, 0.015f, 0.015f, 0.015f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 2);

    this->sceneHelper.loadModelStandard(modularCastleWallBottomPath, true, false, 0.0f, Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 52.8551f, 27.1098f, -153.457f, 0.015f, 0.015f, 0.015f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 2);
    this->sceneHelper.loadModelStandard(modularCastleWallTopPath, true, false, 0.0f, Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 52.8551f, 27.1098f, -153.457f, 0.015f, 0.015f, 0.015f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 2);
    this->sceneHelper.loadModelStandard(modularCastleWallBottomPath, true, false, 0.0f, Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 52.8551f, 27.1098f, -162.38f, 0.015f, 0.015f, 0.015f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 2);
    this->sceneHelper.loadModelStandard(modularCastleWallTopPath, true, false, 0.0f, Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 52.8551f, 27.1098f, -162.38f, 0.015f, 0.015f, 0.015f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 2);

    this->sceneHelper.loadModelStandard(modularCastleTowerPath, true, false, 0.0f, Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 38.7209, 27.1098f, -170.606f, 0.02f, 0.02f, 0.02f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 2);
    this->sceneHelper.loadModelStandard(modularCastleTowerPath, true, false, 0.0f, Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 52.6357, 27.1098f, -170.606f, 0.02f, 0.02f, 0.02f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 2);
    this->sceneHelper.loadModelStandard(modularCastleWallGatePath, true, false, 0.0f, 0.0f, 0.0f, 0, 1, 0, 0, 45.7019, 27.1098f, -170.371f, 0.02f, 0.02f, 0.02f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 2);
}

FlickerLight MoonlitNightScene::createTorchWithFlame(Core::WeakPointer<Core::Engine> engine, CoreScene& coreScene, Core::Atlas& emberAtlas, Core::Atlas& baseFlameAtlas,
                                                    Core::Atlas& brightFlameAtlas, float x, float y, float z, float scale, float lightRadius, float lightIntensity, int torchPostLayer) {
    const std::string torchPost("assets/models/cartoonTorch/cartoonTorch.fbx");
    std::function<void(Core::WeakPointer<Core::Object3D>)> dummyOnLoad = [](Core::WeakPointer<Core::Object3D> root){};
    FlickerLight torchFlickerLight = this->createTorchFlame(engine, coreScene, emberAtlas, baseFlameAtlas, brightFlameAtlas, x, y, z, scale, lightRadius, lightIntensity);
    this->sceneHelper.loadModelStandard(torchPost, true, false, 0.0f, 0.0f, 0.0f, 0, 1, 0, 0, x, y - 5.6367f, z, 0.1f, 0.1f, 0.1f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, torchPostLayer);
    return torchFlickerLight;
}

FlickerLight MoonlitNightScene::createTorchFlame(Core::WeakPointer<Core::Engine> engine, CoreScene& coreScene, Core::Atlas& emberAtlas, Core::Atlas& baseFlameAtlas,
                                                 Core::Atlas& brightFlameAtlas, float x, float y, float z, float scale, float lightRadius, float lightIntensity) {
    Core::WeakPointer<Core::Object3D> torchParticleSystemObject = engine->createObject3D();
    coreScene.addObjectToScene(torchParticleSystemObject);
    Core::WeakPointer<Core::Object3D> emberObject =this->createEmberParticleSystem(engine, torchParticleSystemObject, emberAtlas, scale);
    Core::WeakPointer<Core::Object3D> baseFlameObject =this->createBaseFlameParticleSystem(engine, torchParticleSystemObject, baseFlameAtlas, scale);
    Core::WeakPointer<Core::Object3D> brightFlameObject = this->createBrightFlameParticleSystem(engine, torchParticleSystemObject, brightFlameAtlas, scale);
    //brightFlameObject->getTransform().translate(0.05f, 0.0f, 0.05f);
    Core::WeakPointer<Core::Object3D> torchLightObject = engine->createObject3D();
    torchParticleSystemObject->addChild(torchLightObject);
    torchLightObject->getTransform().translate(0.0f, 1.0f, 0.0f);
    torchParticleSystemObject->getTransform().translate(x, y, z);
    torchParticleSystemObject->setName("Torch");

    FlickerLight torchFlickerLight;
    torchFlickerLight.create(torchLightObject, true, 512, 0.0115f, 0.35f);
    Core::WeakPointer<Core::PointLight> torchLight =torchFlickerLight.getLight();
    torchLight->setColor(1.0f, 0.6f, 0.1f, 1.0f);
    torchLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    torchLight->setRadius(lightRadius);
    torchFlickerLight.setIntensity(lightIntensity);
    return torchFlickerLight;
}

Core::WeakPointer<Core::Object3D> MoonlitNightScene::createEmberParticleSystem(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Object3D> parent, Core::Atlas& atlas, float scale) {
    Core::WeakPointer<Core::Object3D> emberParticleSystemObject = engine->createObject3D();
    parent->addChild(emberParticleSystemObject);
    Core::WeakPointer<Core::ParticleSystemAnimatedSpriteRenderer> emberParticleRenderer = engine->createRenderer<Core::ParticleSystemAnimatedSpriteRenderer, Core::ParticleSystem>(emberParticleSystemObject);
    Core::WeakPointer<Core::ParticleStandardMaterial> emberParticleMaterial = emberParticleRenderer->getMaterial();
    emberParticleMaterial->setAtlas(atlas);
    Core::WeakPointer<Core::ParticleSystem> emberParticleSystem = engine->createParticleSystem(emberParticleSystemObject, 25);
    Core::ConstantParticleEmitter& emberConstantEmitter = emberParticleSystem->setEmitter<Core::ConstantParticleEmitter>();
    emberConstantEmitter.emissionRate = 3;

    emberParticleSystem->addParticleStateInitializer<Core::LifetimeInitializer>(Core::RandomGenerator<Core::Real>(3.0f, 1.5f, false));
    emberParticleSystem->addParticleStateInitializer<Core::SizeInitializer>(Core::RandomGenerator<Core::Vector2r>(Core::Vector2r(0.0f, 0.0f), Core::Vector2r(scale * 0.15f, scale  * 0.15f), 0.0f, 0.0f, false));
    emberParticleSystem->addParticleStateInitializer<Core::BoxPositionInitializer>(0.05f * scale, 0.0f, 0.05f * scale, -0.025f * scale, 0.0f, -0.025f * scale);
    emberParticleSystem->addParticleStateInitializer<Core::RandomVelocityInitializer>(0.4f * scale, 0.5f * scale, 0.4f * scale, -0.2f * scale, 0.8f * scale, -0.2f * scale, 0.6f * scale, 0.8f * scale);

    Core::OpacityInterpolatorOperator& emberOpacityInterpolatorOperator = emberParticleSystem->addParticleStateOperator<Core::OpacityInterpolatorOperator>();
    emberOpacityInterpolatorOperator.addElement(0.0f, 0.0f);
    emberOpacityInterpolatorOperator.addElement(0.7f, 0.25f);
    emberOpacityInterpolatorOperator.addElement(0.9f, 0.5f);
    emberOpacityInterpolatorOperator.addElement(0.0f, 1.0f);

    Core::ColorInterpolatorOperator& emberColorInterpolatorOperator = emberParticleSystem->addParticleStateOperator<Core::ColorInterpolatorOperator>(true);
    emberColorInterpolatorOperator.addElement(Core::Color(1.0f, 0.7f, 0.0f, 1.0f), 0.0f);
    emberColorInterpolatorOperator.addElement(Core::Color(1.0f, 0.6f, 0.0f, 1.0f), 0.6f);
    emberColorInterpolatorOperator.addElement(Core::Color(1.0f, 0.4f, 0.0f, 1.0f), 1.0f);

    emberParticleSystem->addParticleStateOperator<Core::AccelerationOperator>(Core::SphereRandomGenerator<Core::Vector3r>(Core::Math::TwoPI, 0.0f, Core::Math::PI * 0.85f, -Core::Math::PI * .35f, 15.0f, 1.5f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f));

    emberParticleSystem->setSimulateInWorldSpace(true);
    emberParticleSystem->start();

    return emberParticleSystemObject;
}

Core::WeakPointer<Core::Object3D> MoonlitNightScene::createBaseFlameParticleSystem(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Object3D> parent, Core::Atlas& atlas, float scale) {
    Core::WeakPointer<Core::Object3D> baseFlameParticleSystemObject = engine->createObject3D();
    parent->addChild(baseFlameParticleSystemObject);
    Core::WeakPointer<Core::ParticleSystemAnimatedSpriteRenderer> baseFlameParticleRenderer = engine->createRenderer<Core::ParticleSystemAnimatedSpriteRenderer, Core::ParticleSystem>(baseFlameParticleSystemObject);
    Core::WeakPointer<Core::ParticleStandardMaterial> baseFlameParticleMaterial = baseFlameParticleRenderer->getMaterial();
    baseFlameParticleMaterial->setAtlas(atlas);
    baseFlameParticleMaterial->setInterpolateAtlasFrames(true);
    Core::WeakPointer<Core::ParticleSystem> baseFlameParticleSystem = engine->createParticleSystem(baseFlameParticleSystemObject, 50);
    Core::ConstantParticleEmitter& baseFlameConstantEmitter = baseFlameParticleSystem->setEmitter<Core::ConstantParticleEmitter>();
    baseFlameConstantEmitter.emissionRate = 10;

    baseFlameParticleSystem->addParticleSequence(0, 18);
    Core::WeakPointer<Core::ParticleSequenceGroup> baseFlameParticleSequences = baseFlameParticleSystem->getParticleSequences();

    baseFlameParticleSystem->addParticleStateInitializer<Core::LifetimeInitializer>(Core::RandomGenerator<Core::Real>(0.0f, 0.0f, false));
    baseFlameParticleSystem->addParticleStateInitializer<Core::RotationInitializer>(Core::RandomGenerator<Core::Real>(Core::Math::PI / 2.0f, -Core::Math::PI / 2.0f, false));
    baseFlameParticleSystem->addParticleStateInitializer<Core::RotationalSpeedInitializer>(Core::RandomGenerator<Core::Real>(1.0f, -1.0f, false));
    baseFlameParticleSystem->addParticleStateInitializer<Core::SizeInitializer>(Core::RandomGenerator<Core::Vector2r>(Core::Vector2r(0.25  * scale, 0.25 * scale), Core::Vector2r(0.5f * scale, 0.5f * scale), 0.0f, 0.0f, false));
    baseFlameParticleSystem->addParticleStateInitializer<Core::BoxPositionInitializer>(0.05f * scale, 0.0f, 0.05f * scale, -0.025f * scale, 0.0f, -0.025f * scale);
    baseFlameParticleSystem->addParticleStateInitializer<Core::RandomVelocityInitializer>(0.05f * scale, 0.4f * scale, 0.05f * scale, -0.025f * scale, 0.8f * scale, -0.025f * scale,  0.35f * scale, 0.5f * scale);
    baseFlameParticleSystem->addParticleStateInitializer<Core::SequenceInitializer>(baseFlameParticleSequences);

    baseFlameParticleSystem->addParticleStateOperator<Core::SequenceOperator>(baseFlameParticleSequences, 0.07f, false);

    Core::OpacityInterpolatorOperator& baseFlameOpacityInterpolatorOperator = baseFlameParticleSystem->addParticleStateOperator<Core::OpacityInterpolatorOperator>();
    baseFlameOpacityInterpolatorOperator.addElement(0.0f, 0.0f);
    baseFlameOpacityInterpolatorOperator.addElement(0.35f, 0.25f);
    baseFlameOpacityInterpolatorOperator.addElement(0.35f, 0.5f);
    baseFlameOpacityInterpolatorOperator.addElement(0.0f, 1.0f);

    Core::SizeInterpolatorOperator& baseFlameSizeInterpolatorOperator = baseFlameParticleSystem->addParticleStateOperator<Core::SizeInterpolatorOperator>(true);
    baseFlameSizeInterpolatorOperator.addElement(Core::Vector2r(0.6f, 0.6f), 0.0f);
    baseFlameSizeInterpolatorOperator.addElement(Core::Vector2r(1.0f, 1.0f), 0.4f);
    baseFlameSizeInterpolatorOperator.addElement(Core::Vector2r(1.0f, 1.0f), 1.0f);

    Core::ColorInterpolatorOperator& baseFlameColorInterpolatorOperator = baseFlameParticleSystem->addParticleStateOperator<Core::ColorInterpolatorOperator>(true);
    baseFlameColorInterpolatorOperator.addElement(Core::Color(1.0f, 1.0f, 1.0f, 1.0f), 0.0f);
    baseFlameColorInterpolatorOperator.addElement(Core::Color(1.5f, 1.5f, 1.5f, 1.0f), 0.5f);
    baseFlameColorInterpolatorOperator.addElement(Core::Color(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);

    baseFlameParticleSystem->addParticleStateOperator<Core::AccelerationOperator>(Core::RandomGenerator<Core::Vector3r>(Core::Vector3r(0.0f, 0.0f, 0.0f), Core::Vector3r(0.0f, 1.5f * scale, 0.0f), 0.0f * scale, 0.0f * scale, false));

    baseFlameParticleSystem->setSimulateInWorldSpace(true);
    baseFlameParticleSystem->start();

    return baseFlameParticleSystemObject;
}

Core::WeakPointer<Core::Object3D> MoonlitNightScene::createBrightFlameParticleSystem(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Object3D> parent, Core::Atlas& atlas, float scale) {
    Core::WeakPointer<Core::Object3D> brightFlameParticleSystemObject = engine->createObject3D();
    parent->addChild(brightFlameParticleSystemObject);
    Core::WeakPointer<Core::ParticleSystemAnimatedSpriteRenderer> brightFlameParticleRenderer = engine->createRenderer<Core::ParticleSystemAnimatedSpriteRenderer, Core::ParticleSystem>(brightFlameParticleSystemObject);
    Core::WeakPointer<Core::ParticleStandardMaterial> brightFlameParticleMaterial = brightFlameParticleRenderer->getMaterial();
    brightFlameParticleMaterial->setAtlas(atlas);
    brightFlameParticleMaterial->setInterpolateAtlasFrames(true);
    Core::WeakPointer<Core::ParticleSystem> brightFlameParticleSystem = engine->createParticleSystem(brightFlameParticleSystemObject, 20);
    Core::ConstantParticleEmitter& brightFlameConstantEmitter = brightFlameParticleSystem->setEmitter<Core::ConstantParticleEmitter>();
    brightFlameConstantEmitter.emissionRate = 5;

    brightFlameParticleSystem->addParticleSequence(0, 16);
    Core::WeakPointer<Core::ParticleSequenceGroup> brightFlameParticleSequences = brightFlameParticleSystem->getParticleSequences();

    brightFlameParticleSystem->addParticleStateInitializer<Core::LifetimeInitializer>(Core::RandomGenerator<Core::Real>(0.0f, 0.0f, false));
    brightFlameParticleSystem->addParticleStateInitializer<Core::RotationInitializer>(Core::RandomGenerator<Core::Real>(Core::Math::PI, -Core::Math::PI / 2.0, false));
    brightFlameParticleSystem->addParticleStateInitializer<Core::RotationalSpeedInitializer>(Core::RandomGenerator<Core::Real>(Core::Math::PI / 2.0f, -Core::Math::PI / 4.0f, false));
    brightFlameParticleSystem->addParticleStateInitializer<Core::SizeInitializer>(Core::RandomGenerator<Core::Vector2r>(Core::Vector2r(0.0, 0.0), Core::Vector2r(0.0f, 0.0f), 0.2f * scale, 0.65f * scale, false));
    brightFlameParticleSystem->addParticleStateInitializer<Core::BoxPositionInitializer>(0.1f * scale, 0.0f, 0.1f * scale, -0.05f * scale, 0.0f, -0.05f * scale);
    brightFlameParticleSystem->addParticleStateInitializer<Core::RandomVelocityInitializer>(0.02f * scale, 0.4f * scale, 0.02f * scale, -0.01f * scale, 0.4f * scale, -0.01f * scale,  0.1f * scale, .2f * scale);
    brightFlameParticleSystem->addParticleStateInitializer<Core::SequenceInitializer>(brightFlameParticleSequences, false);
    brightFlameParticleSystem->addParticleStateOperator<Core::SequenceOperator>(brightFlameParticleSequences, 0.1f, false, false);

    Core::OpacityInterpolatorOperator& brightFlameOpacityInterpolatorOperator = brightFlameParticleSystem->addParticleStateOperator<Core::OpacityInterpolatorOperator>();
    brightFlameOpacityInterpolatorOperator.addElement(0.0f, 0.0f);
    brightFlameOpacityInterpolatorOperator.addElement(0.6f, 0.2f);
    brightFlameOpacityInterpolatorOperator.addElement(.5f, 0.75f);
    brightFlameOpacityInterpolatorOperator.addElement(0.0f, 1.0f);

    Core::SizeInterpolatorOperator& brightFlameSizeInterpolatorOperator = brightFlameParticleSystem->addParticleStateOperator<Core::SizeInterpolatorOperator>(true);
    brightFlameSizeInterpolatorOperator.addElement(Core::Vector2r(0.3f, 0.3f), 0.0f);
    brightFlameSizeInterpolatorOperator.addElement(Core::Vector2r(1.0f, 1.0f), 0.4f);
    brightFlameSizeInterpolatorOperator.addElement(Core::Vector2r(1.0f, 1.0f), 0.55f);
    brightFlameSizeInterpolatorOperator.addElement(Core::Vector2r(0.65f, 0.65f), 0.75f);
    brightFlameSizeInterpolatorOperator.addElement(Core::Vector2r(0.1f, 0.1f), 1.0f);

    Core::ColorInterpolatorOperator& brightFlameColorInterpolatorOperator = brightFlameParticleSystem->addParticleStateOperator<Core::ColorInterpolatorOperator>(true);
    brightFlameColorInterpolatorOperator.addElement(Core::Color(1.0f, 1.0f, 1.0f, 1.0f), 0.0f);
    brightFlameColorInterpolatorOperator.addElement(Core::Color(2.0f, 2.0f, 2.0f, 1.0f), 0.3f);
    brightFlameColorInterpolatorOperator.addElement(Core::Color(2.0f, 2.0f, 2.0f, 1.0f), 0.4);
    brightFlameColorInterpolatorOperator.addElement(Core::Color(0.9f, 0.6f, 0.3f, 1.0f), 0.65f);
    brightFlameColorInterpolatorOperator.addElement(Core::Color(0.75f, 0.0f, 0.0f, 1.0f), 1.0f);

    brightFlameParticleSystem->addParticleStateOperator<Core::AccelerationOperator>(Core::RandomGenerator<Core::Vector3r>(Core::Vector3r(0.0f, 0.0f, 0.0f), Core::Vector3r(0.0f, 1.5f * scale, 0.0f), 0.0f * scale, 0.0f * scale, false));

    brightFlameParticleSystem->setSimulateInWorldSpace(true);
    brightFlameParticleSystem->start();

    return brightFlameParticleSystemObject;
}
