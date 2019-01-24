#include "ModelerApp.h"
#include "RenderWindow.h"

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
            this->coreSync = std::make_shared<CoreSync>(renderWindow);
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
                       this->rayCaster.addObject(obj, mesh);
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

    // ====== initial camera setup ====================
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


    // ====== model platform vertex attributes ====================
    Core::Real cubeVertexPositions[] = {
        // back
        -1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0,
        -1.0, -1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0,
        // left
        -1.0, -1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0,
        -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0,
        // right
        1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0,
        1.0, 1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0,
        // top
        -1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, -1.0, 1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
        // bottom
        -1.0, -1.0, -1.0, 1.0, -1.0, -1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 1.0,
        -1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0,
        // front
        1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0, -1.0, 1.0, 1.0, 1.0,
        1.0, 1.0, 1.0, 1.0, 1.0, -1.0, 1.0, 1.0, -1.0, -1.0, 1.0, 1.0
    };

    Core::Real cubeVertexNormals[] = {
        // back
        0.0, 0.0, -1.0, 0.0,  0.0, 0.0, -1.0, 0.0,  0.0, 0.0, -1.0, 0.0,
        0.0, 0.0, -1.0, 0.0,  0.0, 0.0, -1.0, 0.0,  0.0, 0.0, -1.0, 0.0,
        // left
        -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,
        -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0,
        // right
        1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0,
        // top
        0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0,
        // bottom
        0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0,
        0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0, 0.0, -1.0, 0.0, 0.0,
        // front
        0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 1.0, 0.0,
    };

    Core::Color slabColor(0.0f, 0.53f, 0.16f, 1.0f);
    Core::Real cubeVertexColors[] = {
        // back
        slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
        slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
        // left
        slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
        slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
        // right
        slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
        slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
        // top
        slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
        slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
        // bottom
        slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
        slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
        // front
        slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0,
        slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0, slabColor.r, slabColor.g, slabColor.b, 1.0
    };

    Core::WeakPointer<Core::BasicLitMaterial> cubeMaterial = engine->createMaterial<Core::BasicLitMaterial>();


    // ======= model platform objects ===============
    Core::WeakPointer<Core::Mesh> slab(engine->createMesh(36, false));
    slab->init();
    slab->enableAttribute(Core::StandardAttribute::Position);
    Core::Bool positionInited = slab->initVertexPositions();
    ASSERT(positionInited, "Unable to initialize slab mesh vertex positions.");
    slab->getVertexPositions()->store(cubeVertexPositions);

    slab->enableAttribute(Core::StandardAttribute::Color);
    Core::Bool colorInited = slab->initVertexColors();
    ASSERT(colorInited, "Unable to initialize slab mesh vertex colors.");
    slab->getVertexColors()->store(cubeVertexColors);

    slab->enableAttribute(Core::StandardAttribute::Normal);
    Core::Bool normalInited = slab->initVertexNormals();
    ASSERT(normalInited, "Unable to initialize slab mesh vertex normals.");
    slab->getVertexNormals()->store(cubeVertexNormals);

    slab->enableAttribute(Core::StandardAttribute::FaceNormal);
    Core::Bool faceNormalInited = slab->initVertexFaceNormals();

    slab->calculateBoundingBox();
    slab->calculateNormals(75.0f);

    Core::WeakPointer<MeshContainer> bottomSlabObj(engine->createObject3D<MeshContainer>());
    bottomSlabObj->setName("Base platform");
    Core::WeakPointer<Core::MeshRenderer> bottomSlabRenderer(engine->createRenderer<Core::MeshRenderer>(cubeMaterial, bottomSlabObj));
    bottomSlabObj->addRenderable(slab);
    this->coreScene.addObjectToScene(bottomSlabObj);
    this->rayCaster.addObject(bottomSlabObj, slab);
    this->meshToObjectMap[slab->getObjectID()] = bottomSlabObj;
    // this->meshToObjectMap[slab->getObjectID()] = Core::WeakPointer<MeshContainer>::dynamicPointerCast<Core::Object3D>( bottomSlabObj);
    bottomSlabObj->getTransform().getLocalMatrix().scale(15.0f, 1.0f, 15.0f);
    bottomSlabObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, -1.0f, 0.0f));
    bottomSlabObj->getTransform().getLocalMatrix().preRotate(0.0f, 1.0f, 0.0f,Core::Math::PI / 4.0f);


    // ========== lights ============================
    Core::WeakPointer<Core::Object3D> ambientLightObject = engine->createObject3D();
    ambientLightObject->setName("Ambient light");
    this->coreScene.addObjectToScene(ambientLightObject);
    Core::WeakPointer<Core::AmbientLight> ambientLight = engine->createLight<Core::AmbientLight>(ambientLightObject);
    ambientLight->setColor(0.25f, 0.25f, 0.25f, 1.0f);

    Core::WeakPointer<Core::Object3D> pointLightObject = engine->createObject3D();
    pointLightObject->setName("Point light");
    this->coreScene.addObjectToScene(pointLightObject);
    Core::WeakPointer<Core::PointLight> pointLight = engine->createPointLight<Core::PointLight>(pointLightObject, true, 2048, 0.0115, 0.35);
    pointLight->setColor(1.0f, 1.0f, 1.0f, 1.0f);
    pointLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    pointLight->setRadius(10.0f);

    Core::WeakPointer<Core::Object3D> directionalLightObject = engine->createObject3D();
    directionalLightObject->setName("Directonal light");
    this->coreScene.addObjectToScene(directionalLightObject);
    Core::WeakPointer<Core::DirectionalLight> directionalLight = engine->createDirectionalLight<Core::DirectionalLight>(directionalLightObject, 3, true, 4096, 0.0001, 0.0005);
    directionalLight->setColor(1.0, 1.0, 1.0, 1.0f);
    directionalLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    directionalLightObject->getTransform().lookAt(Core::Point3r(1.0f, -1.0f, 1.0f));

    engine->onUpdate([this, pointLightObject]() {

        static Core::Real rotationAngle = 0.0;
        static Core::Real counter = 0.0;
        counter += 0.01;
        if (counter> 1.0) counter = 0.0;

        if (Core::WeakPointer<Core::Object3D>::isValid(pointLightObject)) {

            rotationAngle += 0.6 * Core::Time::getDeltaTime();
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

            Core::WeakPointer<Core::Object3D> lightObjectPtr = pointLightObject;
            lightObjectPtr->getTransform().getLocalMatrix().copy(worldMatrix);

        }

        auto vp = Core::Engine::instance()->getGraphicsSystem()->getCurrentRenderTarget()->getViewport();
        this->renderCamera->setAspectRatioFromDimensions(vp.z, vp.w);
    }, true);

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

    engine->onRender([this]() {
        if (this->coreScene.getSelectedObject()) {
            Core::WeakPointer<Core::Object3D> selectedObject = this->coreScene.getSelectedObject();

            this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Color, false);
            this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Depth, false);
            this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, false);

            Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderObjectBasic(selectedObject, this->renderCamera, this->highlightMaterial);

            Core::Engine::instance()->getGraphicsSystem()->setStencilTestEnabled(true);
            Core::Engine::instance()->getGraphicsSystem()->setStencilOperation(Core::RenderState::StencilAction::Keep,
                                                                              Core::RenderState::StencilAction::Keep,
                                                                              Core::RenderState::StencilAction::Replace);

            this->renderCamera->setRenderBufferEnabled(Core::RenderBufferType::Depth, false);
            this->renderCamera->setRenderBufferEnabled(Core::RenderBufferType::Color, false);
            this->renderCamera->setRenderBufferEnabled(Core::RenderBufferType::Stencil, true);
            Core::Engine::instance()->getGraphicsSystem()->setFaceCulling(Core::RenderState::CullFace::None);
            Core::Engine::instance()->getGraphicsSystem()->setDepthTestEnabled(false);
            Core::Engine::instance()->getGraphicsSystem()->setStencilFunction(Core::RenderState::StencilFunction::Always, 1, 0xFF);
            this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, true);
            Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderObjectBasic(selectedObject, this->renderCamera, this->highlightMaterial);
            Core::Engine::instance()->getGraphicsSystem()->setFaceCulling(Core::RenderState::CullFace::Back);

            Core::Engine::instance()->getGraphicsSystem()->setFaceCulling(Core::RenderState::CullFace::None);
            this->outlineMaterial->setColor(this->outlineColor);
            Core::Engine::instance()->getGraphicsSystem()->setDepthTestEnabled(true);
            Core::Engine::instance()->getGraphicsSystem()->setStencilFunction(Core::RenderState::StencilFunction::NotEqual, 1, 0xFF);
            this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, false);
            this->renderCamera->setRenderBufferEnabled(Core::RenderBufferType::Stencil, false);
            this->renderCamera->setRenderBufferEnabled(Core::RenderBufferType::Color, true);
            this->renderCamera->setRenderBufferEnabled(Core::RenderBufferType::Depth, false);
            Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderObjectBasic(selectedObject, this->renderCamera, this->outlineMaterial);

            Core::Engine::instance()->getGraphicsSystem()->setDepthFunction(Core::RenderState::DepthFunction::GreaterThanOrEqual);
            this->outlineMaterial->setColor(this->darkOutlineColor);
            Core::Engine::instance()->getGraphicsSystem()->setStencilFunction(Core::RenderState::StencilFunction::NotEqual, 1, 0xFF);
            Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderObjectBasic(selectedObject, this->renderCamera, this->outlineMaterial);
            Core::Engine::instance()->getGraphicsSystem()->setDepthFunction(Core::RenderState::DepthFunction::LessThanOrEqual);

            Core::Engine::instance()->getGraphicsSystem()->setStencilTestEnabled(false);
            Core::Engine::instance()->getGraphicsSystem()->setFaceCulling(Core::RenderState::CullFace::Back);
            this->renderCamera->setRenderBufferEnabled(Core::RenderBufferType::Depth, true);
            this->renderCamera->setRenderBufferEnabled(Core::RenderBufferType::Color, true);
            this->renderCamera->setRenderBufferEnabled(Core::RenderBufferType::Stencil, true);

            this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Color, true);
            this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Depth, true);
            this->renderCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, true);
        }
    }, true);

    this->renderWindow->onUpdate([this](Core::WeakPointer<Core::Engine> engine){
        this->resolveOnUpdateCallbacks();
    });
}

void ModelerApp::gesture(GestureAdapter::GestureEvent event) {
    if (this->engineIsReady) {
        GestureAdapter::GestureEventType eventType = event.getType();
        switch(eventType) {
            case GestureAdapter::GestureEventType::Move:
                this->rayCastForObjectSelection(event.end.x, event.end.y, false);
            break;
            case GestureAdapter::GestureEventType::Drag:
            case GestureAdapter::GestureEventType::Scroll:
                this->orbitControls->handleGesture((event));
            break;
        }
    }
}

void ModelerApp::mouseButton(MouseAdapter::MouseEventType type, Core::UInt32 button, Core::UInt32 x, Core::UInt32 y) {
    switch(type) {
        case MouseAdapter::MouseEventType::ButtonPress:
        {
            if (button == 1) this->rayCastForObjectSelection(x, y, true);
        }
        break;
    }
}

void ModelerApp::rayCastForObjectSelection(Core::UInt32 x, Core::UInt32 y, bool setSelectedObject) {
    Core::Point3r pos((Core::Real)x, (Core::Real)y, (Core::Real)-1.0f);
    CoreSync::Runnable runnable = [this, pos, setSelectedObject](Core::WeakPointer<Core::Engine> engine) {

        Core::WeakPointer<Core::Graphics> graphics = this->engine->getGraphicsSystem();
        Core::WeakPointer<Core::Renderer> rendererPtr = graphics->getRenderer();
        Core::Vector4u viewport = graphics->getViewport();

        Core::Real ndcX = (Core::Real)pos.x / (Core::Real)viewport.z * 2.0f - 1.0f;
        Core::Real ndcY = -((Core::Real)pos.y / (Core::Real)viewport.w * 2.0f - 1.0f);
        Core::Point3r ndcPos(ndcX, ndcY, -1.0);
        this->renderCamera->unProject(ndcPos);
        Core::Transform& camTransform = this->renderCamera->getOwner()->getTransform();
        camTransform.updateWorldMatrix();
        Core::Matrix4x4 camMat = camTransform.getWorldMatrix();
        Core::Matrix4x4 camMatInverse = camMat;
        camMatInverse.invert();

        Core::Point3r worldPos = ndcPos;
        camMat.transform(worldPos);
        Core::Point3r origin;
        camMat.transform(origin);
        Core::Vector3r rayDir = worldPos - origin;
        rayDir.normalize();
        Core::Ray ray(origin, rayDir);

        std::vector<Core::Hit> hits;
        Core::Bool hit = this->rayCaster.castRay(ray, hits);

        if (hits.size() > 0) {
            Core::Hit& hit = hits[0];
            Core::WeakPointer<Core::Mesh> hitObject = hit.Object;
            Core::WeakPointer<Core::Object3D> rootObject =this->meshToObjectMap[hitObject->getObjectID()];

            if (setSelectedObject) this->getCoreScene().setSelectedObject(rootObject);
        }

    };
    if (this->coreSync) {
        this->coreSync->run(runnable);
    }
}


void ModelerApp::resolveOnUpdateCallbacks() {
    QMutexLocker ml(&this->onUpdateMutex);
    for (ModelerAppLifecycleEventCallback callback : this->onUpdates) {
        callback();
    }
}
