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

#include <QDir>

MoonlitNightScene::MoonlitNightScene(ModelerApp& modelerApp): ModelerScene(modelerApp) {
    this->frameCount = 0;
}

void MoonlitNightScene::load() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();

    renderCamera->setHDREnabled(true);
    renderCamera->setHDRToneMapTypeExposure(2.0f);
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
    this->frameCount++;
}

void MoonlitNightScene::setupSkyboxes() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();

    Core::TextureAttributes skyboxTextureAttributes;
    skyboxTextureAttributes.FilterMode = Core::TextureFilter::Linear;
    skyboxTextureAttributes.MipLevels = 2;
    Core::WeakPointer<Core::CubeTexture> skyboxTexture = engine->createCubeTexture(skyboxTextureAttributes);

    std::vector<std::shared_ptr<Core::StandardImage>> skyboxImages;
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/moonlit_night/nightsky_north.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/moonlit_night/nightsky_south.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/moonlit_night/nightsky_up.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/moonlit_night/nightsky_down.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/moonlit_night/nightsky_west.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/moonlit_night/nightsky_east.png", true));
    skyboxTexture->buildFromImages(skyboxImages[0], skyboxImages[1], skyboxImages[2], skyboxImages[3], skyboxImages[4], skyboxImages[5]);

    renderCamera->getSkybox().build(skyboxTexture, true);
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
    directionalLight->setIntensity(1.0f);
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

    // Fire 2 atlas
    std::string fire2TexturePath = fileSystem->fixupPathForLocalFilesystem("assets/textures/fire_particle_2.png");
    std::shared_ptr<Core::StandardImage> fire2TextureImage;
    fire2TextureImage = Core::ImageLoader::loadImageU(fire2TexturePath);
    Core::WeakPointer<Core::Texture2D> fire2Texture = Core::Engine::instance()->getGraphicsSystem()->createTexture2D(texAttributes);
    fire2Texture->buildFromImage(fire2TextureImage);
    Core::Atlas fire2Atlas(fire2Texture);
    fire2Atlas.addTileArray(18, 0.0f, 0.0f, 128.0f / 1024.0f, 128.0f / 512.0f);

    // Fire 4 flat atlas
    std::string fire4FlatTexturePath = fileSystem->fixupPathForLocalFilesystem("assets/textures/fire_particle_4_flat.png");
    std::shared_ptr<Core::StandardImage> fire4FlatTextureImage;
    fire4FlatTextureImage = Core::ImageLoader::loadImageU(fire4FlatTexturePath);
    Core::WeakPointer<Core::Texture2D> fire4Texture = Core::Engine::instance()->getGraphicsSystem()->createTexture2D(texAttributes);
    fire4Texture->buildFromImage(fire4FlatTextureImage);
    Core::Atlas fire4FlatAtlas(fire4Texture);
    fire4FlatAtlas.addTileArray(16, 0.0f, 0.0f, 212.0f / 1024.0f, 256.0f / 1024.0f);

    const std::string fenceEnd5("assets/models/fence_end_5/fence_end_5.fbx");
    const std::string campfire("assets/models/toonlevel/campfire/campfire01.fbx");
    // torch 1
    Core::WeakPointer<Core::PointLight> torch1Light = this->createTorchFlame(engine, coreScene, fire2Atlas, fire4FlatAtlas, 40.4505, 32.3185f, -141.762f, 1.0f, 12.0f, 50.0f);
    Core::IntMask torch1LightCullingMask = torch1Light->getCullingMask();
    Core::IntMaskUtil::clearBit(&torch1LightCullingMask, 1);
    Core::IntMaskUtil::clearBit(&torch1LightCullingMask, 2);
    torch1Light->setCullingMask(torch1LightCullingMask);
    this->sceneHelper.loadModelStandard(fenceEnd5, true, false, 0.0f, 0.0f, 0.0f, 0, 1, 0, 0,  40.5005f, 27.4293f, -141.762f, 0.005f, 0.005f, 0.02f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 4);

    // torch 2
    Core::WeakPointer<Core::PointLight> torch2Light = this->createTorchFlame(engine, coreScene, fire2Atlas, fire4FlatAtlas, 51.0316f, 32.3185f, -141.762f, 1.0f, 12.0f, 50.0f);
    Core::IntMask torch2LightCullingMask = torch2Light->getCullingMask();
    Core::IntMaskUtil::clearBit(&torch2LightCullingMask, 1);
    Core::IntMaskUtil::clearBit(&torch2LightCullingMask, 2);
    torch2Light->setCullingMask(torch2LightCullingMask);
    this->sceneHelper.loadModelStandard(fenceEnd5, true, false, 0.0f, 0.0f, 0.0f, 0, 1, 0, 0, 51.0816f, 27.4293f, -141.762f, 0.005f, 0.005f, 0.02f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 4);

    // torch 3
    Core::WeakPointer<Core::PointLight> torch3Light = this->createTorchFlame(engine, coreScene, fire2Atlas, fire4FlatAtlas, 45.4915f, 28.405f, -164.412f, 2.0f, 10.0f, 75.0f);
    Core::IntMask torch3LightCullingMask = torch3Light->getCullingMask();
    Core::IntMaskUtil::clearBit(&torch3LightCullingMask, 1);
    Core::IntMaskUtil::clearBit(&torch3LightCullingMask, 4);
    torch3Light->setCullingMask(torch3LightCullingMask);
    torch3Light->getOwner()->getTransform().translate(0.0f, 1.0f, 0.0f);
    this->sceneHelper.loadModelStandard(campfire, true, false, 0.0f, 0.0f, 0.0f, 0, 1, 0, 0,45.4915f, 27.405f, -164.412f, 0.5f, 0.5f, 0.5f, false, 0.0f, 0.85f, false, 0, false, false, false, dummyOnLoad, 2);

    // torch 4
    Core::WeakPointer<Core::PointLight> torch4Light = this->createTorchFlame(engine, coreScene, fire2Atlas, fire4FlatAtlas, 31.6682f, 31.113f, -170.746f, 1.0f, 10.0f, 75.0f);
    Core::IntMask torch4LightCullingMask = torch4Light->getCullingMask();
    Core::IntMaskUtil::clearBit(&torch4LightCullingMask, 1);
    Core::IntMaskUtil::clearBit(&torch4LightCullingMask, 4);
    torch4Light->setCullingMask(torch4LightCullingMask);
    this->sceneHelper.loadModelStandard(fenceEnd5, true, false, 0.0f, 0.0f, 0.0f, 0, 1, 0, 0, 31.7182f, 26.1624f, -170.746f, 0.005f, 0.005f, 0.02f, false, 0.0f, 0.85f, false, 0, false, false, true, dummyOnLoad, 2);

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

Core::WeakPointer<Core::PointLight> MoonlitNightScene::createTorchFlame(Core::WeakPointer<Core::Engine> engine, CoreScene& coreScene, Core::Atlas& fire2Atlas,
                                                                        Core::Atlas& fire4FlatAtlas, float x, float y, float z, float scale, float lightRadius, float lightIntensity) {
    Core::WeakPointer<Core::Object3D> torchParticleSystemObject = engine->createObject3D();
    coreScene.addObjectToScene(torchParticleSystemObject);
    this->createFire2ParticleSystem(engine, torchParticleSystemObject, fire2Atlas, scale);
    this->createFire4ParticleSystem(engine, torchParticleSystemObject, fire4FlatAtlas, scale);
    Core::WeakPointer<Core::Object3D> torchLightObject = engine->createObject3D();
    torchParticleSystemObject->addChild(torchLightObject);
    torchLightObject->getTransform().translate(0.0f, 1.0f, 0.0f);
    torchParticleSystemObject->getTransform().translate(x, y, z);
    torchParticleSystemObject->setName("Torch");
    Core::WeakPointer<Core::PointLight> torchLight = engine->createPointLight<Core::PointLight>(torchLightObject, true, 512, 0.0115f, 0.35f);
    torchLight->setColor(1.0f, 0.6f, 0.1f, 1.0f);
    torchLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    torchLight->setRadius(lightRadius);
    torchLight->setIntensity(lightIntensity);
    return torchLight;
}

void MoonlitNightScene::createFire2ParticleSystem(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Object3D> parent, Core::Atlas& atlas, float scale) {
    Core::WeakPointer<Core::Object3D> fire2ParticleSystemObject = engine->createObject3D();
    parent->addChild(fire2ParticleSystemObject);
    Core::WeakPointer<Core::ParticleSystemAnimatedSpriteRenderer> fire2ParticleRenderer = engine->createRenderer<Core::ParticleSystemAnimatedSpriteRenderer, Core::ParticleSystem>(fire2ParticleSystemObject);
    Core::WeakPointer<Core::ParticleStandardMaterial> fire2ParticleMaterial = fire2ParticleRenderer->getMaterial();
    fire2ParticleMaterial->setAtlas(atlas);
    Core::WeakPointer<Core::ParticleSystem> fire2ParticleSystem = engine->createParticleSystem(fire2ParticleSystemObject, 6);
    Core::ConstantParticleEmitter& fire2ConstantEmitter = fire2ParticleSystem->setEmitter<Core::ConstantParticleEmitter>();
    fire2ConstantEmitter.emissionRate = 5;
    fire2ParticleSystem->addParticleSequence(0, 18);
    Core::WeakPointer<Core::ParticleSequenceGroup> fire2ParticleSequences = fire2ParticleSystem->getParticleSequences();
    fire2ParticleSystem->addParticleStateInitializer<Core::LifetimeInitializer>(0.0f, 0.0f);
   // fire2ParticleSystem->addParticleStateInitializer<Core::RotationInitializer>(Core::Math::PI * 2.0, -Core::Math::PI);
    fire2ParticleSystem->addParticleStateInitializer<Core::RotationalSpeedInitializer>(1.0, -1.0f);
    fire2ParticleSystem->addParticleStateInitializer<Core::SizeInitializer>(Core::Vector2r(0.25  * scale, 0.25 * scale), 0.0f, Core::Vector2r(0.5f * scale, 0.5f * scale), 0.0f);
    fire2ParticleSystem->addParticleStateInitializer<Core::BoxPositionInitializer>(0.05f * scale, 0.0f, 0.05f * scale, 0.0f, 0.0f, 0.0f);
    fire2ParticleSystem->addParticleStateInitializer<Core::RandomVelocityInitializer>(0.05f * scale, 0.4f * scale, 0.05f * scale, -0.025f * scale, 0.8f * scale, -0.025f * scale,  0.5f * scale, 1.0f * scale);
    fire2ParticleSystem->addParticleStateInitializer<Core::SequenceInitializer>(fire2ParticleSequences);
    fire2ParticleSystem->addParticleStateOperator<Core::SequenceOperator>(fire2ParticleSequences, 0.055f, false);

    Core::OpacityInterpolatorOperator& fire2OpacityInterpolatorOperator = fire2ParticleSystem->addParticleStateOperator<Core::OpacityInterpolatorOperator>();
    fire2OpacityInterpolatorOperator.addElement(0.0f, 0.0f);
    fire2OpacityInterpolatorOperator.addElement(0.3f, 0.25f);
    fire2OpacityInterpolatorOperator.addElement(0.3f, 0.5f);
    fire2OpacityInterpolatorOperator.addElement(0.0f, 1.0f);

    Core::SizeInterpolatorOperator& fire2SizeInterpolatorOperator = fire2ParticleSystem->addParticleStateOperator<Core::SizeInterpolatorOperator>(true);
    fire2SizeInterpolatorOperator.addElement(Core::Vector2r(0.6f, 0.6f), 0.0f);
    fire2SizeInterpolatorOperator.addElement(Core::Vector2r(1.0f, 1.0f), 0.4f);
    fire2SizeInterpolatorOperator.addElement(Core::Vector2r(1.0f, 1.0f), 1.0f);

    Core::ColorInterpolatorOperator& fire2ColorInterpolatorOperator = fire2ParticleSystem->addParticleStateOperator<Core::ColorInterpolatorOperator>(true);
    fire2ColorInterpolatorOperator.addElement(Core::Color(1.0f, 1.0f, 1.0f, 1.0f), 0.0f);
    fire2ColorInterpolatorOperator.addElement(Core::Color(1.5f, 1.5f, 1.5f, 1.0f), 0.5f);
    fire2ColorInterpolatorOperator.addElement(Core::Color(1.0f, 1.0f, 1.0f, 1.0f), 1.0f);

    fire2ParticleSystem->setSimulateInWorldSpace(true);
    fire2ParticleSystem->start();
}

void MoonlitNightScene::createFire4ParticleSystem(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Object3D> parent, Core::Atlas& atlas, float scale) {
    Core::WeakPointer<Core::Object3D> fire4FlatParticleSystemObject = engine->createObject3D();
    parent->addChild(fire4FlatParticleSystemObject);
    Core::WeakPointer<Core::ParticleSystemAnimatedSpriteRenderer> fire4FlatParticleRenderer = engine->createRenderer<Core::ParticleSystemAnimatedSpriteRenderer, Core::ParticleSystem>(fire4FlatParticleSystemObject);
    Core::WeakPointer<Core::ParticleStandardMaterial> fire4FlatParticleMaterial = fire4FlatParticleRenderer->getMaterial();
    fire4FlatParticleMaterial->setAtlas(atlas);
    Core::WeakPointer<Core::ParticleSystem> fire4FlatParticleSystem = engine->createParticleSystem(fire4FlatParticleSystemObject, 20);
    Core::ConstantParticleEmitter& fire4FlatConstantEmitter = fire4FlatParticleSystem->setEmitter<Core::ConstantParticleEmitter>();
    fire4FlatConstantEmitter.emissionRate = 5;
    fire4FlatParticleSystem->addParticleSequence(0, 16);
    Core::WeakPointer<Core::ParticleSequenceGroup> fire4FlatParticleSequences = fire4FlatParticleSystem->getParticleSequences();
    fire4FlatParticleSystem->addParticleStateInitializer<Core::LifetimeInitializer>(0.0f, 0.0f);
    fire4FlatParticleSystem->addParticleStateInitializer<Core::RotationInitializer>(Core::Math::PI / 2.0f, -Core::Math::PI / 4.0f);
    fire4FlatParticleSystem->addParticleStateInitializer<Core::RotationalSpeedInitializer>(Core::Math::PI, -Core::Math::PI / 2.0f);
    fire4FlatParticleSystem->addParticleStateInitializer<Core::SizeInitializer>(Core::Vector2r(0.0, 0.0), 0.6f * scale, Core::Vector2r(0.0f, 0.0f), 0.4f * scale);
    fire4FlatParticleSystem->addParticleStateInitializer<Core::BoxPositionInitializer>(0.1f * scale, 0.0f, 0.1f * scale, 0.0f, 0.0f, 0.0f);
    fire4FlatParticleSystem->addParticleStateInitializer<Core::RandomVelocityInitializer>(0.05f * scale, 0.4f * scale, 0.05f * scale, -0.025f * scale, 0.8f * scale, -0.025f * scale,  0.5f * scale, 1.3f * scale);
    fire4FlatParticleSystem->addParticleStateInitializer<Core::SequenceInitializer>(fire4FlatParticleSequences, false);
    fire4FlatParticleSystem->addParticleStateOperator<Core::SequenceOperator>(fire4FlatParticleSequences, 0.075f, false, false);

    Core::OpacityInterpolatorOperator& fire4FlatOpacityInterpolatorOperator = fire4FlatParticleSystem->addParticleStateOperator<Core::OpacityInterpolatorOperator>();
    fire4FlatOpacityInterpolatorOperator.addElement(0.0f, 0.0f);
    fire4FlatOpacityInterpolatorOperator.addElement(0.6f, 0.2f);
    fire4FlatOpacityInterpolatorOperator.addElement(.5f, 0.75f);
    fire4FlatOpacityInterpolatorOperator.addElement(0.0f, 1.0f);

    Core::SizeInterpolatorOperator& fire4FlatSizeInterpolatorOperator = fire4FlatParticleSystem->addParticleStateOperator<Core::SizeInterpolatorOperator>(true);
    fire4FlatSizeInterpolatorOperator.addElement(Core::Vector2r(0.3f, 0.3f), 0.0f);
    fire4FlatSizeInterpolatorOperator.addElement(Core::Vector2r(1.0f, 1.0f), 0.4f);
    fire4FlatSizeInterpolatorOperator.addElement(Core::Vector2r(1.0f, 1.0f), 0.55f);
    fire4FlatSizeInterpolatorOperator.addElement(Core::Vector2r(0.65f, 0.65f), 0.75f);
    fire4FlatSizeInterpolatorOperator.addElement(Core::Vector2r(0.1f, 0.1f), 1.0f);

    Core::ColorInterpolatorOperator& fire4FlatColorInterpolatorOperator = fire4FlatParticleSystem->addParticleStateOperator<Core::ColorInterpolatorOperator>(true);
    fire4FlatColorInterpolatorOperator.addElement(Core::Color(1.0f, 1.0f, 1.0f, 1.0f), 0.0f);
    fire4FlatColorInterpolatorOperator.addElement(Core::Color(2.0f, 2.0f, 2.0f, 1.0f), 0.3f);
    fire4FlatColorInterpolatorOperator.addElement(Core::Color(2.0f, 2.0f, 2.0f, 1.0f), 0.4);
    fire4FlatColorInterpolatorOperator.addElement(Core::Color(0.9f, 0.6f, 0.3f, 1.0f), 0.65f);
    fire4FlatColorInterpolatorOperator.addElement(Core::Color(0.75f, 0.0f, 0.0f, 1.0f), 1.0f);

    fire4FlatParticleSystem->setSimulateInWorldSpace(true);
    fire4FlatParticleSystem->start();
}
