#include "SceneHelper.h"

#include "ModelerApp.h"

#include "Core/Engine.h"
#include "Core/material/StandardPhysicalMaterial.h"
#include "Core/material/StandardPhysicalMaterialMultiLight.h"
#include "Core/render/MeshRenderer.h"
#include "Core/render/EngineRenderQueue.h"
#include "Core/scene/Object3D.h"
#include "Core/scene/Scene.h"
#include "Core/image/TextureUtils.h"
#include "Core/image/Texture2D.h"
#include "Core/light/AmbientIBLLight.h"
#include "Core/asset/ModelLoader.h"
#include "Core/animation/AnimationManager.h"
#include "Core/animation/AnimationPlayer.h"

SceneHelper::SceneHelper(ModelerApp& modelerApp): modelerApp(modelerApp) {
}

void SceneHelper::createDemoSpheres() {
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    CoreScene& coreScene = this->modelerApp.getCoreScene();
    Core::UInt32 iterations = 6;
    Core::WeakPointer<Core::Object3D> rootObject = engine->createObject3D();
    rootObject->setName("Spheres");
    for (Core::UInt32 m = 0; m < iterations; m++) {
        for (Core::UInt32 r = 0; r < iterations; r++) {
            Core::Real metallic = (Core::Real)m / (Core::Real)(iterations - 1);
            Core::Real roughness = (Core::Real)r / (Core::Real)(iterations - 1);
            Core::Color sphereColor(1.0f, 1.0f, 1.0f, 1.0f);
            Core::WeakPointer<Core::Mesh> sphereMesh = Core::GeometryUtils::buildSphereMesh(3.0f, 32, sphereColor);
            Core::WeakPointer<Core::StandardPhysicalMaterial> sphereMaterial = engine->createMaterial<Core::StandardPhysicalMaterial>();
            sphereMaterial->setLit(true);
            sphereMaterial->setRoughness(1.0 - (roughness * 0.9f));
            sphereMaterial->setMetallic(metallic);
            sphereMaterial->setAlbedo(Core::Color(1.0f, 1.0f, 1.0f ,1.0f));

            Core::WeakPointer<Core::MeshContainer> sphereObj = Core::GeometryUtils::buildMeshContainer(sphereMesh, sphereMaterial, "physical sphere");
            Core::WeakPointer<Core::MeshRenderer> meshRenderer = Core::WeakPointer<Core::ObjectRenderer<Core::Mesh>>::dynamicPointerCast<Core::MeshRenderer>(sphereObj->getRenderer());
            meshRenderer->setCastShadows(false);
            rootObject->addChild(sphereObj);
            coreScene.addObjectToSceneRaycaster(sphereObj, sphereMesh);
            sphereObj->getTransform().getLocalMatrix().translate((Core::Real)m * 10.0f, (Core::Real)r * 10.0f, -(Core::Real)m * 10.0f  + 50.0f);
            sphereObj->getTransform().updateWorldMatrix();

        }
    }
    coreScene.addObjectToScene(rootObject);
}

Core::WeakPointer<Core::ReflectionProbe> SceneHelper::createSkyboxReflectionProbe(float x, float y, float z) {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    CoreScene& coreScene = this->modelerApp.getCoreScene();

    Core::WeakPointer<Core::Object3D> reflectionProbeObject = engine->createObject3D();
    reflectionProbeObject->setName("Reflection probe");
    Core::WeakPointer<Core::ReflectionProbe> reflectionProbe = engine->createReflectionProbe(reflectionProbeObject);
    reflectionProbe->setNeedsFullUpdate(true);
    reflectionProbeObject->getTransform().getLocalMatrix().translate(x, y, z);
    coreScene.addObjectToScene(reflectionProbeObject);
    reflectionProbe->setSkybox(renderCamera->getSkybox());
    reflectionProbe->setSkyboxOnly(false);
    reflectionProbe->setRenderWithPhysical(true);
    Core::WeakPointer<Core::AmbientIBLLight> iblLight = engine->createLight<Core::AmbientIBLLight>(reflectionProbeObject);
    iblLight->setReflectionProbe(reflectionProbe);
    return reflectionProbe;
}

void SceneHelper::loadHouse(bool usePhysicalMaterial, float rotation, float x, float y, float z) {
    this->modelerApp.loadModel("assets/models/house/house.fbx", .15f, 90, true, true, usePhysicalMaterial, [this, rotation, x, y, z](Core::WeakPointer<Core::Object3D> rootObject){
        rootObject->getTransform().rotate(0.0f, 1.0f, 0.0f, rotation, Core::TransformationSpace::World);
        rootObject->getTransform().translate(x, y, z,  Core::TransformationSpace::World);
        Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
        Core::WeakPointer<Core::Scene> scene = engine->getActiveScene();
        Core::WeakPointer<Core::MeshContainer> firstMeshContainer;
        scene->visitScene(rootObject, [this, &rootObject, &firstMeshContainer, &engine](Core::WeakPointer<Core::Object3D> obj){

            Core::WeakPointer<Core::MeshContainer> meshContainer = Core::WeakPointer<Core::Object3D>::dynamicPointerCast<Core::MeshContainer>(obj);
            if (meshContainer) {
                obj->setStatic(true);
                if (!firstMeshContainer.isValid()) {
                    firstMeshContainer = meshContainer;
                }
                Core::WeakPointer<Core::ObjectRenderer<Core::Mesh>> objectRenderer = meshContainer->getRenderer();
                if (objectRenderer) {
                    Core::WeakPointer<Core::MeshRenderer> meshRenderer = Core::WeakPointer<Core::ObjectRenderer<Core::Mesh>>::dynamicPointerCast<Core::MeshRenderer>(objectRenderer);
                    if (meshRenderer) {
                        Core::WeakPointer<Core::Material> renderMaterial = meshRenderer->getMaterial();
                        Core::WeakPointer<Core::StandardPhysicalMaterial> physicalMaterial = Core::WeakPointer<Core::Material>::dynamicPointerCast<Core::StandardPhysicalMaterial>(renderMaterial);
                        if (physicalMaterial) {
                            physicalMaterial->setMetallic(0.0f);
                            physicalMaterial->setRoughness(0.85f);
                        }
                        //Core::WeakPointer<Core::Mesh> mesh = meshContainer->getRenderables()[0];
                       // mesh->setNormalsSmoothingThreshold(Core::Math::PI / 1.5f);
                        //mesh->update();
                    }
                }
            }
        });
    });
}

void SceneHelper::loadModelStandard(const std::string& path, bool usePhysicalMaterial, float yRotation, float rx, float ry, float rz, float ra,
                                    float tx, float ty, float tz, float scale, bool singlePassMultiLight, float metallic, float roughness,
                                    bool transparent, unsigned int enabledAlphaChannel, bool doubleSided, bool customShadowRendering,
                                    std::function<void(Core::WeakPointer<Core::Object3D>)> onLoad) {
    this->modelerApp.loadModel(path, scale, 80 * Core::Math::DegreesToRads, true, true, usePhysicalMaterial,
                               [this, yRotation, rx, ry, rz, ra, tx, ty, tz, singlePassMultiLight, metallic, roughness,
                                transparent, enabledAlphaChannel, doubleSided, customShadowRendering, onLoad](Core::WeakPointer<Core::Object3D> rootObject){
        rootObject->getTransform().rotate(0.0f, 1.0f, 0.0f, yRotation, Core::TransformationSpace::World);
        rootObject->getTransform().translate(tx, ty, tz,  Core::TransformationSpace::World);
        rootObject->getTransform().rotate(rx, ry, rz, ra);
        Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
        Core::WeakPointer<Core::Scene> scene = engine->getActiveScene();
        Core::WeakPointer<Core::MeshContainer> firstMeshContainer;
        scene->visitScene(rootObject, [this, &rootObject, &firstMeshContainer, &engine, singlePassMultiLight,
                                       metallic, roughness, transparent, enabledAlphaChannel, doubleSided, customShadowRendering](Core::WeakPointer<Core::Object3D> obj){

            Core::WeakPointer<Core::MeshContainer> meshContainer = Core::WeakPointer<Core::Object3D>::dynamicPointerCast<Core::MeshContainer>(obj);
            if (meshContainer) {
                obj->setStatic(true);
                if (!firstMeshContainer.isValid()) {
                    firstMeshContainer = meshContainer;
                }
                Core::WeakPointer<Core::ObjectRenderer<Core::Mesh>> objectRenderer = meshContainer->getRenderer();
                if (objectRenderer) {
                    Core::WeakPointer<Core::MeshRenderer> meshRenderer = Core::WeakPointer<Core::ObjectRenderer<Core::Mesh>>::dynamicPointerCast<Core::MeshRenderer>(objectRenderer);
                    if (meshRenderer) {
                        Core::WeakPointer<Core::Material> renderMaterial = meshRenderer->getMaterial();
                        Core::WeakPointer<Core::StandardPhysicalMaterial> physicalMaterial = Core::WeakPointer<Core::Material>::dynamicPointerCast<Core::StandardPhysicalMaterial>(renderMaterial);
                        if (physicalMaterial) {
                            physicalMaterial->setMetallic(metallic);
                            physicalMaterial->setRoughness(roughness);
                        }
                        if (singlePassMultiLight) {
                            Core::WeakPointer<Core::StandardPhysicalMaterialMultiLight> multiLightSinglePassPhysicalMaterial = engine->createMaterial<Core::StandardPhysicalMaterialMultiLight>();
                            multiLightSinglePassPhysicalMaterial->copyAttributesFromStandardPhysicalMaterial(physicalMaterial);
                            meshRenderer->setMaterial(multiLightSinglePassPhysicalMaterial);
                            renderMaterial = physicalMaterial = multiLightSinglePassPhysicalMaterial;
                        }
                        if (transparent) {
                            renderMaterial->setBlendingMode(Core::RenderState::BlendingMode::Custom);
                            renderMaterial->setSourceBlendingFactor(Core::RenderState::BlendingFactor::SrcAlpha);
                            renderMaterial->setDestBlendingFactor(Core::RenderState::BlendingFactor::OneMinusSrcAlpha);
                            renderMaterial->setRenderQueue(EngineRenderQueue::Transparent);
                            if (enabledAlphaChannel == 1) {
                                physicalMaterial->setOpacityChannelRedEnabled(true);
                                physicalMaterial->setOpacityChannelAlphaEnabled(false);
                            }
                            else if (enabledAlphaChannel == 4) {
                                physicalMaterial->setOpacityChannelRedEnabled(false);
                                physicalMaterial->setOpacityChannelAlphaEnabled(true);
                            }
                            physicalMaterial->setDiscardMask(0x80);
                        }
                        if (customShadowRendering) {
                            renderMaterial->setCustomDepthOutput(true);
                            renderMaterial->setCustomDepthOutputCopyOverrideMatrialState(true);
                        }
                        if (doubleSided) {
                            renderMaterial->setFaceCullingEnabled(false);
                            renderMaterial->setCustomDepthOutput(true);
                            renderMaterial->setCustomDepthOutputStateCopyExcludeFaceCulling(true);
                        }
                        //Core::WeakPointer<Core::Mesh> mesh = meshContainer->getRenderables()[0];
                       // mesh->setNormalsSmoothingThreshold(Core::Math::PI / 1.5f);
                        //mesh->update();
                    }
                }
            }
        });
        onLoad(rootObject);
    });
}

void SceneHelper::loadTerrain(bool usePhysicalMaterial, float rotation, float x, float y, float z) {
    this->modelerApp.loadModel("assets/models/terrain/terrain.fbx", .01f, 80 * Core::Math::DegreesToRads, true, true, usePhysicalMaterial, [this, rotation, x, y, z](Core::WeakPointer<Core::Object3D> rootObject){
        rootObject->getTransform().rotate(0.0f, 1.0f, 0.0f, rotation, Core::TransformationSpace::World);
        Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
        Core::WeakPointer<Core::Scene> scene = engine->getActiveScene();
        Core::WeakPointer<Core::MeshContainer> firstMeshContainer;
        scene->visitScene(rootObject, [this, &rootObject, &firstMeshContainer, &engine](Core::WeakPointer<Core::Object3D> obj){

            Core::WeakPointer<Core::MeshContainer> meshContainer = Core::WeakPointer<Core::Object3D>::dynamicPointerCast<Core::MeshContainer>(obj);
            if (meshContainer) {
                obj->setStatic(true);
                if (!firstMeshContainer.isValid()) {
                    firstMeshContainer = meshContainer;
                }
                Core::WeakPointer<Core::ObjectRenderer<Core::Mesh>> objectRenderer = meshContainer->getRenderer();
                if (objectRenderer) {
                    Core::WeakPointer<Core::MeshRenderer> meshRenderer = Core::WeakPointer<Core::ObjectRenderer<Core::Mesh>>::dynamicPointerCast<Core::MeshRenderer>(objectRenderer);
                    if (meshRenderer) {
                        Core::WeakPointer<Core::Material> renderMaterial = meshRenderer->getMaterial();
                        Core::WeakPointer<Core::StandardPhysicalMaterial> physicalMaterial = Core::WeakPointer<Core::Material>::dynamicPointerCast<Core::StandardPhysicalMaterial>(renderMaterial);
                        if (physicalMaterial) {
                            physicalMaterial->setMetallic(0.0f);
                            physicalMaterial->setRoughness(0.85f);
                        }
                    }
                }
            }
        });
    });
}

void SceneHelper::loadWarrior(bool usePhysicalMaterial, float rotation, float x, float y, float z) {

    this->modelerApp.loadModel("assets/models/toonwarrior/character/warrior.fbx", 4.0f, 80 * Core::Math::DegreesToRads, false, true, usePhysicalMaterial, [this, rotation, x, y, z](Core::WeakPointer<Core::Object3D> rootObject){
        rootObject->getTransform().rotate(0.0f, 1.0f, 0.0f, rotation, Core::TransformationSpace::World);
        rootObject->getTransform().translate(x, y, z,  Core::TransformationSpace::World);

        Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
        Core::WeakPointer<Core::Scene> scene = engine->getActiveScene();
        Core::WeakPointer<Core::MeshContainer> firstMeshContainer;
        scene->visitScene(rootObject, [this, &rootObject, &firstMeshContainer, &engine](Core::WeakPointer<Core::Object3D> obj){

            Core::WeakPointer<Core::MeshContainer> meshContainer = Core::WeakPointer<Core::Object3D>::dynamicPointerCast<Core::MeshContainer>(obj);
            if (meshContainer) {
                if (!firstMeshContainer.isValid()) {
                    firstMeshContainer = meshContainer;
                }
                Core::WeakPointer<Core::ObjectRenderer<Core::Mesh>> objectRenderer = meshContainer->getRenderer();
                if (objectRenderer) {
                    Core::WeakPointer<Core::MeshRenderer> meshRenderer = Core::WeakPointer<Core::ObjectRenderer<Core::Mesh>>::dynamicPointerCast<Core::MeshRenderer>(objectRenderer);
                    if (meshRenderer) {
                        Core::WeakPointer<Core::Material> renderMaterial = meshRenderer->getMaterial();
                        Core::WeakPointer<Core::StandardPhysicalMaterial> physicalMaterial = Core::WeakPointer<Core::Material>::dynamicPointerCast<Core::StandardPhysicalMaterial>(renderMaterial);
                        if (physicalMaterial) {

                            Core::WeakPointer<Core::Object3D> parent = obj->getParent();
                            Core::UInt32 warriorIndex= 0;
                            if (parent && parent->getName() == "warrior") {
                                if (parent->childCount() >=2 && parent->getChild(1) == obj) {
                                    warriorIndex = 1;
                                }
                            }
                            if (warriorIndex == 1) {
                                physicalMaterial->setMetallic(0.0f);
                                physicalMaterial->setRoughness(0.75f);
                            }
                            else {
                                physicalMaterial->setMetallic(0.75f);
                                physicalMaterial->setRoughness(0.4f);
                            }
                        }
                        Core::WeakPointer<Core::Mesh> mesh = meshContainer->getRenderable(0);
                        mesh->setNormalsSmoothingThreshold(Core::Math::PI / 1.5f);
                        mesh->update();
                    }
                }
            }
        });

        Core::WeakPointer<Core::Animation> animation = Core::Engine::instance()->getModelLoader().loadAnimation("assets/models/toonwarrior/animations/idle.fbx", false, true);
        Core::WeakPointer<Core::AnimationManager> animationManager = Core::Engine::instance()->getAnimationManager();
        Core::WeakPointer<Core::AnimationPlayer> animationPlayer = animationManager->retrieveOrCreateAnimationPlayer(firstMeshContainer->getSkeleton());
        animationPlayer->addAnimation(animation);
        animationPlayer->setSpeed(animation, 1.0f);
        animationPlayer->play(animation);
    });
}

void SceneHelper::createBasePlatform() {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    CoreScene& coreScene = this->modelerApp.getCoreScene();

    Core::WeakPointer<Core::StandardPhysicalMaterial> cubeMaterial = engine->createMaterial<Core::StandardPhysicalMaterial>();
    cubeMaterial->setMetallic(0.05f);
    cubeMaterial->setRoughness(0.1f);
    cubeMaterial->setAmbientOcclusion(1.0f);
    Core::Color slabColor(1.0f, 1.0f, 1.0f, 1.0f);
    Core::WeakPointer<Core::Mesh> slab = Core::GeometryUtils::buildBoxMesh(2.0, 2.0, 2.0, slabColor);
    slab->calculateNormals(75.0f);

    Core::WeakPointer<Core::MeshContainer> bottomSlabObj(engine->createObject3D<Core::MeshContainer>());
    bottomSlabObj->setName("Base platform");
    Core::WeakPointer<Core::MeshRenderer> bottomSlabRenderer(engine->createRenderer<Core::MeshRenderer, Core::Mesh>(cubeMaterial, bottomSlabObj));
    bottomSlabObj->addRenderable(slab);
    coreScene.addObjectToScene(bottomSlabObj);
    coreScene.addObjectToSceneRaycaster(bottomSlabObj, slab);
    bottomSlabObj->getTransform().getLocalMatrix().scale(15.0f, 1.0f, 15.0f);
    bottomSlabObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, -1.0f, 0.0f));
    bottomSlabObj->getTransform().getLocalMatrix().preRotate(0.0f, 1.0f, 0.0f,Core::Math::PI / 4.0f);
    bottomSlabObj->setStatic(true);
}

void SceneHelper::setupCommonSceneElements() {
    Core::WeakPointer<Core::Object3D> renderCameraObject = this->modelerApp.getRenderCameraObject();

    this->centerProbe = this->createSkyboxReflectionProbe(0.0f, 10.0f, 0.0f);

    const std::string bush1Path("assets/models/bush_5/bush_5.fbx");
    const std::string tree1Path("assets/models/tree_00/tree_00.fbx");

    std::function<void(Core::WeakPointer<Core::Object3D>)> dummyOnLoad = [](Core::WeakPointer<Core::Object3D> root){};

    this->loadWarrior(true, 0.0f, 48.82f, 27.18f, -138.77f);
    this->loadTerrain(true, Core::Math::PI / 2.0f, 0.0f, 0.0f, 0.0f);
    this->loadModelStandard("assets/models/castle/castle.fbx", true, Core::Math::PI / 2.0f, 0, 1, 0, 0, 48.82f, 27.62f, -164.77f, 0.015f, false, 0.0f, 0.85f, false, 0, false, false, dummyOnLoad);
    this->loadModelStandard(bush1Path, true, Core::Math::PI / 2.0f, 0, 1, 0, 0, 35.0f, 27.5136f, -135.0f, 0.01f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);
    this->loadModelStandard(bush1Path, true, 0.0f, 0, 1, 0, 0, 28.6463f, 27.5136, -137.331f, 0.015f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);
    this->loadModelStandard(bush1Path, true, Core::Math::PI / 2.0f, 0, 1, 0, 0, 23.0214f, 27.5136f, -141.079f, 0.01f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);
    this->loadModelStandard(tree1Path, true, Core::Math::PI / 2.0f, 1, 0, 0, -5.75f * Core::Math::DegreesToRads , 70.7394f, 27.5136f, -139.049f, 0.01f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);
    this->loadModelStandard(tree1Path, true, 0.0f, 0, 1, 0, 0, 74.83f, 27.5136f, -142.29f, 0.0075f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);
    this->loadModelStandard(bush1Path, true, 0.0f, 0, 1, 0, 0, 78.36f, 27.5136f, -146.75f, 0.015f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);
    this->loadModelStandard(bush1Path, true, 0.0f, 0, 1, 0, 0, 63.26f, 27.5136f, -139.87f, 0.01f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);

    renderCameraObject->getTransform().rotate(0.0f, 1.0f, 0.0f, Core::Math::PI * .8, Core::TransformationSpace::World);
    this->modelerApp.setCameraPosition(48.82f, 45.62f, -104.77f);
}
