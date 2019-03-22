#include "SceneHelper.h"

#include "ModelerApp.h"

#include "Core/Engine.h"
#include "Core/material/StandardPhysicalMaterial.h"
#include "Core/render/MeshRenderer.h"
#include "Core/scene/Object3D.h"
#include "Core/scene/Scene.h"
#include "Core/light/AmbientIBLLight.h"

SceneHelper::SceneHelper(ModelerApp& modelerApp): modelerApp(modelerApp) {

}
Core::WeakPointer<Core::ReflectionProbe> SceneHelper::createSkyboxReflectionProbe(float x, float y, float z) {
    Core::WeakPointer<Core::Camera> renderCamera = this->modelerApp.getRenderCamera();
    Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
    CoreScene& coreScene = this->modelerApp.getCoreScene();

    Core::WeakPointer<Core::Object3D> reflectionProbeObject = engine->createObject3D();
    reflectionProbeObject->setName("Reflection probe");
    Core::WeakPointer<Core::ReflectionProbe> reflectionProbe = engine->createReflectionProbe(reflectionProbeObject);
    reflectionProbe->setNeedsUpdate(true);
    reflectionProbeObject->getTransform().getLocalMatrix().translate(x, y, z);
    coreScene.addObjectToScene(reflectionProbeObject);
    reflectionProbe->setSkybox(renderCamera->getSkybox());
    reflectionProbe->setSkyboxOnly(true);
    Core::WeakPointer<Core::AmbientIBLLight> iblLight = engine->createLight<Core::AmbientIBLLight>(reflectionProbeObject);
    return reflectionProbe;
}

void SceneHelper::loadWarrior(bool usePhysicalMaterial, float rotation) {
    this->modelerApp.loadModel("Assets/models/toonwarrior/character/warrior.fbx", .075f, 80, true, usePhysicalMaterial, [this, rotation](Core::WeakPointer<Core::Object3D> rootObject){
        rootObject->getTransform().rotate(0.0f, 1.0f, 0.f, rotation, Core::TransformationSpace::World);
        rootObject->getTransform().translate(0.0f, 0.0f, -11.0f, Core::TransformationSpace::World);

        Core::WeakPointer<Core::Engine> engine = this->modelerApp.getEngine();
        Core::WeakPointer<Core::Scene> scene = engine->getActiveScene();
        scene->visitScene(rootObject, [this, &rootObject](Core::WeakPointer<Core::Object3D> obj){

            Core::WeakPointer<Core::RenderableContainer<Core::Mesh>> meshContainer = Core::WeakPointer<Core::Object3D>::dynamicPointerCast<Core::RenderableContainer<Core::Mesh>>(obj);
            if (meshContainer) {
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
                                physicalMaterial->setRoughness(0.5f);
                            }
                            else {
                                physicalMaterial->setMetallic(0.85f);
                                physicalMaterial->setRoughness(0.3f);
                            }
                        }
                    }
                }
            }
        });
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

    Core::WeakPointer<Core::RenderableContainer<Core::Mesh>> bottomSlabObj(engine->createObject3D<Core::RenderableContainer<Core::Mesh>>());
    bottomSlabObj->setName("Base platform");
    Core::WeakPointer<Core::MeshRenderer> bottomSlabRenderer(engine->createRenderer<Core::MeshRenderer>(cubeMaterial, bottomSlabObj));
    bottomSlabObj->addRenderable(slab);
    coreScene.addObjectToScene(bottomSlabObj);
    coreScene.addObjectToSceneRaycaster(bottomSlabObj, slab);
    bottomSlabObj->getTransform().getLocalMatrix().scale(15.0f, 1.0f, 15.0f);
    bottomSlabObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, -1.0f, 0.0f));
    bottomSlabObj->getTransform().getLocalMatrix().preRotate(0.0f, 1.0f, 0.0f,Core::Math::PI / 4.0f);
}
