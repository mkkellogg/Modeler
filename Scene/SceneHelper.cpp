#include "SceneHelper.h"

#include "ModelerApp.h"

#include "Core/Engine.h"
#include "Core/material/StandardPhysicalMaterial.h"
#include "Core/material/StandardPhysicalMaterialMultiLight.h"
#include "Core/render/MeshRenderer.h"
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
    return reflectionProbe;
}

void SceneHelper::loadGun(float rotation, float x, float y, float z) {
    this->modelerApp.loadModel("assets/models/gun/gun.fbx", .25f, 80 * Core::Math::DegreesToRads, true, true, true, [this, rotation, x, y, z](Core::WeakPointer<Core::Object3D> rootObject){
        Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
        rootObject->getTransform().rotate(0.0f, 1.0f, 0.f, rotation, Core::TransformationSpace::World);
        rootObject->getTransform().translate(x, y, z, Core::TransformationSpace::World);

        Core::TextureAttributes texAttributes;
        texAttributes.FilterMode = Core::TextureFilter::TriLinear;
        texAttributes.MipLevels = 6;
        texAttributes.WrapMode = Core::TextureWrap::Clamp;
        texAttributes.Format = Core::TextureFormat::RGBA8;

        Core::TextureAttributes texAttributesSingleChannel;
        texAttributesSingleChannel.FilterMode = Core::TextureFilter::TriLinear;
        texAttributesSingleChannel.MipLevels = 6;
        texAttributesSingleChannel.WrapMode = Core::TextureWrap::Clamp;
        texAttributesSingleChannel.Format = Core::TextureFormat::R32F;

        std::shared_ptr<Core::StandardImage> normalImage = Core::ImageLoader::loadImageU("assets/models/gun/Cerberus_N.tga");
        Core::WeakPointer<Core::Texture2D> normalMap = engine->getGraphicsSystem()->createTexture2D(texAttributes);
        normalMap->buildFromImage(normalImage);

        std::shared_ptr<Core::StandardImage> roughnessImage = Core::ImageLoader::loadImageU("assets/models/gun/Cerberus_R.tga");
        Core::WeakPointer<Core::Texture2D> roughnessMap = engine->getGraphicsSystem()->createTexture2D(texAttributes);
        roughnessMap->buildFromImage(roughnessImage);

        std::shared_ptr<Core::StandardImage> metallicImage = Core::ImageLoader::loadImageU("assets/models/gun/Cerberus_M.tga");
        Core::WeakPointer<Core::Texture2D> metallicMap = engine->getGraphicsSystem()->createTexture2D(texAttributes);
        metallicMap->buildFromImage(metallicImage);

        Core::WeakPointer<Core::Scene> scene = engine->getActiveScene();
        scene->visitScene(rootObject, [this, &rootObject, &normalMap, &roughnessMap, &metallicMap](Core::WeakPointer<Core::Object3D> obj){

            Core::WeakPointer<Core::MeshContainer> meshContainer = Core::WeakPointer<Core::Object3D>::dynamicPointerCast<Core::MeshContainer>(obj);
            if (meshContainer) {
                Core::WeakPointer<Core::ObjectRenderer<Core::Mesh>> objectRenderer = meshContainer->getRenderer();
                if (objectRenderer) {
                    Core::WeakPointer<Core::MeshRenderer> meshRenderer = Core::WeakPointer<Core::ObjectRenderer<Core::Mesh>>::dynamicPointerCast<Core::MeshRenderer>(objectRenderer);
                    if (meshRenderer) {
                        Core::WeakPointer<Core::Material> renderMaterial = meshRenderer->getMaterial();
                        Core::WeakPointer<Core::StandardPhysicalMaterial> physicalMaterial = Core::WeakPointer<Core::Material>::dynamicPointerCast<Core::StandardPhysicalMaterial>(renderMaterial);
                        if (physicalMaterial) {
                            physicalMaterial->setNormalMapEnabled(true);
                            physicalMaterial->setNormalMap(normalMap);
                            physicalMaterial->setRoughnessMapEnabled(true);
                            physicalMaterial->setRoughnessMap(roughnessMap);
                            physicalMaterial->setMetallicMapEnabled(true);
                            physicalMaterial->setMetallicMap(metallicMap);
                        }
                        Core::WeakPointer<Core::Mesh> mesh = meshContainer->getRenderables()[0];
                    }
                }
            }
        });


    });
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

void SceneHelper::loadModelStandard(const std::string& path, bool usePhysicalMaterial, float rotation, float x, float y, float z, float scale,
                                    bool singlePassMultiLight, float metallic, float roughness, bool transparent, bool customShadowRendering, bool customName) {
    this->modelerApp.loadModel(path, scale, 80 * Core::Math::DegreesToRads, true, true, usePhysicalMaterial,
                               [this, rotation, x, y, z, singlePassMultiLight, metallic, roughness, transparent, customShadowRendering, customName](Core::WeakPointer<Core::Object3D> rootObject){
        rootObject->getTransform().rotate(0.0f, 1.0f, 0.0f, rotation, Core::TransformationSpace::World);
        rootObject->getTransform().translate(x, y, z,  Core::TransformationSpace::World);
        Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
        Core::WeakPointer<Core::Scene> scene = engine->getActiveScene();
        Core::WeakPointer<Core::MeshContainer> firstMeshContainer;
        scene->visitScene(rootObject, [this, &rootObject, &firstMeshContainer, &engine, singlePassMultiLight, metallic, roughness, transparent, customShadowRendering, customName](Core::WeakPointer<Core::Object3D> obj){

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
                        }
                        if (customShadowRendering) {
                            renderMaterial->setCustomDepthOutput(true);
                            renderMaterial->setCustomDepthOutputCopyOverrideMatrialState(true);
                        }
                        if (customName) {
                            Core::WeakPointer<Core::Mesh> mesh = meshContainer->getRenderables()[0];
                            mesh->setName("Bush");
                            meshContainer->setName("Bush");
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
                        //Core::WeakPointer<Core::Mesh> mesh = meshContainer->getRenderables()[0];
                       // mesh->setNormalsSmoothingThreshold(Core::Math::PI / 1.5f);
                        //mesh->update();
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
                        Core::WeakPointer<Core::Mesh> mesh = meshContainer->getRenderables()[0];
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

    this->loadWarrior(true, 0.0f, 48.82f, 27.18f, -138.77f);
    this->loadTerrain(true, Core::Math::PI / 2.0f, 0.0f, 0.0f, 0.0f);
    this->loadModelStandard("assets/models/castle/castle.fbx", true, Core::Math::PI / 2.0f, 48.82f, 27.62f, -164.77f, 0.015f, false, 0.0f, 0.85f, false, false);
    this->loadModelStandard("assets/models/bush_5/bush_5.fbx", true, Core::Math::PI / 2.0f, 67.5836, 27.6141, -141.718, 0.01f, true, 0.0f, 0.85f, true, true, true);

    renderCameraObject->getTransform().rotate(0.0f, 1.0f, 0.0f, Core::Math::PI * .8, Core::TransformationSpace::World);
    this->modelerApp.setCameraPosition(48.82f, 45.62f, -104.77f);
}
