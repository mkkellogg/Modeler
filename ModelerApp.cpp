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

ModelerApp::ModelerApp(): mainContainer(nullptr), renderWindow(nullptr) {

}

void ModelerApp::init() {
    this->pipedGestureAdapter =
            std::make_shared<PipedEventAdapter<GestureAdapter::GestureEvent>>(std::bind(&ModelerApp::onGesture, this, std::placeholders::_1));
    this->gestureAdapter = std::make_shared<GestureAdapter>();
    this->gestureAdapter->setPipedEventAdapter(this->pipedGestureAdapter);

}

void ModelerApp::setMainContainer(MainContainer* mainContainer) {
    if(this->mainContainer != mainContainer) {
        this->mainContainer = mainContainer;
    }
}

void ModelerApp::setRenderWindow(RenderWindow* renderWindow) {
    if (this->renderWindow != renderWindow) {
        this->renderWindow = renderWindow;

        RenderWindow::LifeCycleEventCallback initer = [this](RenderWindow* renderWindow) {
            this->engine = renderWindow->getEngine();
            this->coreSync = std::make_shared<CoreSync>(renderWindow);
            this->onEngineReady(engine);
            this->orbitControls = std::make_shared<OrbitControls>(this->engine, this->renderCamera, this->coreSync);

            std::shared_ptr<MouseAdapter> mouseAdapter = std::make_shared<MouseAdapter>();
            this->gestureAdapter->setMouseAdapter(*(mouseAdapter.get()));
            this->renderWindow->setMouseAdapter(mouseAdapter);
            mouseAdapter->onMouseButtonPressed(std::bind(&ModelerApp::onMouseButtonAction, this, std::placeholders::_1,  std::placeholders::_2,  std::placeholders::_3, std::placeholders::_4));
        };
        renderWindow->onInit(initer);
    }
}

void ModelerApp::onEngineReady(Core::WeakPointer<Core::Engine> engine) {

    this->engineReady = true;

    Core::WeakPointer<Core::Scene> scene(engine->createScene());
    engine->setActiveScene(scene);
    this->sceneRoot = scene->getRoot();

    engine->getGraphicsSystem()->setClearColor(Core::Color(0,0,0,1));

    // ====== initial camera setup ====================
    Core::WeakPointer<Core::Object3D> cameraObj = engine->createObject3D<Core::Object3D>();
    this->renderCamera = engine->createPerspectiveCamera(cameraObj, Core::Camera::DEFAULT_FOV, Core::Camera::DEFAULT_ASPECT_RATIO, 0.1f, 100);
    this->sceneRoot->addChild(cameraObj);

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

    Core::WeakPointer<Core::BasicMaterial> cubeMaterial(engine->createMaterial<Core::BasicMaterial>());
    cubeMaterial->build();


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
    Core::WeakPointer<Core::MeshRenderer> bottomSlabRenderer(engine->createRenderer<Core::MeshRenderer>(cubeMaterial, bottomSlabObj));
    bottomSlabObj->addRenderable(slab);
    sceneRoot->addChild(bottomSlabObj);
    this->rayCaster.addObject(bottomSlabObj, slab);
    this->meshToObjectMap[slab->getObjectID()] = bottomSlabObj;
    // this->meshToObjectMap[slab->getObjectID()] = Core::WeakPointer<MeshContainer>::dynamicPointerCast<Core::Object3D>( bottomSlabObj);
    bottomSlabObj->getTransform().getLocalMatrix().scale(15.0f, 1.0f, 15.0f);
    bottomSlabObj->getTransform().getLocalMatrix().preTranslate(Core::Vector3r(0.0f, -2.0f, 0.0f));
    bottomSlabObj->getTransform().getLocalMatrix().preRotate(0.0f, 1.0f, 0.0f,Core::Math::PI / 4.0f);


    // ========== lights ============================
    /*Core::WeakPointer<Core::Object3D> ambientLightObject = engine->createObject3D();
    this->sceneRoot->addChild(ambientLightObject);
    Core::WeakPointer<Core::AmbientLight> ambientLight = engine->createLight<Core::AmbientLight>(ambientLightObject);
    ambientLight->setColor(0.25f, 0.25f, 0.25f, 1.0f);

    Core::WeakPointer<Core::Object3D> pointLightObject = engine->createObject3D();
    this->sceneRoot->addChild(pointLightObject);
    Core::WeakPointer<Core::PointLight> pointLight = engine->createPointLight<Core::PointLight>(pointLightObject, true, 2048, 0.0115, 0.35);
    pointLight->setColor(1.0f, 1.0f, 1.0f, 1.0f);
    pointLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    pointLight->setRadius(10.0f);

    Core::WeakPointer<Core::Object3D> directionalLightObject = engine->createObject3D();
    this->sceneRoot->addChild(directionalLightObject);
    Core::WeakPointer<Core::DirectionalLight> directionalLight = engine->createDirectionalLight<Core::DirectionalLight>(directionalLightObject, 3, true, 4096, 0.0001, 0.0005);
    directionalLight->setColor(1.0, 1.0, 1.0, 1.0f);
    directionalLight->setShadowSoftness(Core::ShadowLight::Softness::VerySoft);
    directionalLightObject->getTransform().lookAt(Core::Point3r(1.0f, -1.0f, 1.0f));*/

   engine->onUpdate([this]() {

        static Core::Real rotationAngle = 0.0;
        static Core::Real counter = 0.0;
        counter += 0.01;
        if (counter> 1.0) counter = 0.0;

        /*if (Core::WeakPointer<Core::Object3D>::isValid(pointLightObject)) {

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

        }*/

        auto vp = Core::Engine::instance()->getGraphicsSystem()->getCurrentRenderTarget()->getViewport();
        this->renderCamera->setAspectRatioFromDimensions(vp.z, vp.w);
    }, true);

}

void ModelerApp::onGesture(GestureAdapter::GestureEvent event) {
    if (this->engineReady) {
        GestureAdapter::GestureEventType eventType = event.getType();
        switch(eventType) {
            case GestureAdapter::GestureEventType::Drag:
            case GestureAdapter::GestureEventType::Scroll:
                this->orbitControls->handleGesture((event));
            break;
        }
    }
}

void ModelerApp::onMouseButtonAction(MouseAdapter::MouseEventType type, Core::UInt32 button, Core::UInt32 x, Core::UInt32 y) {
    switch(type) {
        case MouseAdapter::MouseEventType::ButtonPress:
        {
            Core::Point3r pos((Core::Real)x, (Core::Real)y, (Core::Real)-1.0f);
            if (button == 1) {
                CoreSync::Runnable runnable = [this, pos](Core::WeakPointer<Core::Engine> engine) {

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

                   // std::cerr << "Hit count: " << hits.size() << std::endl;
                    if (hits.size() > 0) {
                        Core::Hit& hit = hits[0];
                        Core::WeakPointer<Core::Mesh> hitObject = hit.Object;
                        Core::WeakPointer<Core::Object3D> rootObject =this->meshToObjectMap[hitObject->getObjectID()];
                        this->selectedObject = rootObject;
                        if (this->selectedObject) {
                            // std::cerr << "Selected: " << this->selectedObject->getObjectID() << std::endl;
                        }
                    }

                };
                if (this->coreSync) {
                    this->coreSync->run(runnable);
                }
            }

            break;
        }
    }
}
