#include "ModelerApp.h"
#include "RenderWindow.h"
#include "SceneUtils.h"
#include "KeyboardAdapter.h"
#include "Scene/CornfieldScene.h"
#include "Scene/RedSkyScene.h"
#include "Util/FileUtil.h"

#include "Core/util/Time.h"
#include "Core/scene/Scene.h"
#include "Core/common/types.h"
#include "Core/math/Math.h"
#include "Core/math/Matrix4x4.h"
#include "Core/math/Quaternion.h"
#include "Core/render/Camera.h"
#include "Core/color/Color.h"
#include "Core/color/IntColor.h"
#include "Core/material/StandardAttributes.h"
#include "Core/geometry/Mesh.h"
#include "Core/geometry/Vector2.h"
#include "Core/geometry/Vector3.h"
#include "Core/render/RenderableContainer.h"
#include "Core/render/MeshRenderer.h"
#include "Core/render/RenderTarget.h"
#include "Core/material/BasicCubeMaterial.h"
#include "Core/material/BasicMaterial.h"
#include "Core/material/BasicTexturedMaterial.h"
#include "Core/material/BasicLitMaterial.h"
#include "Core/material/StandardAttributes.h"
#include "Core/material/SkyboxMaterial.h"
#include "Core/material/StandardPhysicalMaterial.h"
#include "Core/image/RawImage.h"
#include "Core/image/CubeTexture.h"
#include "Core/image/Texture2D.h"
#include "Core/image/RawImage.h"
#include "Core/image/ImagePainter.h"
#include "Core/image/TextureUtils.h"
#include "Core/util/WeakPointer.h"
#include "Core/asset/ModelLoader.h"
#include "Core/material/BasicTexturedMaterial.h"
#include "Core/geometry/GeometryUtils.h"
#include "Core/light/PointLight.h"
#include "Core/light/AmbientLight.h"
#include "Core/light/AmbientIBLLight.h"
#include "Core/light/DirectionalLight.h"
#include "Core/scene/Transform.h"
#include "Core/scene/TransformationSpace.h"
#include "Core/render/RenderTargetCube.h"
#include "Core/render/RenderTarget.h"

using MeshContainer = Core::MeshContainer;

ModelerApp::ModelerApp(): renderWindow(nullptr) {

}

ModelerApp::~ModelerApp(){
    if (this->scene.isValid()) Core::Engine::safeReleaseObject(this->scene);
}

void ModelerApp::init() {

}

void ModelerApp::setRenderWindow(RenderWindow* renderWindow) {
    if (this->renderWindow != renderWindow) {
        this->renderWindow = renderWindow;
        RenderWindow::LifeCycleEventCallback onRenderWindowInit = [this](RenderWindow* renderWindow) {
            this->engine = renderWindow->getEngine();
            this->coreSync = std::make_shared<CoreSync>();
            this->engineReady(engine);
            this->orbitControls = std::make_shared<OrbitControls>(this->engine, this->renderCamera, this->coreSync);

            std::shared_ptr<MouseAdapter> mouseAdapter = std::make_shared<MouseAdapter>();
            this->renderWindow->setMouseAdapter(mouseAdapter);
            mouseAdapter->onMouseButtonPressed(std::bind(&ModelerApp::mouseButton, this, std::placeholders::_1,  std::placeholders::_2,  std::placeholders::_3, std::placeholders::_4));
            mouseAdapter->onMouseButtonReleased(std::bind(&ModelerApp::mouseButton, this, std::placeholders::_1,  std::placeholders::_2,  std::placeholders::_3, std::placeholders::_4));

            this->pipedGestureAdapter = std::make_shared<PipedEventAdapter<GestureAdapter::GestureEvent>>(std::bind(&ModelerApp::gesture, this, std::placeholders::_1));
            this->gestureAdapter = std::make_shared<GestureAdapter>();
            this->gestureAdapter->setPipedEventAdapter(this->pipedGestureAdapter);
            this->gestureAdapter->setMouseAdapter(*(mouseAdapter.get()));
        };
        renderWindow->onInit(onRenderWindowInit);
    }
}

void ModelerApp::loadModel(const std::string& path, float scale, float smoothingThreshold, bool zUp, bool preserveFBXPivots, bool usePhysicalMaterial, ModelerAppLoadModelCallback callback) {
   if (this->engineIsReady) {
        std::string sPath = path;
        sPath = FileUtil::removePrefix(sPath, "file://");
        std::string abbrevName = FileUtil::extractFileNameFromPath(sPath, true);

        if (smoothingThreshold < 0 ) smoothingThreshold = 0;
        if (smoothingThreshold >= 90) smoothingThreshold = 90;

        CoreSync::Runnable runnable = [this, sPath, scale, smoothingThreshold, zUp, preserveFBXPivots, usePhysicalMaterial, abbrevName, callback](Core::WeakPointer<Core::Engine> engine) {
           Core::ModelLoader& modelLoader = engine->getModelLoader();
           Core::WeakPointer<Core::Object3D> rootObject = modelLoader.loadModel(sPath, scale, smoothingThreshold, true, true, preserveFBXPivots, usePhysicalMaterial);

           Core::WeakPointer<Core::Object3D> newRoot = engine->createObject3D();
           newRoot->getTransform().getLocalMatrix().copy(rootObject->getTransform().getLocalMatrix());
           newRoot->addChild(rootObject);
           rootObject->getTransform().getLocalMatrix().setIdentity();
           rootObject = newRoot;

           this->coreScene.addObjectToScene(rootObject);
           rootObject->setName(abbrevName);
           Core::WeakPointer<Core::Scene> scene = engine->getActiveScene();
           scene->visitScene(rootObject, [this, rootObject](Core::WeakPointer<Core::Object3D> obj){
               Core::WeakPointer<Core::MeshContainer> meshContainer =
                       Core::WeakPointer<Core::Object3D>::dynamicPointerCast<Core::MeshContainer>(obj);
               if (meshContainer) {
                   const std::vector<Core::PersistentWeakPointer<Core::Mesh>>& meshes = meshContainer->getRenderables();
                   for (Core::WeakPointer<Core::Mesh> mesh : meshes) {
                       this->coreScene.addObjectToSceneRaycaster(obj, mesh);
                   }
               }
           });

           if (zUp) {
               rootObject->getTransform().rotate(1.0f, 0.0f, 0.0f, -Core::Math::PI / 2.0);
           }

           callback(rootObject);
       };
       this->coreSync->run(runnable);
   }
}

void ModelerApp::loadAnimation(const std::string& path, bool addLoopPadding, bool preserveFBXPivots, ModelerAppLoadAnimationCallback callback) {
    if (this->engineIsReady) {
         std::string sPath = path;
         sPath = FileUtil::removePrefix(sPath, "file://");
         CoreSync::Runnable runnable = [this, sPath, addLoopPadding, preserveFBXPivots, callback](Core::WeakPointer<Core::Engine> engine) {
            Core::ModelLoader& modelLoader = engine->getModelLoader();
            Core::WeakPointer<Core::Animation> animation = modelLoader.loadAnimation(sPath, addLoopPadding, preserveFBXPivots);
            callback(animation);
        };
        this->coreSync->run(runnable);
    }
}

CoreScene& ModelerApp::getCoreScene() {
    return this->coreScene;
}

void ModelerApp::onUpdate(ModelerAppLifecycleEventCallback callback) {
    QMutexLocker ml(&this->onUpdateMutex);
    this->onUpdates.push_back(callback);
}

std::shared_ptr<CoreSync> ModelerApp::getCoreSync() {
    return this->coreSync;
}

bool ModelerApp::isSceneObjectHidden(Core::WeakPointer<Core::Object3D> object) {
    return this->hiddenSceneObjects[object->getObjectID()];
}

void ModelerApp::setSceneObjectHidden(Core::WeakPointer<Core::Object3D> object, bool hidden) {
    this->hiddenSceneObjects[object->getObjectID()] = hidden;
}

Core::WeakPointer<Core::Camera> ModelerApp::getRenderCamera() {
    return this->renderCamera;
}

Core::WeakPointer<Core::Engine> ModelerApp::getEngine() {
    return this->engine;
}

void ModelerApp::setTransformModeTranslation() {
    this->transformWidget.activateTranslationMode();
}

void ModelerApp::setTransformModeRotation() {
    this->transformWidget.activateRotationMode();
}

void ModelerApp::engineReady(Core::WeakPointer<Core::Engine> engine) {

    this->engineIsReady = true;
    this->scene = engine->createScene();
    engine->setActiveScene(this->scene);
    this->coreScene.setEngine(engine);
    this->coreScene.setSceneRoot(this->scene->getRoot());
    engine->getGraphicsSystem()->setClearColor(Core::Color(0,0,0,1));
    this->setupRenderCamera();

    this->loadScene(0);

    this->transformWidget.init(this->renderCamera);
    this->setupHighlightMaterials();

    this->coreScene.onSelectedObjectAdded([this](Core::WeakPointer<Core::Object3D> selectedObject){
        if (selectedObject) {
            this->transformWidget.addTargetObject(selectedObject);
        }
    });

    this->coreScene.onSelectedObjectRemoved([this](Core::WeakPointer<Core::Object3D> deselectedObject){
        if (deselectedObject) {
            this->transformWidget.removeTargetObject(deselectedObject);
        }
    });

    engine->onPreRender([this]() {
        this->preRenderCallback();
    }, true);

    engine->onPostRender([this]() {
        this->postRenderCallback();
    }, true);

    engine->onUpdate([this]() {
        auto vp = this->engine->getGraphicsSystem()->getCurrentRenderTarget()->getViewport();
        this->renderCamera->setAspectRatioFromDimensions(vp.z, vp.w);
        this->resolveOnUpdateCallbacks();
        this->modelerScene->update();
    }, true);

    this->basicTextureMaterial = this->engine->createMaterial<Core::BasicTexturedFullScreenQuadMaterial>();
}

void ModelerApp::setupRenderCamera() {

    Core::WeakPointer<Core::Object3D> cameraObj = this->engine->createObject3D<Core::Object3D>();
    cameraObj->setName("Main camera");
    this->renderCamera = this->engine->createPerspectiveCamera(cameraObj, Core::Camera::DEFAULT_FOV, Core::Camera::DEFAULT_ASPECT_RATIO, 0.1f, 500);
    this->renderCamera->setSSAOEnabled(true);
    this->renderCamera->setSSAORadius(2.0f);
    this->renderCamera->setSSAOBias(0.001f);
    this->coreScene.addObjectToScene(cameraObj);
    this->setSceneObjectHidden(cameraObj, true);

    Core::Quaternion qA = Core::Quaternion::fromAngleAxis(0.0, 0, 1, 0);
    Core::Matrix4x4 worldMatrix;
    worldMatrix.multiply(qA.rotationMatrix());
    worldMatrix.translate(0, 0, 12);
    worldMatrix.translate(0, 5, 0);

}

void ModelerApp::loadScene(int scene) {
    switch(scene) {
        case 0:
        {
           std::shared_ptr<RedSkyScene> redSkyScene = std::make_shared<RedSkyScene>(*this);
           redSkyScene->load();
           this->modelerScene = redSkyScene;
        }
        break;
        case 1:
        {
           std::shared_ptr<CornfieldScene> cornfieldScene = std::make_shared<CornfieldScene>(*this);
           cornfieldScene->load();
           this->modelerScene = cornfieldScene;

        }
        break;
        default:
        {
            std::shared_ptr<RedSkyScene> redSkyScene = std::make_shared<RedSkyScene>(*this);
            redSkyScene->load();
            this->modelerScene = redSkyScene;
        }
        break;
    }
}

void ModelerApp::setupHighlightMaterials() {
    this->highlightColor.set(1.0, 0.65, 0.0, 0.35);
    this->outlineColor.set(1.0, 0.65, 0.0, 1.0);
    this->darkOutlineColor.set(this->outlineColor.r * .5, this->outlineColor.g * .5, this->outlineColor.b * .5, 1.0);

    this->highlightMaterial = this->engine->createMaterial<Core::BasicColoredMaterial>();
    this->highlightMaterial->setBlendingMode(Core::RenderState::BlendingMode::Custom);
    this->highlightMaterial->setSourceBlendingFactor(Core::RenderState::BlendingFactor::SrcAlpha);
    this->highlightMaterial->setDestBlendingFactor(Core::RenderState::BlendingFactor::OneMinusSrcAlpha);
    this->highlightMaterial->setLit(false);
    this->highlightMaterial->setZOffset(-.00005f);
    this->highlightMaterial->setObjectColor(highlightColor);

    this->outlineMaterial = this->engine->createMaterial<Core::OutlineMaterial>();
    this->outlineMaterial->setLit(false);
    this->outlineMaterial->setOutlineColor(outlineColor);
    this->outlineMaterial->setEdgeWidth(.01f);
    this->outlineMaterial->setAbsExtend(.004f);
    this->outlineMaterial->setBlendingMode(Core::RenderState::BlendingMode::Custom);
    this->outlineMaterial->setSourceBlendingFactor(Core::RenderState::BlendingFactor::SrcAlpha);
    this->outlineMaterial->setDestBlendingFactor(Core::RenderState::BlendingFactor::OneMinusSrcAlpha);
}

void ModelerApp::preRenderCallback() {
    this->transformWidget.updateTransformationForTargetObjects();
}

void ModelerApp::postRenderCallback() {
    const std::vector<Core::WeakPointer<Core::Object3D>>& selectedObjects = this->coreScene.getSelectedObjects();
    if (selectedObjects.size() > 0 ) {

        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Color, false);
        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Depth, false);
        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, false);

        Core::Bool isSkyboxEnabled = this->renderCamera->isSkyboxEnabled();
        Core::Bool isHDREnabled = this->renderCamera->isHDREnabled();
        this->renderCamera->setSkyboxEnabled(false);
        this->renderCamera->setHDREnabled(false);

        this->highlightMaterial->setStencilTestEnabled(false);
        this->highlightMaterial->setStencilWriteMask(0x00);
        this->highlightMaterial->setFaceCullingEnabled(true);
        this->highlightMaterial->setColorWriteEnabled(true);
        this->highlightMaterial->setDepthTestEnabled(true);
        this->highlightMaterial->setDepthWriteEnabled(true);
        this->renderOnce(selectedObjects, this->renderCamera, this->highlightMaterial);

        this->engine->getGraphicsSystem()->clearActiveRenderTarget(false, false, true);
        this->highlightMaterial->setStencilWriteMask(0xFF);
        this->highlightMaterial->setStencilReadMask(0xFF);
        this->highlightMaterial->setStencilRef(1);
        this->highlightMaterial->setStencilTestEnabled(true);
        this->highlightMaterial->setStencilComparisonFunction(Core::RenderState::StencilFunction::Always);
        this->highlightMaterial->setStencilFailActionStencil(Core::RenderState::StencilAction::Keep);
        this->highlightMaterial->setStencilFailActionDepth(Core::RenderState::StencilAction::Keep);
        this->highlightMaterial->setStencilAllPassAction(Core::RenderState::StencilAction::Replace);
        this->highlightMaterial->setFaceCullingEnabled(false);
        this->highlightMaterial->setColorWriteEnabled(false);
        this->highlightMaterial->setDepthTestEnabled(false);
        this->highlightMaterial->setDepthWriteEnabled(true);
        this->renderOnce(selectedObjects, this->renderCamera, this->highlightMaterial);

        this->outlineMaterial->setStencilWriteMask(0x00);
        this->outlineMaterial->setStencilReadMask(0xFF);
        this->outlineMaterial->setStencilRef(1);
        this->outlineMaterial->setStencilTestEnabled(true);
        this->outlineMaterial->setStencilComparisonFunction(Core::RenderState::StencilFunction::NotEqual);
        this->outlineMaterial->setStencilFailActionStencil(Core::RenderState::StencilAction::Keep);
        this->outlineMaterial->setStencilFailActionDepth(Core::RenderState::StencilAction::Keep);
        this->outlineMaterial->setStencilAllPassAction(Core::RenderState::StencilAction::Replace);
        this->outlineMaterial->setFaceCullingEnabled(false);
        this->outlineMaterial->setOutlineColor(this->outlineColor);
        this->outlineMaterial->setColorWriteEnabled(true);
        this->outlineMaterial->setDepthWriteEnabled(false);
        this->outlineMaterial->setDepthTestEnabled(true);
        this->outlineMaterial->setDepthFunction(Core::RenderState::DepthFunction::LessThanOrEqual);
        this->renderOnce(selectedObjects, this->renderCamera, this->outlineMaterial);

        this->outlineMaterial->setOutlineColor(this->darkOutlineColor);
        this->outlineMaterial->setDepthFunction(Core::RenderState::DepthFunction::GreaterThanOrEqual);
        this->renderOnce(selectedObjects, this->renderCamera, this->outlineMaterial);

        this->renderCamera->setSkyboxEnabled(isSkyboxEnabled);
        this->renderCamera->setHDREnabled(isHDREnabled);
        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Color, true);
        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Depth, true);
        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, true);

        this->transformWidget.updateCamera();
        this->transformWidget.render();
    }

    this->basicTextureMaterial->setTexture( this->engine->getGraphicsSystem()->getRenderer()->getSSAOTexture());
    //this->engine->getGraphicsSystem()->renderFullScreenQuad(this->engine->getGraphicsSystem()->getDefaultRenderTarget(), -1, this->basicTextureMaterial);

    this->frameCount++;
}

void ModelerApp::gesture(GestureAdapter::GestureEvent event) {
    if (this->engineIsReady) {
        GestureAdapter::GestureEventType eventType = event.getType();
        switch(eventType) {
            case GestureAdapter::GestureEventType::Move:
                this->transformWidget.rayCastForSelection(event.end.x, event.end.y);
            break;
            case GestureAdapter::GestureEventType::Drag:
            case GestureAdapter::GestureEventType::Scroll:
                if (!this->transformWidget.handleDrag(event.end.x, event.end.y)) {
                    this->orbitControls->handleGesture(event);
                }
            break;
        }
    }
}

void ModelerApp::mouseButton(MouseAdapter::MouseEventType type, Core::UInt32 button, Core::Int32 x, Core::Int32 y) {
    switch(type) {
        case MouseAdapter::MouseEventType::ButtonPress:
            this->orbitControls->resetMove();
            if (button == 1) {
                if (!this->transformWidget.startAction(x, y)) {
                    this->coreScene.rayCastForObjectSelection(this->renderCamera, x, y, true, KeyboardAdapter::isModifierActive(KeyboardAdapter::Modifier::Ctrl));
                }
            }
        break;
        case MouseAdapter::MouseEventType::ButtonRelease:
            if (button == 1) this->transformWidget.endAction(x, y);
        break;
    }
}

void ModelerApp::resolveOnUpdateCallbacks() {
    QMutexLocker ml(&this->onUpdateMutex);
    for (ModelerAppLifecycleEventCallback callback : this->onUpdates) {
        callback();
    }
}

void ModelerApp::renderOnce(const std::vector<Core::WeakPointer<Core::Object3D>>& objects, Core::WeakPointer<Core::Camera> camera, Core::WeakPointer<Core::Material> material) {
    Core::WeakPointer<Core::Renderer> renderer =  this->engine->getGraphicsSystem()->getRenderer();
    static std::vector<Core::WeakPointer<Core::Object3D>> roots;
    roots.resize(0);
    SceneUtils::getRootObjects(objects, roots);
    for (unsigned int i = 0; i < roots.size(); i++) {
        Core::WeakPointer<Core::Object3D> object = roots[i];
        renderer->renderObjectBasic(object, camera, material);
    }
}
