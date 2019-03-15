#include "CornfieldScene.h"

#include "Core/image/TextureUtils.h"
#include "Core/material/StandardPhysicalMaterial.h"
#include "Core/geometry/GeometryUtils.h"
#include "Core/geometry/Mesh.h"
#include "Core/render/RenderableContainer.h"
#include "Core/render/MeshRenderer.h"
#include "Core/render/ReflectionProbe.h"
#include "Core/render/RenderTargetCube.h"
#include "Core/light/AmbientIBLLight.h"
#include "Core/light/AmbientLight.h"
#include "Core/material/BasicMaterial.h"
#include "Core/material/BasicCubeMaterial.h"

#include <QDir>

CornfieldScene::CornfieldScene(): coreScene(nullptr) {
    this->frameCount = 0;
}

void CornfieldScene::setupScene(Core::WeakPointer<Core::Engine> engine, ModelerApp& modelerApp, CoreScene& coreScene,
                                Core::WeakPointer<Core::Camera> renderCamera) {
    this->engine = engine;
    this->modelerApp = &modelerApp;
    this->coreScene = &coreScene;

    renderCamera->setHDREnabled(true);
    renderCamera->setHDRToneMapTypeExposure(2.5f);
    renderCamera->setHDRGamma(1.9f);

    this->setupSkyboxes(renderCamera);
    this->setupDefaultObjects(renderCamera);
    this->setupLights();
}

void CornfieldScene::update() {
    // TODO: remove this code (it displays a cube that shows the irradiance map)
    if (this->frameCount == 1) {
        Core::Color cubeColor(1.0f, 1.0f, 1.0f, 1.0f);
        Core::WeakPointer<Core::Mesh> cubeMesh = Core::GeometryUtils::buildBoxMesh(4.0, 4.0, 4.0, cubeColor);
        Core::WeakPointer<Core::BasicCubeMaterial> cubeMaterial = engine->createMaterial<Core::BasicCubeMaterial>();
        cubeMaterial->setLit(false);
        Core::WeakPointer<Core::CubeTexture> irradianceMap = Core::WeakPointer<Core::Texture>::dynamicPointerCast<Core::CubeTexture>(this->centerProbe->getIrradianceMap()->getColorTexture());
        //Core::WeakPointer<Core::CubeTexture> irradianceMap = Core::WeakPointer<Core::Texture>::dynamicPointerCast<Core::CubeTexture>(this->centerProbe->getSceneRenderTarget()->getColorTexture());
        cubeMaterial->setTexture(irradianceMap);
        Core::WeakPointer<Core::RenderableContainer<Core::Mesh>> cubeObj = Core::GeometryUtils::buildMeshContainer(cubeMesh, cubeMaterial, "testCube");
        this->coreScene->addObjectToScene(cubeObj);
        this->coreScene->addObjectToSceneRaycaster(cubeObj, cubeMesh);
        cubeObj->getTransform().getLocalMatrix().translate(0.0f, 5.0f, 0.0f);
        cubeObj->getTransform().updateWorldMatrix();
    }
    this->frameCount++;
}

void CornfieldScene::setupSkyboxes(Core::WeakPointer<Core::Camera> renderCamera) {
    std::vector<std::shared_ptr<Core::StandardImage>> skyboxImages;
    skyboxImages.push_back(Core::ImageLoader::loadImageU("../../skyboxes/redorange/fixed/front.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("../../skyboxes/redorange/fixed/back.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("../../skyboxes/redorange/fixed/up.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("../../skyboxes/redorange/fixed/down.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("../../skyboxes/redorange/fixed/left.png", true));
    skyboxImages.push_back(Core::ImageLoader::loadImageU("../../skyboxes/redorange/fixed/right.png", true));

    Core::TextureAttributes skyboxTextureAttributes;
    skyboxTextureAttributes.FilterMode = Core::TextureFilter::BiLinear;
    skyboxTextureAttributes.MipMapLevel = 0;
    Core::WeakPointer<Core::CubeTexture> skyboxTexture = this->engine->createCubeTexture(skyboxTextureAttributes);
    skyboxTexture->buildFromImages(skyboxImages[0], skyboxImages[1], skyboxImages[2], skyboxImages[3], skyboxImages[4], skyboxImages[5]);

    Core::WeakPointer<Core::CubeTexture> hdrSkyboxTexture = Core::TextureUtils::loadFromEquirectangularImage("../../skyboxes/HDR/mealie_road_8k.hdr", true);
    renderCamera->getSkybox().build(hdrSkyboxTexture, true, 2.7f);
    //this->renderCamera->getSkybox().build(skyboxTexture, false);
    renderCamera->setSkyboxEnabled(true);
}

void CornfieldScene::setupDefaultObjects(Core::WeakPointer<Core::Camera> renderCamera) {
    Core::WeakPointer<Core::StandardPhysicalMaterial> cubeMaterial = this->engine->createMaterial<Core::StandardPhysicalMaterial>();
    cubeMaterial->setMetallic(0.05f);
    cubeMaterial->setRoughness(0.1f);
    cubeMaterial->setAmbientOcclusion(1.0f);
    Core::Color slabColor(1.0f, 1.0f, 1.0f, 1.0f);
    Core::WeakPointer<Core::Mesh> slab = Core::GeometryUtils::buildBoxMesh(2.0, 2.0, 2.0, slabColor);
    slab->calculateNormals(75.0f);

    Core::WeakPointer<Core::RenderableContainer<Core::Mesh>> bottomSlabObj(this->engine->createObject3D<Core::RenderableContainer<Core::Mesh>>());
    bottomSlabObj->setName("Base platform");
    Core::WeakPointer<Core::MeshRenderer> bottomSlabRenderer(this->engine->createRenderer<Core::MeshRenderer>(cubeMaterial, bottomSlabObj));
    bottomSlabObj->addRenderable(slab);
    this->coreScene->addObjectToScene(bottomSlabObj);
    this->coreScene->addObjectToSceneRaycaster(bottomSlabObj, slab);
    // this->meshToObjectMap[slab->getObjectID()] = Core::WeakPointer<MeshContainer>::dynamicPointerCast<Core::Object3D>( bottomSlabObj);
    bottomSlabObj->getTransform().getLocalMatrix().scale(15.0f, 1.0f, 15.0f);
    bottomSlabObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, -1.0f, 0.0f));
    bottomSlabObj->getTransform().getLocalMatrix().preRotate(0.0f, 1.0f, 0.0f,Core::Math::PI / 4.0f);

    Core::WeakPointer<Core::Object3D> reflectionProbeObject = engine->createObject3D();
    reflectionProbeObject->setName("Reflection probe");
    this->centerProbe = engine->createReflectionProbe(reflectionProbeObject);
    this->centerProbe->setNeedsUpdate(true);
    reflectionProbeObject->getTransform().getLocalMatrix().translate(0.0f, 10.0f, 0.0f);
    this->coreScene->addObjectToScene(reflectionProbeObject);
    this->centerProbe->setSkybox(renderCamera->getSkybox());
    this->centerProbe->setSkyboxOnly(true);
    Core::WeakPointer<Core::AmbientIBLLight> iblLight = engine->createLight<Core::AmbientIBLLight>(reflectionProbeObject);

    this->modelerApp->loadModel("Assets/models/metal_tank/Water_Tank_fbx.fbx", 3.0f, 80, true, true, [this](Core::WeakPointer<Core::Object3D> rootObject){
        rootObject->getTransform().translate(-11.0f, 0.0f, 0.0f, Core::TransformationSpace::World);
    });

    this->modelerApp->loadModel("Assets/models/toonwarrior/character/warrior.fbx", .075f, 80, true, false, [this](Core::WeakPointer<Core::Object3D> rootObject){
        rootObject->getTransform().translate(0.0f, 0.0f, -11.0f, Core::TransformationSpace::World);
    });
}

void CornfieldScene::setupLights() {

    this->ambientLightObject = this->engine->createObject3D();
    this->ambientLightObject->setName("Ambient light");
    this->coreScene->addObjectToScene(ambientLightObject);
    Core::WeakPointer<Core::AmbientLight> ambientLight = this->engine->createLight<Core::AmbientLight>(ambientLightObject);
    ambientLight->setColor(0.25f, 0.25f, 0.25f, 1.0f);

    this->pointLightObject = this->engine->createObject3D<Core::RenderableContainer<Core::Mesh>>();
    this->pointLightObject->setName("Point light");
    //this->coreScene.addObjectToScene(pointLightObject);
    Core::WeakPointer<Core::PointLight> pointLight = this->engine->createPointLight<Core::PointLight>(pointLightObject, true, 2048, 0.0115, 0.35);
    pointLight->setColor(1.0f, 1.0f, 1.0f, 1.0f);
    pointLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    pointLight->setRadius(30.0f);
    this->pointLightObject->getTransform().translate(Core::Vector3r(5.0f, 10.0f, 5.0f));
    Core::Real pointLightSize = 0.35f;

    Core::WeakPointer<Core::Mesh> pointLightMesh = Core::GeometryUtils::buildBoxMesh(pointLightSize, pointLightSize, pointLightSize, Core::Color(1.0f, 1.0f, 1.0f, 1.0f));
    Core::WeakPointer<Core::BasicMaterial> pointLightMaterial = this->engine->createMaterial<Core::BasicMaterial>();
    Core::WeakPointer<Core::MeshRenderer> pointLightRenderer(this->engine->createRenderer<Core::MeshRenderer>(pointLightMaterial, this->pointLightObject));
    pointLightRenderer->setCastShadows(false);
    this->pointLightObject->addRenderable(pointLightMesh);
    this->coreScene->addObjectToSceneRaycaster(this->pointLightObject, pointLightMesh);

    this->directionalLightObject = this->engine->createObject3D();
    this->directionalLightObject->setName("Directonal light");
    this->coreScene->addObjectToScene(directionalLightObject);
    //Core::WeakPointer<Core::DirectionalLight> directionalLight = this->engine->createDirectionalLight<Core::DirectionalLight>(directionalLightObject, 3, true, 4096, 0.0001, 0.0005);
    Core::WeakPointer<Core::DirectionalLight> directionalLight = this->engine->createDirectionalLight<Core::DirectionalLight>(directionalLightObject, 3, true, 4096, 0.0001, 0.0005);
    directionalLight->setIntensity(0.5f);
    directionalLight->setColor(1.0, 1.0, 0.6, 1.0f);
    directionalLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    this->directionalLightObject->getTransform().lookAt(Core::Point3r(.75f, -1.0f, 1.25f));
}
