#include "RedSkyScene.h"

#include "Core/image/TextureUtils.h"
#include "Core/image/Texture2D.h"
#include "Core/material/StandardPhysicalMaterial.h"
#include "Core/geometry/GeometryUtils.h"
#include "Core/geometry/Mesh.h"
#include "Core/render/RenderableContainer.h"
#include "Core/render/MeshRenderer.h"
#include "Core/render/ReflectionProbe.h"
#include "Core/render/RenderTargetCube.h"
#include "Core/render/RenderTarget2D.h"
#include "Core/light/AmbientIBLLight.h"
#include "Core/light/AmbientLight.h"
#include "Core/material/BasicMaterial.h"
#include "Core/material/BasicCubeMaterial.h"
#include "Core/util/Time.h"

#include <QDir>

RedSkyScene::RedSkyScene(ModelerApp& modelerApp): ModelerScene(modelerApp) {
    this->frameCount = 0;
}

void RedSkyScene::load() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();

    renderCamera->setHDREnabled(true);
    renderCamera->setHDRToneMapTypeExposure(2.0f);
    renderCamera->setHDRGamma(1.0f);

    /*renderCamera->setHDREnabled(true);
    renderCamera->setHDRToneMapTypeExposure(1.25f);
    renderCamera->setHDRGamma(1.5f);*/

    Core::WeakPointer<Core::Object3D> cameraObj = renderCamera->getOwner();
    cameraObj->getTransform().translate(-20, 15, -30, Core::TransformationSpace::World);
    cameraObj->getTransform().updateWorldMatrix();
    cameraObj->getTransform().lookAt(Core::Point3r(0, 0, 0));

    this->setupSkyboxes();
    this->setupDefaultObjects();
    this->setupLights();
}

void RedSkyScene::update() {
    static Core::Real lightAngle = 0.0f;
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    CoreScene& coreScene = this->modelerApp.getCoreScene();

    lightAngle += Core::Time::getDeltaTime() * 0.1f;
    this->directionalLightObject->getTransform().lookAt(Core::Point3r(Core::Math::cos(lightAngle), -1.0f, Core::Math::sin(lightAngle)));

    // TODO: remove this code (it displays a cube that shows the irradiance map)
   /* if (this->frameCount == 1) {
        Core::Color cubeColor(1.0f, 1.0f, 1.0f, 1.0f);
       // Core::WeakPointer<Core::Mesh> cubeMesh = Core::GeometryUtils::buildBoxMesh(4.0, 4.0, 4.0, cubeColor);
        Core::WeakPointer<Core::Mesh> testMesh = Core::GeometryUtils::buildSphereMesh(3.0f, 32, cubeColor);
        Core::WeakPointer<Core::BasicCubeMaterial> testMaterial = engine->createMaterial<Core::BasicCubeMaterial>();
        testMaterial->setLit(false);
        testMaterial->setCullFace(Core::RenderState::CullFace::Front);
        Core::WeakPointer<Core::CubeTexture> irradianceMap = Core::WeakPointer<Core::Texture>::dynamicPointerCast<Core::CubeTexture>(this->centerProbe->getSpecularIBLPreFilteredMap()->getColorTexture());
        //Core::WeakPointer<Core::CubeTexture> irradianceMap = Core::WeakPointer<Core::Texture>::dynamicPointerCast<Core::CubeTexture>(this->centerProbe->getSceneRenderTarget()->getColorTexture());
        testMaterial->setCubeTexture(irradianceMap);
        Core::WeakPointer<Core::Texture2D> brdfMap = Core::WeakPointer<Core::Texture>::dynamicPointerCast<Core::Texture2D>(this->centerProbe->getSpecularIBLBRDFMap()->getColorTexture());
        testMaterial->setRectTexture(brdfMap);
        Core::WeakPointer<Core::MeshContainer> testObj = Core::GeometryUtils::buildMeshContainer(testMesh, testMaterial, "testCube");
        coreScene.addObjectToScene(testObj);
        coreScene.addObjectToSceneRaycaster(testObj, testMesh);
        testObj->getTransform().getLocalMatrix().translate(0.0f, 5.0f, 0.0f);
        testObj->getTransform().updateWorldMatrix();
    }*/
    this->frameCount++;
}

void RedSkyScene::setupSkyboxes() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    CoreScene& coreSene = this->modelerApp.getCoreScene();

    std::vector<std::shared_ptr<Core::StandardImage>> skyboxImages;
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/redorange/fixed/front.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/redorange/fixed/back.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/redorange/fixed/up.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/redorange/fixed/down.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/redorange/fixed/left.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("assets/skyboxes/redorange/fixed/right.png", true));

    Core::TextureAttributes skyboxTextureAttributes;
    skyboxTextureAttributes.FilterMode = Core::TextureFilter::Linear;
    skyboxTextureAttributes.MipLevels = 2;
    Core::WeakPointer<Core::CubeTexture> skyboxTexture = engine->createCubeTexture(skyboxTextureAttributes);
    skyboxTexture->buildFromImages(skyboxImages[0], skyboxImages[1], skyboxImages[2], skyboxImages[3], skyboxImages[4], skyboxImages[5]);
    renderCamera->getSkybox().build(skyboxTexture, false);
    renderCamera->setSkyboxEnabled(true);
}

void RedSkyScene::setupDefaultObjects() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    CoreScene& coreScene = this->modelerApp.getCoreScene();

   // this->sceneHelper.createBasePlatform();

    this->centerProbe = this->sceneHelper.createSkyboxReflectionProbe(0.0f, 10.0f, 0.0f);

    /*this->modelerApp.loadModel("assets/models/metal_tank/Water_Tank_fbx.fbx", 3.0f, 80, true, true, true, [this](Core::WeakPointer<Core::Object3D> rootObject){
        rootObject->getTransform().translate(-11.0f, 0.0f, 0.0f, Core::TransformationSpace::World);
    });*/

    this->sceneHelper.loadWarrior(true, 0.0f, -27.0f, 0.0f, 6.0f);
    this->sceneHelper.loadHouse(true, Core::Math::PI / 2.0f * 1.15f, 0.0f, 0.0f, 0.0f);

    //this->sceneHelper.createDemoSpheres();
}

void RedSkyScene::setupLights() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    CoreScene& coreScene = this->modelerApp.getCoreScene();

    this->ambientLightObject = engine->createObject3D();
    this->ambientLightObject->setName("Ambient light");
    coreScene.addObjectToScene(ambientLightObject);
    Core::WeakPointer<Core::AmbientLight> ambientLight = engine->createLight<Core::AmbientLight>(ambientLightObject);
    ambientLight->setColor(0.25f, 0.25f, 0.25f, 1.0f);

    this->pointLightObject = engine->createObject3D<Core::MeshContainer>();
    this->pointLightObject->setName("Point light");
    //this->coreScene.addObjectToScene(pointLightObject);
    Core::WeakPointer<Core::PointLight> pointLight = engine->createPointLight<Core::PointLight>(pointLightObject, true, 2048, 0.0115, 0.35);
    pointLight->setColor(1.0f, 1.0f, 1.0f, 1.0f);
    pointLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    pointLight->setRadius(30.0f);
    this->pointLightObject->getTransform().translate(Core::Vector3r(5.0f, 10.0f, 5.0f));
    Core::Real pointLightSize = 0.35f;

    Core::WeakPointer<Core::Mesh> pointLightMesh = Core::GeometryUtils::buildBoxMesh(pointLightSize, pointLightSize, pointLightSize, Core::Color(1.0f, 1.0f, 1.0f, 1.0f));
    Core::WeakPointer<Core::BasicMaterial> pointLightMaterial = engine->createMaterial<Core::BasicMaterial>();
    Core::WeakPointer<Core::MeshRenderer> pointLightRenderer(engine->createRenderer<Core::MeshRenderer, Core::Mesh>(pointLightMaterial, this->pointLightObject));
    pointLightRenderer->setCastShadows(false);
    this->pointLightObject->addRenderable(pointLightMesh);
    coreScene.addObjectToSceneRaycaster(this->pointLightObject, pointLightMesh);

    this->directionalLightObject = engine->createObject3D();
    this->directionalLightObject->setName("Directonal light");
    coreScene.addObjectToScene(directionalLightObject);
    //Core::WeakPointer<Core::DirectionalLight> directionalLight = this->engine->createDirectionalLight<Core::DirectionalLight>(directionalLightObject, 3, true, 4096, 0.0001, 0.0005);
    Core::WeakPointer<Core::DirectionalLight> directionalLight = engine->createDirectionalLight<Core::DirectionalLight>(directionalLightObject, 3, true, 4096, 0.0001, 0.0005);
    directionalLight->setIntensity(1.85f);
    directionalLight->setColor(1.0, 0.8, 0.5, 1.0f);
    directionalLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    this->directionalLightObject->getTransform().lookAt(Core::Point3r(1.0f, -1.0f, -1.0f));
}
