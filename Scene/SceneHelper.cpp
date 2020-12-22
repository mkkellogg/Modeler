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
#include "Core/image/TextureAttr.h"
#include "Core/image/Atlas.h"
#include "Core/light/AmbientIBLLight.h"
#include "Core/asset/ModelLoader.h"
#include "Core/animation/AnimationManager.h"
#include "Core/animation/AnimationPlayer.h"
#include "Core/render/BaseObject3DRenderer.h"
#include "Core/filesys/FileSystem.h"

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

            Core::WeakPointer<Core::Object3D> sphereObj = Core::GeometryUtils::buildMeshContainerObject(sphereMesh, sphereMaterial, "physical sphere");
            Core::WeakPointer<Core::MeshRenderer> meshRenderer = Core::WeakPointer<Core::BaseObject3DRenderer>::dynamicPointerCast<Core::MeshRenderer>(sphereObj->getBaseRenderer());
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
    reflectionProbeObject->getTransform().getLocalMatrix().translate(x, y, z);
    coreScene.addObjectToScene(reflectionProbeObject);
    Core::WeakPointer<Core::AmbientIBLLight> iblLight = engine->createLight<Core::AmbientIBLLight>(reflectionProbeObject);
    Core::WeakPointer<Core::ReflectionProbe> reflectionProbe = engine->createReflectionProbe(reflectionProbeObject);
    reflectionProbe->setNeedsFullUpdate(true);
    reflectionProbe->setSkybox(renderCamera->getSkybox());
    reflectionProbe->setSkyboxOnly(false);
    reflectionProbe->setRenderWithPhysical(true);
    iblLight->setReflectionProbe(reflectionProbe);
    return reflectionProbe;
}

void SceneHelper::loadModelStandard(const std::string& path, bool usePhysicalMaterial, bool overrideLoadedTransform, float ex, float ey, float ez, float rx, float ry, float rz, float ra,
                                    float tx, float ty, float tz, float scaleX, float scaleY, float scaleZ, bool singlePassMultiLight, float metallic, float roughness,
                                    bool transparent, unsigned int enabledAlphaChannel, bool doubleSided, bool customShadowRendering,
                                    std::function<void(Core::WeakPointer<Core::Object3D>)> onLoad) {

    std::function<void(Core::WeakPointer<Core::Object3D>)> onLoaded = [this, overrideLoadedTransform, ex, ey, ez, rx, ry, rz, ra, tx, ty, tz, scaleX, scaleY, scaleZ,
                                                                       singlePassMultiLight, metallic, roughness, transparent, enabledAlphaChannel, doubleSided,
                                                                       customShadowRendering, onLoad](Core::WeakPointer<Core::Object3D> rootObject){
        Core::Matrix4x4 rotationMatrix;
        rotationMatrix.makeRotationFromEuler(ex, ey, ez);
        if (!overrideLoadedTransform) rotationMatrix.multiply(rootObject->getTransform().getLocalMatrix());
        rotationMatrix.preRotate(rx, ry, rz, ra);

        Core::Matrix4x4 transform;
        transform.makeScale(scaleX, scaleY, scaleZ);
        transform.preMultiply(rotationMatrix);
        transform.preTranslate(tx, ty, tz);
        rootObject->getTransform().getLocalMatrix().copy(transform);

        Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
        Core::WeakPointer<Core::Scene> scene = engine->getActiveScene();
        Core::WeakPointer<Core::MeshContainer> firstMeshContainer;
        scene->visitScene(rootObject, [&firstMeshContainer, &engine, singlePassMultiLight, metallic, roughness, transparent,
                                       enabledAlphaChannel, doubleSided, customShadowRendering](Core::WeakPointer<Core::Object3D> obj){

            Core::WeakPointer<Core::BaseRenderableContainer> baseRenderableContainer = obj->getBaseRenderableContainer();
           // obj->setLayer(0);
            if (baseRenderableContainer.isValid()) {
                Core::WeakPointer<Core::MeshContainer> meshContainer = Core::WeakPointer<Core::BaseRenderableContainer>::dynamicPointerCast<Core::MeshContainer>(baseRenderableContainer);
                if (meshContainer) {
                    obj->setStatic(true);
                    if (!firstMeshContainer.isValid()) {
                        firstMeshContainer = meshContainer;
                    }
                    Core::WeakPointer<Core::Object3DRenderer<Core::Mesh>> objectRenderer = Core::WeakPointer<Core::BaseObject3DRenderer>::dynamicPointerCast<Core::Object3DRenderer<Core::Mesh>>(obj->getBaseRenderer());
                    if (objectRenderer) {
                        Core::WeakPointer<Core::MeshRenderer> meshRenderer = Core::WeakPointer<Core::Object3DRenderer<Core::Mesh>>::dynamicPointerCast<Core::MeshRenderer>(objectRenderer);
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
                        }
                    }
                }
            }
        });
        onLoad(rootObject);
    };

    this->modelerApp.loadModel(path, 1.0f, 80 * Core::Math::DegreesToRads, true, true, usePhysicalMaterial, onLoaded);
}

void SceneHelper::loadTerrain(bool usePhysicalMaterial, float rotation) {
     std::function<void(Core::WeakPointer<Core::Object3D>)> onLoaded = [this, rotation](Core::WeakPointer<Core::Object3D> rootObject){
        rootObject->getTransform().rotate(0.0f, 1.0f, 0.0f, rotation, Core::TransformationSpace::World);
        Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
        Core::WeakPointer<Core::Scene> scene = engine->getActiveScene();
        Core::WeakPointer<Core::MeshContainer> firstMeshContainer;
        scene->visitScene(rootObject, [&firstMeshContainer](Core::WeakPointer<Core::Object3D> obj){

            Core::WeakPointer<Core::BaseRenderableContainer> baseRenderableContainer = obj->getBaseRenderableContainer();
            if (obj->getName() != "Scene.003") {
                 obj->setLayer(1);
            }

            if (baseRenderableContainer.isValid()) {
                Core::WeakPointer<Core::MeshContainer> meshContainer = Core::WeakPointer<Core::BaseRenderableContainer>::dynamicPointerCast<Core::MeshContainer>(baseRenderableContainer);
                if (meshContainer) {
                    obj->setStatic(true);
                    if (!firstMeshContainer.isValid()) {
                        firstMeshContainer = meshContainer;
                    }
                    Core::WeakPointer<Core::Object3DRenderer<Core::Mesh>> objectRenderer = Core::WeakPointer<Core::BaseObject3DRenderer>::dynamicPointerCast<Core::Object3DRenderer<Core::Mesh>>(obj->getBaseRenderer());
                    if (objectRenderer) {
                        Core::WeakPointer<Core::MeshRenderer> meshRenderer = Core::WeakPointer<Core::Object3DRenderer<Core::Mesh>>::dynamicPointerCast<Core::MeshRenderer>(objectRenderer);
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
            }
        });
    };

    this->modelerApp.loadModel("assets/models/terrain/terrain.fbx", .01f, 90 * Core::Math::DegreesToRads, true, true, usePhysicalMaterial, onLoaded);
}

void SceneHelper::loadWarrior(bool usePhysicalMaterial, float rotation, float x, float y, float z) {

   std::function<void(Core::WeakPointer<Core::Object3D>)> onLoaded = [this, rotation, x, y, z](Core::WeakPointer<Core::Object3D> rootObject){
        rootObject->getTransform().rotate(0.0f, 1.0f, 0.0f, rotation, Core::TransformationSpace::World);
        rootObject->getTransform().translate(x, y, z,  Core::TransformationSpace::World);

        Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
        Core::WeakPointer<Core::Scene> scene = engine->getActiveScene();
        Core::WeakPointer<Core::MeshContainer> firstMeshContainer;
        scene->visitScene(rootObject, [&firstMeshContainer](Core::WeakPointer<Core::Object3D> obj){

            Core::WeakPointer<Core::BaseRenderableContainer> baseRenderableContainer = obj->getBaseRenderableContainer();
            if (baseRenderableContainer.isValid()) {
                Core::WeakPointer<Core::MeshContainer> meshContainer = Core::WeakPointer<Core::BaseRenderableContainer>::dynamicPointerCast<Core::MeshContainer>(baseRenderableContainer);
                if (meshContainer) {
                    if (!firstMeshContainer.isValid()) {
                        firstMeshContainer = meshContainer;
                    }
                    Core::WeakPointer<Core::Object3DRenderer<Core::Mesh>> objectRenderer = Core::WeakPointer<Core::BaseObject3DRenderer>::dynamicPointerCast<Core::Object3DRenderer<Core::Mesh>>(obj->getBaseRenderer());
                    if (objectRenderer) {
                        Core::WeakPointer<Core::MeshRenderer> meshRenderer = Core::WeakPointer<Core::Object3DRenderer<Core::Mesh>>::dynamicPointerCast<Core::MeshRenderer>(objectRenderer);
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
            }
        });

        Core::WeakPointer<Core::Animation> animation = Core::Engine::instance()->getModelLoader().loadAnimation("assets/models/toonwarrior/animations/idle.fbx", false, true);
        Core::WeakPointer<Core::AnimationManager> animationManager = Core::Engine::instance()->getAnimationManager();
        Core::WeakPointer<Core::AnimationPlayer> animationPlayer = animationManager->retrieveOrCreateAnimationPlayer(firstMeshContainer->getSkeleton());
        animationPlayer->addAnimation(animation);
        animationPlayer->setSpeed(animation, 1.0f);
        animationPlayer->play(animation);
    };

    this->modelerApp.loadModel("assets/models/toonwarrior/character/warrior.fbx", 4.0f, 80 * Core::Math::DegreesToRads, false, false, usePhysicalMaterial, onLoaded);
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

    Core::WeakPointer<Core::Object3D> bottomSlabObj(engine->createObject3D());
    bottomSlabObj->setName("Base platform");
    Core::WeakPointer<Core::MeshRenderer> bottomSlabRenderer(engine->createRenderer<Core::MeshRenderer, Core::Mesh>(cubeMaterial, bottomSlabObj));
    Core::WeakPointer<Core::MeshContainer> bottomSlabMeshContainer = engine->createRenderableContainer<Core::MeshContainer, Core::Mesh>(bottomSlabObj);
    bottomSlabMeshContainer->addRenderable(slab);
    coreScene.addObjectToScene(bottomSlabObj);
    coreScene.addObjectToSceneRaycaster(bottomSlabObj, slab);
    bottomSlabObj->getTransform().getLocalMatrix().scale(15.0f, 1.0f, 15.0f);
    bottomSlabObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, -1.0f, 0.0f));
    bottomSlabObj->getTransform().getLocalMatrix().preRotate(0.0f, 1.0f, 0.0f,Core::Math::PI / 4.0f);
    bottomSlabObj->setStatic(true);
}

void SceneHelper::setupCommonSceneElements(bool excludeCastle) {
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    CoreScene& coreScene = this->modelerApp.getCoreScene();
    Core::WeakPointer<Core::Object3D> renderCameraObject = this->modelerApp.getRenderCameraObject();

    this->centerProbe = this->createSkyboxReflectionProbe(0.0f, 10.0f, 0.0f);

    const std::string bush1Path("assets/models/bush_5/bush_5.fbx");
    const std::string tree1Path("assets/models/tree_00/tree_00.fbx");
    const std::string tree3Path("assets/models/tree_03/tree_03.fbx");
    const std::string stone2Path("assets/models/stone_02/stone_02.fbx");
    const std::string stone4Path("assets/models/stone_04/stone_04.fbx");
    const std::string cliff1Path("assets/models/cliff_01/cliff_01.fbx");
    const std::string wellPath("assets/models/well/well.fbx");
    const std::string castle1Path("assets/models/castle/castle.fbx");

    std::function<void(Core::WeakPointer<Core::Object3D>)> dummyOnLoad = [](Core::WeakPointer<Core::Object3D> root){};

    this->loadWarrior(true, 0.0f, 45.4452f, 27.18f, -140.123f);
    this->loadTerrain(true, Core::Math::PI / 2.0f);
    if (!excludeCastle) this->loadModelStandard(castle1Path, true, false, 0.0f, Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 48.82f, 27.62f, -164.77f, 0.015f, 0.015f, 0.015f, false, 0.0f, 0.85f, false, 0, false, false, dummyOnLoad);

    // front left bushes
    this->loadModelStandard(bush1Path, true, false, 0.0f, Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 35.0f, 27.5136f, -135.0f, 0.01f, 0.01f, 0.01f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);
    this->loadModelStandard(bush1Path, true, false, 0.0f, 0.0f, 0.0f, 0, 1, 0, 0, 28.6463f, 27.5136, -137.331f, 0.015f, 0.015f, 0.015f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);
    this->loadModelStandard(bush1Path, true, false, 0.0f, Core::Math::PI / 2.0f, 0.0f, 0, 1, 0, 0, 23.0214f, 27.5136f, -141.079f, 0.01f, 0.01f, 0.01f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);


    // front right objects
    this->loadModelStandard(tree1Path, true, false, 0.0f, 0.174f, 0.0f, 0, 1, 0, 0, 68.91f, 26.5136f, -139.049f, 0.01f, 0.01f, 0.01f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);
    this->loadModelStandard(tree1Path, true, false, 0.0f, 0.0f, 0.0f, 0, 1, 0, 0, 74.83f, 27.5136f, -142.29f, 0.0075f, 0.0075f, 0.0075f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);
    this->loadModelStandard(tree3Path, true, false, 0.0f, 0.0f, 0.0f, 0, 1, 0, 0, 85.49f, 26.8536f, -126.29f, 0.0055f, 0.0055f, 0.0075f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);
    this->loadModelStandard(bush1Path, true, false, 0.0f, 0.0f, 0.0f, 0, 1, 0, 0, 78.36f, 27.5136f, -146.75f, 0.015f, 0.015f, 0.015f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);
    this->loadModelStandard(bush1Path, true, false, 0.0f, 0.0f,  0.0f, 0, 1, 0, 0, 63.26f, 27.5136f, -139.87f, 0.01f,  0.01f, 0.01f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);
    this->loadModelStandard(stone2Path, true, false, 0.0f, 2.0f, 0.0f, 0, 1, 0, 0, 79.5061, 25.6019f, -127.909f, 0.015f, 0.025f, 0.07f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);
    this->loadModelStandard(wellPath, true, false, 0.0f, 2.0f, 0.0f, 0, 1, 0, 0, 73.74, 27.11f, -154.55f, 0.02f, 0.02f, 0.02f, false, 0.0f, 0.85f, false, 1, false, false, dummyOnLoad);
    this->loadModelStandard(tree1Path, true, false, 0.0f, 0.174f, 0.0f, 0, 1, 0, 0, 93.14F, 26.69f, -138.61f, 0.0075f, 0.0075f, 0.0075f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);


    // cliffs
    this->loadModelStandard(stone4Path, true, false, 0.0f, 2.0f, 0.0f, 0, 1, 0, 0,  55.8991f, 27.178f, -140.469f, 0.015f, 0.025f, 0.01f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);
    this->loadModelStandard(cliff1Path, true, false, -0.2f, 0.0f, 0.0f, 0, 1, 0, 0, 54.098, 4.426f, -121.042f, 0.01f, 0.01f, 0.01f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);
    this->loadModelStandard(cliff1Path, true, true, -1.64f, 0.10983f, -.284439f, 0, 1, 0, 0, 29.1575, 13.3392f, -127.239f, 0.008f, 0.012f, 0.0065f, true, 0.0f, 0.85f, true, 1, true, true, dummyOnLoad);

    // random trees
    this->loadModelStandard(tree1Path, true, false, 0.0f, 0.174f, 0.0f, 0, 1, 0, 0, 23.9029f, 27.095f, -154.38f, 0.011f, 0.011f, 0.015f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);
    this->loadModelStandard(tree3Path, true, true, -1.585f, -0.0929f, 0.1719f, 0, 1, 0, 0, 23.9192f, 24.926f, -160.299f, 0.0075f, 0.0075f, 0.015f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);
    this->loadModelStandard(tree3Path, true, false, 0.0f, 0.174f, 0.0f, 0, 1, 0, 0, 23.817f, 26.264f, -182.373f, 0.005f, 0.005f, 0.01f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);
    this->loadModelStandard(tree3Path, true, false, 0.0f, 0.174f, 0.0f, 0, 1, 0, 0, 18.8396f, 17.9282f, -130.676f, 0.005f, 0.005f, 0.01f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);
    this->loadModelStandard(tree1Path, true, false, 0.0f, 0.174f, 0.0f, 0, 1, 0, 0, .940061f, 26.69f, -132.04f, 0.0075f, 0.0075f, 0.0075f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);
    this->loadModelStandard(tree1Path, true, false, 0.0f, 0.174f, 0.0f, 0, 1, 0, 0, 5.8642f, 49.927f, -211.811f, 0.0125f, 0.0145f, 0.0125f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);
    this->loadModelStandard(tree3Path, true, false, 0.0f, 0.0f, 0.0f, 0, 1, 0, 0, 13.78f, 49.927f, -211.5f, 0.0095f, 0.0095f, 0.0165f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);
    this->loadModelStandard(tree1Path, true, false, 0.0f, 2.0f, 0.0f, 0, 1, 0, 0, 56.367f, 56.553f, -211.185f, 0.0115f, 0.0115f, 0.0135f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);
    this->loadModelStandard(tree1Path, true, false, 0.0f, 2.0f, 0.0f, 0, 1, 0, 0, 86.006f, 11.7081f, -119.096f, 0.0115f, 0.0115f, 0.0135f, true, 0.0f, 0.85f, true, 4, true, true, dummyOnLoad);

    renderCameraObject->getTransform().rotate(0.0f, 1.0f, 0.0f, Core::Math::PI * .8, Core::TransformationSpace::World);
    this->modelerApp.setCameraPosition(48.82f, 45.62f, -104.77f);
}
