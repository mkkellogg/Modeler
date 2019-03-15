#include "CornfieldScene.h"

#include "Core/image/TextureUtils.h"
#include "Core/material/StandardPhysicalMaterial.h"
#include "Core/geometry/GeometryUtils.h"
#include "Core/geometry/Mesh.h"
#include "Core/render/RenderableContainer.h"

CornfieldScene::CornfieldScene(): coreScene(nullptr) {

}

void CornfieldScene::setupScene(Core::WeakPointer<Core::Engine> engine, CoreScene& coreScene,
                                Core::WeakPointer<Core::Camera> renderCamera) {
    this->engine = engine;
    this->coreScene = &coreScene;
    this->setupSkyboxes(renderCamera);
    this->setupDefaultObjects();
    this->setupLights();
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

void CornfieldScene::setupDefaultObjects() {
    /*Core::WeakPointer<Core::StandardPhysicalMaterial> cubeMaterial = this->engine->createMaterial<Core::StandardPhysicalMaterial>();
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
    this->addObjectToSceneRaycaster(bottomSlabObj, slab);
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
    this->centerProbe->setSkybox(this->renderCamera->getSkybox());
    this->centerProbe->setSkyboxOnly(true);
    Core::WeakPointer<Core::AmbientIBLLight> iblLight = engine->createLight<Core::AmbientIBLLight>(reflectionProbeObject);*/
}

void CornfieldScene::setupLights() {

}
