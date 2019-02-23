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
    this->transformWidgetActiveComponentID = -1;
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
                       this->sceneRaycaster.addObject(obj, mesh);
                       this->meshToObjectMap[mesh->getObjectID()] = obj;
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
    this->setupTransformWidget();
    this->setupLights();
    this->setupHighlightMaterials();

    engine->onPreRender([this]() {
        this->preRenderCallback();
    }, true);

    engine->onPostRender([this]() {
        this->postRenderCallback();
    }, true);

    engine->onUpdate([this]() {
        this->resolveOnUpdateCallbacks();
    });
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
    this->sceneRaycaster.addObject(bottomSlabObj, slab);
    this->meshToObjectMap[slab->getObjectID()] = bottomSlabObj;
    // this->meshToObjectMap[slab->getObjectID()] = Core::WeakPointer<MeshContainer>::dynamicPointerCast<Core::Object3D>( bottomSlabObj);
    bottomSlabObj->getTransform().getLocalMatrix().scale(15.0f, 1.0f, 15.0f);
    bottomSlabObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, -1.0f, 0.0f));
    bottomSlabObj->getTransform().getLocalMatrix().preRotate(0.0f, 1.0f, 0.0f,Core::Math::PI / 4.0f);

}

void ModelerApp::setupTransformWidget() {
    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();

    transformWidgetXMaterial = engine->createMaterial<BasicRimShadowMaterial>();
    transformWidgetXMaterial->setHighlightLowerBound(0.6f);
    transformWidgetXMaterial->setHighlightScale(1.25f);
    transformWidgetXMaterial->setDepthTestEnabled(true);
    transformWidgetYMaterial = Core::WeakPointer<Core::Material>::dynamicPointerCast<BasicRimShadowMaterial>(transformWidgetXMaterial->clone());
    transformWidgetZMaterial = Core::WeakPointer<Core::Material>::dynamicPointerCast<BasicRimShadowMaterial>(transformWidgetXMaterial->clone());

    this->transformWidgetRoot = engine->createObject3D();
    this->transformWidgetRoot->setName("TransformWidget");

    transformWidgetHighlightColor.set(1.0f, 1.0f, 1.0f, 1.0f);
    Core::Real baseLength = 2.0f;
    Core::Real coneLength = 0.4f;
    Core::Real halfLength = (baseLength + coneLength) / 2.0f;
    Core::WeakPointer<Core::Mesh> arrowMesh = GeometryUtils::buildArrowMesh(baseLength, 0.035f, coneLength, 0.15f, 16, transformWidgetHighlightColor);
    Core::WeakPointer<Core::Mesh> arrowColliderMesh = GeometryUtils::buildBoxMesh(.15f, baseLength + coneLength, .15f, transformWidgetHighlightColor);

    transformWidgetXColor.set(1.0f, 0.0f, 0.0f, 1.0f);
    transformWidgetXMaterial->setHighlightColor(transformWidgetXColor);
    Core::WeakPointer<MeshContainer> xArrow = GeometryUtils::buildMeshContainer(arrowMesh, transformWidgetXMaterial, "XArrow");
    xArrow->getTransform().getLocalMatrix().preRotate(0.0f, 0.0f, 1.0f, -Core::Math::PI / 2.0f);
    xArrow->getTransform().getLocalMatrix().preTranslate(halfLength, 0.0f, 0.0f);
    this->transformWidgetXTranslateID = this->transformWidgetRaycaster.addObject(xArrow, arrowColliderMesh);

    transformWidgetYColor.set(0.0f, 1.0f, 0.0f, 1.0f);
    transformWidgetYMaterial->setHighlightColor(transformWidgetYColor);
    Core::WeakPointer<MeshContainer> yArrow = GeometryUtils::buildMeshContainer(arrowMesh, transformWidgetYMaterial, "YArrow");
    yArrow->getTransform().getLocalMatrix().preTranslate(0.0f, halfLength, 0.0f);
    this->transformWidgetYTranslateID = this->transformWidgetRaycaster.addObject(yArrow, arrowColliderMesh);

    transformWidgetZColor.set(0.0f, 0.0f, 1.0f, 1.0f);
    transformWidgetZMaterial->setHighlightColor(transformWidgetZColor);
    Core::WeakPointer<MeshContainer> zArrow = GeometryUtils::buildMeshContainer(arrowMesh, transformWidgetZMaterial, "ZArrow");
    zArrow->getTransform().getLocalMatrix().preRotate(1.0f, 0.0f, 0.0f, -Core::Math::PI / 2.0f);
    zArrow->getTransform().getLocalMatrix().preTranslate(0.0f, 0.0f, -halfLength);
    this->transformWidgetZTranslateID = this->transformWidgetRaycaster.addObject(zArrow, arrowColliderMesh);

    this->transformWidgetRoot->addChild(xArrow);
    this->transformWidgetRoot->addChild(yArrow);
    this->transformWidgetRoot->addChild(zArrow);

    this->transformWidgetCameraObj = engine->createObject3D();
    this->transformWidgetCamera = engine->createPerspectiveCamera(this->transformWidgetCameraObj, Core::Camera::DEFAULT_FOV, Core::Camera::DEFAULT_ASPECT_RATIO, 0.1f, 100);

}

void ModelerApp::setupLights() {
    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();

    this->ambientLightObject = engine->createObject3D();
    this->ambientLightObject->setName("Ambient light");
    this->coreScene.addObjectToScene(ambientLightObject);
    Core::WeakPointer<Core::AmbientLight> ambientLight = engine->createLight<Core::AmbientLight>(ambientLightObject);
    ambientLight->setColor(0.25f, 0.25f, 0.25f, 1.0f);

    this->pointLightObject = engine->createObject3D();
    this->pointLightObject->setName("Point light");
    this->coreScene.addObjectToScene(pointLightObject);
    Core::WeakPointer<Core::PointLight> pointLight = engine->createPointLight<Core::PointLight>(pointLightObject, true, 2048, 0.0115, 0.35);
    pointLight->setColor(1.0f, 1.0f, 1.0f, 1.0f);
    pointLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    pointLight->setRadius(10.0f);

    this->directionalLightObject = engine->createObject3D();
    this->directionalLightObject->setName("Directonal light");
    this->coreScene.addObjectToScene(directionalLightObject);
    Core::WeakPointer<Core::DirectionalLight> directionalLight = engine->createDirectionalLight<Core::DirectionalLight>(directionalLightObject, 3, true, 4096, 0.0001, 0.0005);
    directionalLight->setColor(1.0, 1.0, 1.0, 1.0f);
    directionalLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    this->directionalLightObject->getTransform().lookAt(Core::Point3r(1.0f, -1.0f, 1.0f));

    engine->onUpdate([this]() {
        this->updateLights();
    }, true);
}

void ModelerApp::updateLights() {
    static Core::Real rotationAngle = 0.0;
    static Core::Real counter = 0.0;
    counter += 0.01;
    if (counter> 1.0) counter = 0.0;

    if (Core::WeakPointer<Core::Object3D>::isValid(this->pointLightObject)) {

        rotationAngle += 0.6 * Core::Time::getDeltaTime();
        rotationAngle = 0.0f;
        if (rotationAngle >= Core::Math::TwoPI) rotationAngle -= Core::Math::TwoPI;

        Core::Quaternion qA;
        qA.fromAngleAxis(rotationAngle, 0, 1, 0);
        Core::Matrix4x4 rotationMatrixA;
        qA.rotationMatrix(rotationMatrixA);

        Core::Quaternion qB;
        qB.fromAngleAxis(-0.8, 1, 0, 0);
        Core::Matrix4x4 rotationMatrixB;
        qB.rotationMatrix(rotationMatrixB);

        Core::Matrix4x4 worldMatrix;

        worldMatrix.preTranslate(10.0f, 10.0f, 0.0f);
        worldMatrix.preMultiply(rotationMatrixA);
        //worldMatrix.multiply(rotationMatrixB);

        Core::WeakPointer<Core::Object3D> lightObjectPtr = this->pointLightObject;
        lightObjectPtr->getTransform().getLocalMatrix().copy(worldMatrix);

    }

    auto vp = Core::Engine::instance()->getGraphicsSystem()->getCurrentRenderTarget()->getViewport();
    this->renderCamera->setAspectRatioFromDimensions(vp.z, vp.w);
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
        this->updateTransformWidgetForObject(selectedObject);
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

\
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

        this->updateTransformWidgetCamera();
        this->transformWidgetCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Color, false);
        this->transformWidgetCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Depth, true);
        this->transformWidgetCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, true);
        Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderObjectBasic(this->transformWidgetRoot, this->transformWidgetCamera);
    }
}

void ModelerApp::updateTransformWidgetForObject(Core::WeakPointer<Core::Object3D> object) {
     Core::Transform& objectTransform = object->getTransform();
     objectTransform.updateWorldMatrix();

     Core::Point3r origin;
     Core::Vector3r forward = Core::Vector3r::Forward;
     Core::Vector3r up = Core::Vector3r::Up;

     objectTransform.transform(origin);
     objectTransform.transform(forward);
     objectTransform.transform(up);

     Core::Transform& transformWidgetTransform = this->transformWidgetRoot->getTransform();
     transformWidgetTransform.getLocalMatrix().lookAt(origin, origin + forward, up);
}

void ModelerApp::updateTransformWidgetCamera() {

    this->transformWidgetCamera->copyFrom(this->renderCamera);

    Core::Point3r transformWidgetPosition;
    Core::Transform& transformWidgetTransform = this->transformWidgetRoot->getTransform();
    transformWidgetTransform.updateWorldMatrix();
    transformWidgetTransform.transform(transformWidgetPosition);

    Core::Point3r renderCameraPosition;
    Core::Transform& renderCameraTransform = this->renderCamera->getOwner()->getTransform();
    renderCameraTransform.updateWorldMatrix();
    renderCameraTransform.transform(renderCameraPosition);

    Core::Vector3r transformWidgetToRenderCamera = renderCameraPosition - transformWidgetPosition;
    transformWidgetToRenderCamera.normalize();
    transformWidgetToRenderCamera.scale(18.0f);

    Core::Point3r newTransformWidgetCameraPosition = transformWidgetPosition + transformWidgetToRenderCamera;

    Core::WeakPointer<Core::Object3D> transformWidgetCameraObj = this->transformWidgetCamera->getOwner();
    transformWidgetCameraObj->getTransform().updateWorldMatrix();
    Core::Point3r transformWidgetCameraPosition;
    renderCameraTransform.transform(transformWidgetCameraPosition);
    Core::Vector3r translation = newTransformWidgetCameraPosition - transformWidgetCameraPosition;
    transformWidgetCameraObj->getTransform().getLocalMatrix().copy(renderCameraTransform.getLocalMatrix());
    transformWidgetCameraObj->getTransform().translate(translation, Core::TransformationSpace::World);
    transformWidgetCameraObj->getTransform().updateWorldMatrix();
}

void ModelerApp::gesture(GestureAdapter::GestureEvent event) {
    if (this->engineIsReady) {
        GestureAdapter::GestureEventType eventType = event.getType();
        switch(eventType) {
            case GestureAdapter::GestureEventType::Move:
                this->rayCastForTransformWidgetSelection(event.end.x, event.end.y);
            break;
            case GestureAdapter::GestureEventType::Drag:
            case GestureAdapter::GestureEventType::Scroll:
                if (this->transformWidgetActiveComponentID == -1) {
                    this->orbitControls->handleGesture((event));
                }
                else {
                    this->updateTransformWidgetAction(event.end.x, event.end.y);
                }
            break;
        }
    }
}

void ModelerApp::mouseButton(MouseAdapter::MouseEventType type, Core::UInt32 button, Core::UInt32 x, Core::UInt32 y) {
    switch(type) {
        case MouseAdapter::MouseEventType::ButtonPress: {
            if (this->transformWidgetActiveComponentID == -1) {
                if (button == 1) this->rayCastForObjectSelection(x, y, true);
            }
            else {
                this->startTransformWidgetAction(x, y);
            }
        }
        break;
    }
}

void ModelerApp::startTransformWidgetAction(Core::UInt32 x, Core::UInt32 y) {
    this->transformWidgetActionStartPosition.set(0.0f, 0.0f, 0.0f);
    Core::Transform& transformWidgetTransform = this->transformWidgetRoot->getTransform();
    transformWidgetTransform.updateWorldMatrix();
    transformWidgetTransform.transform(this->transformWidgetActionStartPosition);


     Core::Vector3r planeNormal;
    if (this->transformWidgetActiveComponentID == this->transformWidgetXTranslateID) {
        this->transformWidgetActionNormal.set(1.0f, 0.0f, 0.0f);
        planeNormal.set(0.0f, 0.0f, -1.0f);
    }
    else if (this->transformWidgetActiveComponentID == this->transformWidgetYTranslateID) {
        this->transformWidgetActionNormal.set(0.0f, 1.0f, 0.0f);
        planeNormal.set(0.0f, 0.0f, -1.0f);
    }
    else if (this->transformWidgetActiveComponentID == this->transformWidgetZTranslateID) {
        this->transformWidgetActionNormal.set(0.0f, 0.0f, -1.0f);
        planeNormal.set(0.0f, 1.0f, 0.0f);
    }

    transformWidgetTransform.transform(this->transformWidgetActionNormal);
    transformWidgetTransform.transform(planeNormal);
    Core::Real d = planeNormal.dot(this->transformWidgetActionStartPosition);
    this->transformWidgetPlane.set(planeNormal.x, planeNormal.y, planeNormal.z, d);
}

void ModelerApp::updateTransformWidgetAction(Core::UInt32 x, Core::UInt32 y) {
    Core::WeakPointer<Core::Graphics> graphics = this->engine->getGraphicsSystem();
    Core::Vector4u viewport = graphics->getViewport();
    Core::Ray ray = this->transformWidgetCamera->getRay(viewport, x, y);

    Core::Vector4r rayOrigin(ray.Origin.x, ray.Origin.y, ray.Origin.z, 1.0f);
    Core::Vector4r rayDir(ray.Direction.x, ray.Direction.y, ray.Direction.z, 0.0f);
    Core::Real t = -(Core::Vector4r::dot(this->transformWidgetPlane, rayOrigin) / Core::Vector4r::dot(this->transformWidgetPlane, rayDir));
    Core::Point3r intersection = ray.Origin + ray.Direction * t;

    Core::Vector3r toIntersection = intersection - this->transformWidgetActionStartPosition;
    Core::Real p = toIntersection.dot(this->transformWidgetActionNormal);
    Core::Vector3r offset = this->transformWidgetActionNormal * p;
    Core::Point3r targetPosition = this->transformWidgetActionStartPosition + offset;


    Core::Point3r currentWidgetPosition(0.0f, 0.0f, 0.0f);
    Core::Transform& transformWidgetTransform = this->transformWidgetRoot->getTransform();
    transformWidgetTransform.updateWorldMatrix();
    transformWidgetTransform.transform(currentWidgetPosition);

    Core::Vector3r translation = targetPosition - currentWidgetPosition;
    transformWidgetTransform.translate(translation, Core::TransformationSpace::World);

    this->coreScene.getSelectedObject()->getTransform().translate(translation, Core::TransformationSpace::World);
}

void ModelerApp::rayCastForTransformWidgetSelection(Core::UInt32 x, Core::UInt32 y) {
    this->updateTransformWidgetCamera();
    Core::WeakPointer<Core::Graphics> graphics = this->engine->getGraphicsSystem();
    Core::Vector4u viewport = graphics->getViewport();
    Core::Ray ray = this->transformWidgetCamera->getRay(viewport, x, y);
    std::vector<Core::Hit> hits;
    Core::Bool hitOccurred = this->transformWidgetRaycaster.castRay(ray, hits);

    if (hitOccurred) {
        Core::Hit& hit = hits[0];
        Core::WeakPointer<Core::Mesh> hitObject = hit.Object;
        if (this->transformWidgetActiveComponentID != hit.ID) {
            this->transformWidgetXMaterial->setHighlightColor(transformWidgetXColor);
            this->transformWidgetYMaterial->setHighlightColor(transformWidgetYColor);
            this->transformWidgetZMaterial->setHighlightColor(transformWidgetZColor);
            this->transformWidgetActiveComponentID = hit.ID;
            if (hit.ID == this->transformWidgetXTranslateID) {
                this->transformWidgetXMaterial->setHighlightColor(this->transformWidgetHighlightColor);
            }
            else if (hit.ID == this->transformWidgetYTranslateID) {
                this->transformWidgetYMaterial->setHighlightColor(this->transformWidgetHighlightColor);
            }
            else if (hit.ID == this->transformWidgetZTranslateID) {
                this->transformWidgetZMaterial->setHighlightColor(this->transformWidgetHighlightColor);
            }
        }
    }
    else {
        this->transformWidgetActiveComponentID = -1;
        this->transformWidgetXMaterial->setHighlightColor(transformWidgetXColor);
        this->transformWidgetYMaterial->setHighlightColor(transformWidgetYColor);
        this->transformWidgetZMaterial->setHighlightColor(transformWidgetZColor);
    }
}

void ModelerApp::rayCastForObjectSelection(Core::UInt32 x, Core::UInt32 y, bool setSelectedObject) {
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
