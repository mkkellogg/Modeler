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
    this->setupDefaultObjects();
    this->setupLights();
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

void MoonlitNightScene::setupDefaultObjects() {
    this->sceneHelper.setupCommonSceneElements();
}

void MoonlitNightScene::setupLights() {
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
    directionalLight->setIntensity(1.0f);
    directionalLight->setColor(1.0, 1.0, 1.0, 1.0f);
    directionalLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);

    Core::Vector3r lightVector(-0.5f, -1.0f, -0.5f);
    Core::Vector3r offsetVector = lightVector;
    offsetVector = offsetVector * -1000.0f;
    this->directionalLightObject->getTransform().translate(offsetVector, Core::TransformationSpace::World);
    this->directionalLightObject->getTransform().lookAt(Core::Point3r(lightVector.x, lightVector.y, lightVector.z));







    std::shared_ptr<Core::FileSystem> fileSystem = Core::FileSystem::getInstance();
    Core::TextureAttributes texAttributes;
    texAttributes.FilterMode = Core::TextureFilter::TriLinear;
    texAttributes.MipLevels = 4;
    texAttributes.WrapMode = Core::TextureWrap::Clamp;
    texAttributes.Format = Core::TextureFormat::RGBA8;
    Core::WeakPointer<Core::Object3D> particleSystemObject = engine->createObject3D();
    coreScene.addObjectToScene(particleSystemObject);

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


    // fire 2 particle system
    Core::WeakPointer<Core::Object3D> fire2ParticleSystemObject = engine->createObject3D();
    particleSystemObject->addChild(fire2ParticleSystemObject);
    Core::WeakPointer<Core::ParticleSystemAnimatedSpriteRenderer> fire2ParticleRenderer = engine->createRenderer<Core::ParticleSystemAnimatedSpriteRenderer, Core::ParticleSystem>(fire2ParticleSystemObject);
    Core::WeakPointer<Core::ParticleStandardMaterial> fire2ParticleMaterial = fire2ParticleRenderer->getMaterial();
    fire2ParticleMaterial->setAtlas(fire2Atlas);
    Core::WeakPointer<Core::ParticleSystem> fire2ParticleSystem = engine->createParticleSystem(fire2ParticleSystemObject, 6);
    Core::ConstantParticleEmitter& fire2ConstantEmitter = fire2ParticleSystem->setEmitter<Core::ConstantParticleEmitter>();
    fire2ConstantEmitter.emissionRate = 5;
    fire2ParticleSystem->addParticleSequence(0, 18);
    Core::WeakPointer<Core::ParticleSequenceGroup> fire2ParticleSequences = fire2ParticleSystem->getParticleSequences();
    fire2ParticleSystem->addParticleStateInitializer<Core::LifetimeInitializer>(0.0f, 0.0f);
    fire2ParticleSystem->addParticleStateInitializer<Core::RotationInitializer>(Core::Math::PI * 2.0, -Core::Math::PI);
    fire2ParticleSystem->addParticleStateInitializer<Core::RotationalSpeedInitializer>(1.5, -.75f);
    fire2ParticleSystem->addParticleStateInitializer<Core::SizeInitializer>(Core::Vector2r(0.25, 0.25), 0.0f, Core::Vector2r(0.5f, 0.5f), 0.0f);
    fire2ParticleSystem->addParticleStateInitializer<Core::BoxPositionInitializer>(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
    fire2ParticleSystem->addParticleStateInitializer<Core::RandomVelocityInitializer>(0.1f, 0.4f, 0.1f, -0.05f, 0.8f, -0.05f, 0.5f, 0.8f);
    fire2ParticleSystem->addParticleStateInitializer<Core::SequenceInitializer>(fire2ParticleSequences);
    fire2ParticleSystem->addParticleStateOperator<Core::SequenceOperator>(fire2ParticleSequences, 0.055f, false);
    Core::OpacityInterpolatorOperator& fire2OpacityInterpolatorOperator = fire2ParticleSystem->addParticleStateOperator<Core::OpacityInterpolatorOperator>();
    fire2OpacityInterpolatorOperator.addElement(0.0f, 0.0f);
    fire2OpacityInterpolatorOperator.addElement(0.3f, 0.25f);
    fire2OpacityInterpolatorOperator.addElement(0.3f, 0.5f);
    fire2OpacityInterpolatorOperator.addElement(0.0f, 1.0f);
    fire2ParticleSystem->setSimulateInWorldSpace(true);
    fire2ParticleSystem->start();


    // fire 4 particle system
    Core::WeakPointer<Core::Object3D> fire4FlatParticleSystemObject = engine->createObject3D();
    particleSystemObject->addChild(fire4FlatParticleSystemObject);
    Core::WeakPointer<Core::ParticleSystemAnimatedSpriteRenderer> fire4FlatParticleRenderer = engine->createRenderer<Core::ParticleSystemAnimatedSpriteRenderer, Core::ParticleSystem>(fire4FlatParticleSystemObject);
    Core::WeakPointer<Core::ParticleStandardMaterial> fire4FlatParticleMaterial = fire4FlatParticleRenderer->getMaterial();
    fire4FlatParticleMaterial->setAtlas(fire4FlatAtlas);
    Core::WeakPointer<Core::ParticleSystem> fire4FlatParticleSystem = engine->createParticleSystem(fire4FlatParticleSystemObject, 20);
    Core::ConstantParticleEmitter& fire4FlatConstantEmitter = fire4FlatParticleSystem->setEmitter<Core::ConstantParticleEmitter>();
    //fire4FlatConstantEmitter.emissionRate = 5;
    fire4FlatConstantEmitter.emissionRate = 5;
    fire4FlatParticleSystem->addParticleSequence(0, 16);
    Core::WeakPointer<Core::ParticleSequenceGroup> fire4FlatParticleSequences = fire4FlatParticleSystem->getParticleSequences();
    fire4FlatParticleSystem->addParticleStateInitializer<Core::LifetimeInitializer>(0.0f, 0.0f);
    fire4FlatParticleSystem->addParticleStateInitializer<Core::RotationInitializer>(Core::Math::PI * 2.0, -Core::Math::PI);
    fire4FlatParticleSystem->addParticleStateInitializer<Core::RotationalSpeedInitializer>(3.0f, -1.5f);
    fire4FlatParticleSystem->addParticleStateInitializer<Core::SizeInitializer>(Core::Vector2r(0.0, 0.0), 0.6f, Core::Vector2r(0.0f, 0.0f), 0.4f);
    fire4FlatParticleSystem->addParticleStateInitializer<Core::BoxPositionInitializer>(0.1f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f);
    fire4FlatParticleSystem->addParticleStateInitializer<Core::RandomVelocityInitializer>(0.1f, 0.4f, 0.1f, -0.05f, 0.8f, -0.05f,  0.5f, 1.3f);
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

    fire4FlatParticleSystem->setSimulateInWorldSpace(true);
    fire4FlatParticleSystem->start();

    Core::WeakPointer<Core::Object3D> fireLightObject = engine->createObject3D();
    particleSystemObject->addChild(fireLightObject);
    fireLightObject->getTransform().translate(0.0f, 1.0f, 0.0f);
    particleSystemObject->getTransform().translate(43.3167f, 32.3185f, -136.33f);

    particleSystemObject->setName("Torch");
    Core::WeakPointer<Core::PointLight> pointLight = engine->createPointLight<Core::PointLight>(fireLightObject, true, 512, 0.0115f, 0.35f);
    pointLight->setColor(1.0f, 0.6f, 0.1f, 1.0f);
    pointLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    pointLight->setRadius(12.0f);
    pointLight->setIntensity(50.0f);

}
