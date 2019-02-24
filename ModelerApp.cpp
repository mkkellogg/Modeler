#include "ModelerApp.h"
#include "RenderWindow.h"
#include "GeometryUtils.h"

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
#include "Core/material/BasicLitMaterial.h"
#include "Core/material/StandardAttributes.h"
#include "Core/image/RawImage.h"
#include "Core/image/CubeTexture.h"
#include "Core/image/Texture2D.h"
#include "Core/util/WeakPointer.h"
#include "Core/asset/ModelLoader.h"
#include "Core/image/RawImage.h"
#include "Core/image/ImagePainter.h"
#include "Core/material/BasicTexturedMaterial.h"
#include "Core/geometry/GeometryUtils.h"
#include "Core/light/PointLight.h"
#include "Core/light/AmbientLight.h"
#include "Core/light/DirectionalLight.h"
#include "Core/scene/Transform.h"
#include "Core/scene/TransformationSpace.h"

using MeshContainer = Core::RenderableContainer<Core::Mesh>;

ModelerApp::ModelerApp(): renderWindow(nullptr) {

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

void ModelerApp::loadModel(const std::string& path, float scale, float smoothingThreshold, const bool zUp) {
   if (this->engineIsReady) {
       std::string sPath = path;
       std::string filePrefix("file://");
       std::string pathPrefix = sPath.substr(0, 7) ;
       if (pathPrefix == filePrefix) {
           sPath = sPath.substr(7);
       }

       if (smoothingThreshold < 0 ) smoothingThreshold = 0;
       if (smoothingThreshold >= 90) smoothingThreshold = 90;

       CoreSync::Runnable runnable = [this, sPath, scale, smoothingThreshold, zUp](Core::WeakPointer<Core::Engine> engine) {
           Core::ModelLoader& modelLoader = engine->getModelLoader();
           Core::WeakPointer<Core::Object3D> rootObject = modelLoader.loadModel(sPath, scale, smoothingThreshold, false, false, true);
           this->coreScene.addObjectToScene(rootObject);

           Core::WeakPointer<Core::Scene> scene = engine->getActiveScene();
           scene->visitScene(rootObject, [this, &rootObject](Core::WeakPointer<Core::Object3D> obj){
               Core::WeakPointer<Core::RenderableContainer<Core::Mesh>> meshContainer =
                       Core::WeakPointer<Core::Object3D>::dynamicPointerCast<Core::RenderableContainer<Core::Mesh>>(obj);
               if (meshContainer) {
                   std::vector<Core::WeakPointer<Core::Mesh>> meshes = meshContainer->getRenderables();
                   for (Core::WeakPointer<Core::Mesh> mesh : meshes) {
                       this->addObjectToSceneRaycaster(obj, mesh);
                   }
               }
           });

           if (zUp) {
               rootObject->getTransform().rotate(1.0f, 0.0f, 0.0f, -Core::Math::PI / 2.0);
           }
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

void ModelerApp::engineReady(Core::WeakPointer<Core::Engine> engine) {

    this->engineIsReady = true;

    Core::WeakPointer<Core::Scene> scene(engine->createScene());
    engine->setActiveScene(scene);
    this->coreScene.setSceneRoot(scene->getRoot());
    engine->getGraphicsSystem()->setClearColor(Core::Color(0,0,0,1));

    this->setupRenderCamera();
    this->setupDefaultObjects();
    this->transformWidget.init(this->renderCamera, this->coreScene);
    this->setupLights();
    this->setupHighlightMaterials();

    engine->onPreRender([this]() {
        this->preRenderCallback();
    }, true);

    engine->onPostRender([this]() {
        this->postRenderCallback();
    }, true);

    engine->onUpdate([this]() {
        auto vp = Core::Engine::instance()->getGraphicsSystem()->getCurrentRenderTarget()->getViewport();
        this->renderCamera->setAspectRatioFromDimensions(vp.z, vp.w);
        this->resolveOnUpdateCallbacks();
    }, true);

}

void ModelerApp::setupRenderCamera() {

    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();

    Core::WeakPointer<Core::Object3D> cameraObj = engine->createObject3D<Core::Object3D>();
    cameraObj->setName("Main camera");
    this->renderCamera = engine->createPerspectiveCamera(cameraObj, Core::Camera::DEFAULT_FOV, Core::Camera::DEFAULT_ASPECT_RATIO, 0.1f, 100);
    this->coreScene.addObjectToScene(cameraObj);

    Core::Quaternion qA;
    qA.fromAngleAxis(0.0, 0, 1, 0);
    Core::Matrix4x4 rotationMatrixA;
    qA.rotationMatrix(rotationMatrixA);

    Core::Matrix4x4 worldMatrix;
    worldMatrix.multiply(rotationMatrixA);
    worldMatrix.translate(0, 0, 12);
    worldMatrix.translate(0, 5, 0);

    cameraObj->getTransform().getLocalMatrix().copy(worldMatrix);
    cameraObj->getTransform().translate(5, 0, 0);
    cameraObj->getTransform().updateWorldMatrix();
    cameraObj->getTransform().lookAt(Core::Point3r(0, 0, 0));
}

void ModelerApp::setupDefaultObjects() {
    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();

    Core::WeakPointer<Core::BasicLitMaterial> cubeMaterial = engine->createMaterial<Core::BasicLitMaterial>();
    Core::Color slabColor(0.0f, 0.53f, 0.16f, 1.0f);
    Core::WeakPointer<Core::Mesh> slab = GeometryUtils::buildBoxMesh(2.0, 2.0, 2.0, slabColor);
    slab->calculateNormals(75.0f);

    Core::WeakPointer<MeshContainer> bottomSlabObj(engine->createObject3D<MeshContainer>());
    bottomSlabObj->setName("Base platform");
    Core::WeakPointer<Core::MeshRenderer> bottomSlabRenderer(engine->createRenderer<Core::MeshRenderer>(cubeMaterial, bottomSlabObj));
    bottomSlabObj->addRenderable(slab);
    this->coreScene.addObjectToScene(bottomSlabObj);
    this->addObjectToSceneRaycaster(bottomSlabObj, slab);
    // this->meshToObjectMap[slab->getObjectID()] = Core::WeakPointer<MeshContainer>::dynamicPointerCast<Core::Object3D>( bottomSlabObj);
    bottomSlabObj->getTransform().getLocalMatrix().scale(15.0f, 1.0f, 15.0f);
    bottomSlabObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, -1.0f, 0.0f));
    bottomSlabObj->getTransform().getLocalMatrix().preRotate(0.0f, 1.0f, 0.0f,Core::Math::PI / 4.0f);

}

void ModelerApp::setupLights() {
    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();

    this->ambientLightObject = engine->createObject3D();
    this->ambientLightObject->setName("Ambient light");
    this->coreScene.addObjectToScene(ambientLightObject);
    Core::WeakPointer<Core::AmbientLight> ambientLight = engine->createLight<Core::AmbientLight>(ambientLightObject);
    ambientLight->setColor(0.25f, 0.25f, 0.25f, 1.0f);

    this->pointLightObject = engine->createObject3D<MeshContainer>();
    this->pointLightObject->setName("Point light");
    this->coreScene.addObjectToScene(pointLightObject);
    Core::WeakPointer<Core::PointLight> pointLight = engine->createPointLight<Core::PointLight>(pointLightObject, true, 2048, 0.0115, 0.35);
    pointLight->setColor(1.0f, 1.0f, 1.0f, 1.0f);
    pointLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    pointLight->setRadius(10.0f);
    this->pointLightObject->getTransform().translate(Core::Vector3r(5.0f, 10.0f, 5.0f));
    Core::Real pointLightSize = 0.35f;
    Core::WeakPointer<Core::Mesh> pointLightMesh = GeometryUtils::buildBoxMesh(pointLightSize, pointLightSize, pointLightSize, Core::Color(1.0f, 1.0f, 1.0f, 1.0f));
    Core::WeakPointer<Core::BasicMaterial> pointLightMaterial = engine->createMaterial<Core::BasicMaterial>();
    Core::WeakPointer<Core::MeshRenderer> pointLightRenderer(engine->createRenderer<Core::MeshRenderer>(pointLightMaterial, this->pointLightObject));
    pointLightRenderer->setCastShadows(false);
    this->pointLightObject->addRenderable(pointLightMesh);
    this->addObjectToSceneRaycaster(this->pointLightObject, pointLightMesh);

    this->directionalLightObject = engine->createObject3D();
    this->directionalLightObject->setName("Directonal light");
    this->coreScene.addObjectToScene(directionalLightObject);
    Core::WeakPointer<Core::DirectionalLight> directionalLight = engine->createDirectionalLight<Core::DirectionalLight>(directionalLightObject, 3, true, 4096, 0.0001, 0.0005);
    directionalLight->setColor(1.0, 1.0, 1.0, 1.0f);
    directionalLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    this->directionalLightObject->getTransform().lookAt(Core::Point3r(1.0f, -1.0f, 1.0f));
}

void ModelerApp::setupHighlightMaterials() {
    this->highlightColor.set(1.0, 0.65, 0.0, 0.35);
    this->outlineColor.set(1.0, 0.65, 0.0, 1.0);
    this->darkOutlineColor.set(this->outlineColor.r * .5, this->outlineColor.g * .5, this->outlineColor.b * .5, 1.0);

    this->highlightMaterial = engine->createMaterial<Core::BasicColoredMaterial>();
    this->highlightMaterial->setBlendingMode(Core::RenderState::BlendingMode::Custom);
    this->highlightMaterial->setSourceBlendingMethod(Core::RenderState::BlendingMethod::SrcAlpha);
    this->highlightMaterial->setDestBlendingMethod(Core::RenderState::BlendingMethod::OneMinusSrcAlpha);
    this->highlightMaterial->setLit(false);
    this->highlightMaterial->setZOffset(-.00005f);
    this->highlightMaterial->setColor(highlightColor);

    this->outlineMaterial = engine->createMaterial<Core::OutlineMaterial>();
    this->outlineMaterial->setLit(false);
    this->outlineMaterial->setColor(outlineColor);
}

void ModelerApp::preRenderCallback() {
    Core::WeakPointer<Core::Object3D> selectedObject = this->getCoreScene().getSelectedObject();
    if (selectedObject) {
        this->transformWidget.setTargetObject(selectedObject);
    }
}

void ModelerApp::postRenderCallback() {
    if (this->coreScene.getSelectedObject()) {
        Core::WeakPointer<Core::Object3D> selectedObject = this->coreScene.getSelectedObject();

        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Color, false);
        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Depth, false);
        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, false);

        this->highlightMaterial->setStencilTestEnabled(false);
        this->highlightMaterial->setStencilWriteMask(0x00);
        this->highlightMaterial->setFaceCullingEnabled(true);
        this->highlightMaterial->setColorWriteEnabled(true);
        this->highlightMaterial->setDepthTestEnabled(true);
        this->highlightMaterial->setDepthWriteEnabled(true);
        Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderObjectBasic(selectedObject, this->renderCamera, this->highlightMaterial);

        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, true);
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
        Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderObjectBasic(selectedObject, this->renderCamera, this->highlightMaterial);

        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, false);
        this->outlineMaterial->setStencilWriteMask(0x00);
        this->outlineMaterial->setStencilReadMask(0xFF);
        this->outlineMaterial->setStencilRef(1);
        this->outlineMaterial->setStencilTestEnabled(true);
        this->outlineMaterial->setStencilComparisonFunction(Core::RenderState::StencilFunction::NotEqual);
        this->outlineMaterial->setStencilFailActionStencil(Core::RenderState::StencilAction::Keep);
        this->outlineMaterial->setStencilFailActionDepth(Core::RenderState::StencilAction::Keep);
        this->outlineMaterial->setStencilAllPassAction(Core::RenderState::StencilAction::Replace);
        this->outlineMaterial->setFaceCullingEnabled(false);
        this->outlineMaterial->setColor(this->outlineColor);
        this->outlineMaterial->setColorWriteEnabled(true);
        this->outlineMaterial->setDepthWriteEnabled(false);
        this->outlineMaterial->setDepthTestEnabled(true);
        this->outlineMaterial->setDepthFunction(Core::RenderState::DepthFunction::LessThanOrEqual);
        Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderObjectBasic(selectedObject, this->renderCamera, this->outlineMaterial);

        this->outlineMaterial->setColor(this->darkOutlineColor);
        this->outlineMaterial->setDepthFunction(Core::RenderState::DepthFunction::GreaterThanOrEqual);
        Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderObjectBasic(selectedObject, this->renderCamera, this->outlineMaterial);

        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Color, true);
        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Depth, true);
        this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, true);

        this->transformWidget.updateCamera();
        this->transformWidget.render();
    }
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
            if (button == 1) {
                if (!this->transformWidget.startAction(x, y)) {
                    this->rayCastForObjectSelection(x, y, true);
                }
            }
        break;
        case MouseAdapter::MouseEventType::ButtonRelease:
            if (button == 1) this->transformWidget.endAction(x, y);
        break;
    }
}

void ModelerApp::rayCastForObjectSelection(Core::Int32 x, Core::Int32 y, bool setSelectedObject) {
    Core::WeakPointer<Core::Graphics> graphics = this->engine->getGraphicsSystem();
    Core::Vector4u viewport = graphics->getViewport();
    Core::Ray ray = this->renderCamera->getRay(viewport, x, y);
    std::vector<Core::Hit> hits;
    Core::Bool hitOccurred = this->sceneRaycaster.castRay(ray, hits);

    if (hitOccurred) {
        Core::Hit& hit = hits[0];
        Core::WeakPointer<Core::Mesh> hitObject = hit.Object;
        Core::WeakPointer<Core::Object3D> rootObject =this->meshToObjectMap[hitObject->getObjectID()];

        if (setSelectedObject) {
            this->getCoreScene().setSelectedObject(rootObject);
        }
    }
}

void ModelerApp::resolveOnUpdateCallbacks() {
    QMutexLocker ml(&this->onUpdateMutex);
    for (ModelerAppLifecycleEventCallback callback : this->onUpdates) {
        callback();
    }
}

void ModelerApp::addObjectToSceneRaycaster(Core::WeakPointer<Core::Object3D> object, Core::WeakPointer<Core::Mesh> mesh) {
    this->sceneRaycaster.addObject(object, mesh);
    this->meshToObjectMap[mesh->getObjectID()] = object;
}
