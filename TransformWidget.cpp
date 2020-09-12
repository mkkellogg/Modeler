#include "Core/render/Camera.h"

#include "TransformWidget.h"
#include "SceneUtils.h"

using MeshContainer = Core::MeshContainer;

TransformWidget::TransformWidget(): coreScene(nullptr) {
    this->activeComponentID = -1;
    this->actionInProgress = false;
}

TransformWidget::~TransformWidget() {
    if (this->rootObject.isValid()) Core::Engine::safeReleaseObject(this->rootObject);
    if (this->cameraObj.isValid()) Core::Engine::safeReleaseObject(this->cameraObj);
}

void TransformWidget::init(Core::WeakPointer<Core::Camera> targetCamera) {
    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();
    this->targetCamera = targetCamera;

    xMaterial = engine->createMaterial<BasicRimShadowMaterial>();
    xMaterial->setHighlightLowerBound(0.6f);
    xMaterial->setHighlightScale(1.25f);
    xMaterial->setDepthTestEnabled(true);
    yMaterial = Core::WeakPointer<Core::Material>::dynamicPointerCast<BasicRimShadowMaterial>(xMaterial->clone());
    zMaterial = Core::WeakPointer<Core::Material>::dynamicPointerCast<BasicRimShadowMaterial>(xMaterial->clone());

    xColor.set(1.0f, 0.0f, 0.0f, 1.0f);
    xMaterial->setHighlightColor(xColor);
    yColor.set(0.0f, 1.0f, 0.0f, 1.0f);
    yMaterial->setHighlightColor(yColor);
    zColor.set(0.0f, 0.0f, 1.0f, 1.0f);
    zMaterial->setHighlightColor(zColor);

    this->rootObject = engine->createObject3D();
    this->rootObject->setName("TransformWidget");

    this->buildTranslationObject();
    this->buildRotationObject();

    this->cameraObj = engine->createObject3D();
    this->camera = engine->createPerspectiveCamera(this->cameraObj, Core::Camera::DEFAULT_FOV, Core::Camera::DEFAULT_ASPECT_RATIO, 0.1f, 100);

    this->activateTranslationMode();
}

void TransformWidget::updateTransformationForTargetObjects() {
    if (this->targetObjects.size() > 0) {
        Core::Point3r center;
        Core::Point3r origin;
        Core::Vector3r forward = Core::Vector3r::Forward;
        Core::Vector3r up = Core::Vector3r::Up;
        for (unsigned int i = 0; i < this->targetObjects.size(); i++) {
            Core::WeakPointer<Core::Object3D> targetObject = this->targetObjects[i];
            Core::Transform& objectTransform = targetObject->getTransform();
            objectTransform.updateWorldMatrix();
            Core::Point3r objectCenter;
            objectTransform.getWorldMatrix().transform(objectCenter);
            center = center + Core::Vector3r(objectCenter.x, objectCenter.y, objectCenter.z);
            if (i == 0) {
                objectTransform.getWorldMatrix().transform(origin);
                objectTransform.getWorldMatrix().transform(forward);
                objectTransform.getWorldMatrix().transform(up);
            }
        }
        center = center * (1.0f / (float)this->targetObjects.size());
        Core::Transform& widgetTransform = this->rootObject->getTransform();
        widgetTransform.getLocalMatrix().lookAt(center, center + forward, up);
    }
}

void TransformWidget::updateCamera() {
    this->camera->copyFrom(this->targetCamera);
    this->camera->setSkyboxEnabled(false);
    this->camera->setHDREnabled(false);

    Core::Point3r widgetPosition = this->rootObject->getTransform().getWorldPosition();
    Core::Transform& targetCameraTransform = this->targetCamera->getOwner()->getTransform();
    Core::Point3r targetCameraPosition = targetCameraTransform.getWorldPosition();

    Core::Vector3r widgetToTargetCamera = targetCameraPosition - widgetPosition;
    widgetToTargetCamera.normalize();
    widgetToTargetCamera.scale(18.0f);

    Core::Point3r newCameraPosition = widgetPosition + widgetToTargetCamera;
    Core::WeakPointer<Core::Object3D> cameraObj = this->camera->getOwner();
    cameraObj->getTransform().setLocalMatrix(targetCameraTransform.getLocalMatrix());
    cameraObj->getTransform().setWorldPosition(newCameraPosition);
}

void TransformWidget::render() {
    this->camera->setAutoClearRenderBuffer(Core::RenderBufferType::Color, false);
    this->camera->setAutoClearRenderBuffer(Core::RenderBufferType::Depth, true);
    this->camera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, true);
    Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderSceneBasic(this->rootObject, this->camera, true);
}

bool TransformWidget::startAction(Core::Int32 x, Core::Int32 y) {
    if (this->actionInProgress) return true;
    if (this->activeComponentID == -1) return false;

    Core::WeakPointer<Core::Graphics> graphics = Core::Engine::instance()->getGraphicsSystem();
    Core::Vector3r camDir = Core::Vector3r::Forward;
    Core::WeakPointer<Core::Object3D> cameraObj = this->camera->getOwner();
    Core::Transform& cameraTransform = cameraObj->getTransform();
    cameraTransform.updateWorldMatrix();
    cameraTransform.getWorldMatrix().transform(camDir);

    Core::Transform& widgetTransform = this->rootObject->getTransform();
    Core::Point3r widgetPosition;
    widgetTransform.updateWorldMatrix();
    widgetTransform.getWorldMatrix().transform(widgetPosition);

    if (this->activeComponentID == this->xTranslateID || this->activeComponentID == this->xRotateID) {
        this->actionNormal = Core::Vector3r::Right;
    }
    else if (this->activeComponentID == this->yTranslateID || this->activeComponentID == this->yRotateID) {
        this->actionNormal = Core::Vector3r::Up;
    }
    else if (this->activeComponentID == this->zTranslateID || this->activeComponentID == this->zRotateID) {
        this->actionNormal = Core::Vector3r::Forward;
    }
    widgetTransform.getWorldMatrix().transform(this->actionNormal);

    if (currentMode == TransformationMode::Translation) {
        Core::Vector3r tempNormal;
        Core::Vector3r planeNormal;
        Core::Vector3r::cross(camDir, this->actionNormal, tempNormal);
        Core::Vector3r::cross(tempNormal, this->actionNormal, planeNormal);

        Core::Real d = planeNormal.dot(widgetPosition);
        this->actionPlane.set(planeNormal.x, planeNormal.y, planeNormal.z, -d);

        bool validTarget = this->getTranslationTargetPosition(x, y, widgetPosition, this->actionStartPosition);
        if (!validTarget) return false;

        this->actionOffset = widgetPosition - this->actionStartPosition;
        this->actionInProgress = true;
        return true;
    }
    else if (currentMode == TransformationMode::Rotation) {

        Core::Vector3r planeNormal = this->actionNormal;
        Core::Real d = planeNormal.dot(widgetPosition);
        this->actionPlane.set(planeNormal.x, planeNormal.y, planeNormal.z, -d);

        Core::Ray ray = this->camera->getRay(x, y);
        std::vector<Core::Hit> hits;
        Core::Bool hitOccurred = this->raycaster.castRay(ray, hits);

        for (unsigned int i=0; i < hits.size(); i++) {
            if(hits[i].ID = this->activeComponentID) {
                Core::Vector3r startVector = hits[i].Origin - widgetPosition;
                startVector.normalize();
                this->actionStartPosition = hits[i].Origin;
                Core::Vector3r perpVector = startVector.cross(this->actionNormal);
                this->actionPerpendicularPosition = widgetPosition + perpVector;
                this->actionInProgress = true;
                this->actionLastRotation = this->getRotationAngleFromScreenPosition(x, y, widgetPosition, this->actionPerpendicularPosition);
                return true;
            }
        }
    }
    return false;
}

void TransformWidget::endAction(Core::Int32 x, Core::Int32 y) {
    this->actionInProgress = false;
    this->activeComponentID = -1;
    this->rayCastForSelection(x, y);
}

bool TransformWidget::handleDrag(Core::Int32 x, Core::Int32 y) {
    if (this->actionInProgress) {
        this->updateAction(x, y);
        return true;
    }
    return false;
}

void TransformWidget::addTargetObject(Core::WeakPointer<Core::Object3D> object) {
    if (this->hasTargetObject(object)) return;
    this->targetObjects.push_back(object);
    this->updateTransformationForTargetObjects();
}

void TransformWidget::removeTargetObject(Core::WeakPointer<Core::Object3D> object) {
    unsigned int index = 0;
    for (std::vector<Core::WeakPointer<Core::Object3D>>::iterator itr = this->targetObjects.begin(); itr != this->targetObjects.end(); ++itr) {
        Core::WeakPointer<Core::Object3D> targetObject = *itr;
        if(targetObject.get() == object.get()) {
            this->removeTargetObjectAtIndex(index);
            return;
        }
        index++;
    }
}

void TransformWidget::buildTranslationObject() {
    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();

    highlightColor.set(1.0f, 1.0f, 1.0f, 1.0f);
    Core::Real baseLength = 2.0f;
    Core::Real coneLength = 0.4f;
    Core::Real halfLength = (baseLength + coneLength) / 2.0f;
    Core::WeakPointer<Core::Mesh> arrowMesh = Core::GeometryUtils::buildArrowMesh(baseLength, 0.035f, coneLength, 0.15f, 16, highlightColor);
    Core::WeakPointer<Core::Mesh> arrowColliderMesh = Core::GeometryUtils::buildBoxMesh(.15f, baseLength + coneLength, .15f, highlightColor);

    Core::WeakPointer<MeshContainer> xArrow = Core::GeometryUtils::buildMeshContainer(arrowMesh, xMaterial, "XArrow");
    xArrow->getTransform().getLocalMatrix().preRotate(0.0f, 0.0f, 1.0f, -Core::Math::PI / 2.0f);
    xArrow->getTransform().getLocalMatrix().preTranslate(halfLength, 0.0f, 0.0f);
    this->xTranslateID = this->raycaster.addObject(xArrow, arrowColliderMesh);

    Core::WeakPointer<MeshContainer> yArrow = Core::GeometryUtils::buildMeshContainer(arrowMesh, yMaterial, "YArrow");
    yArrow->getTransform().getLocalMatrix().preTranslate(0.0f, halfLength, 0.0f);
    this->yTranslateID = this->raycaster.addObject(yArrow, arrowColliderMesh);

    Core::WeakPointer<MeshContainer> zArrow = Core::GeometryUtils::buildMeshContainer(arrowMesh, zMaterial, "ZArrow");
    zArrow->getTransform().getLocalMatrix().preRotate(1.0f, 0.0f, 0.0f, Core::Math::PI / 2.0f);
    zArrow->getTransform().getLocalMatrix().preTranslate(0.0f, 0.0f, halfLength);
    this->zTranslateID = this->raycaster.addObject(zArrow, arrowColliderMesh);

    this->rootTranslateObject = engine->createObject3D();
    this->rootTranslateObject->setName("TranslationWidget");

    this->rootTranslateObject->addChild(xArrow);
    this->rootTranslateObject->addChild(yArrow);
    this->rootTranslateObject->addChild(zArrow);

    this->rootObject->addChild(this->rootTranslateObject);
}

void TransformWidget::buildRotationObject() {
    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();

    highlightColor.set(1.0f, 1.0f, 1.0f, 1.0f);

    Core::Real torusRadius = 1.5f;
    Core::Real torusTubeRadius = 0.040f;
    Core::WeakPointer<Core::Mesh> torusMesh = Core::GeometryUtils::buildTorusMesh(torusRadius, torusTubeRadius, 32, 16, highlightColor);
    Core::WeakPointer<Core::Mesh> torusColliderMesh = Core::GeometryUtils::buildTorusMesh(torusRadius, torusTubeRadius * 3.0f, 32, 16, highlightColor);

    Core::WeakPointer<MeshContainer> xRing = Core::GeometryUtils::buildMeshContainer(torusMesh, xMaterial, "XRing");
    xRing->getTransform().getLocalMatrix().preRotate(0.0f, 0.0f, -1.0f, Core::Math::PI / 2.0f);
    this->xRotateID = this->raycaster.addObject(xRing, torusColliderMesh);

    Core::WeakPointer<MeshContainer> yRing = Core::GeometryUtils::buildMeshContainer(torusMesh, yMaterial, "YRing");
    this->yRotateID = this->raycaster.addObject(yRing, torusColliderMesh);

    Core::WeakPointer<MeshContainer> zRing = Core::GeometryUtils::buildMeshContainer(torusMesh, zMaterial, "ZRing");
    zRing->getTransform().getLocalMatrix().preRotate(1.0f, 0.0f, 0.0f, Core::Math::PI / 2.0f);
    this->zRotateID = this->raycaster.addObject(zRing, torusColliderMesh);

    this->rootRotateObject = engine->createObject3D();
    this->rootRotateObject->setName("RotationWidget");

    this->rootRotateObject->addChild(xRing);
    this->rootRotateObject->addChild(yRing);
    this->rootRotateObject->addChild(zRing);

    this->rootObject->addChild(this->rootRotateObject);
}

void TransformWidget::removeTargetObjectAtIndex(unsigned int index) {
     this->targetObjects[index] = this->targetObjects[this->targetObjects.size() - 1];
     this->targetObjects.pop_back();
}

bool TransformWidget::hasTargetObject(Core::WeakPointer<Core::Object3D> candidateObject) {
    for (std::vector<Core::WeakPointer<Core::Object3D>>::iterator itr = this->targetObjects.begin(); itr != this->targetObjects.end(); ++itr) {
        Core::WeakPointer<Core::Object3D> targetObject = *itr;
        if(targetObject.get() == candidateObject.get()) return true;
    }
    return false;
}

void TransformWidget::activateTranslationMode() {
    this->currentMode = TransformationMode::Translation;
    this->setChildObjectsActive(this->rootTranslateObject, true);
    this->setChildObjectsActive(this->rootRotateObject, false);
}

void TransformWidget::activateRotationMode() {
    this->currentMode = TransformationMode::Rotation;
    this->setChildObjectsActive(this->rootTranslateObject, false);
    this->setChildObjectsActive(this->rootRotateObject, true);
}

void TransformWidget::rayCastForSelection(Core::Int32 x, Core::Int32 y) {
    this->updateCamera();
    Core::Ray ray = this->camera->getRay(x, y);
    std::vector<Core::Hit> hits;
    Core::Bool hitOccurred = this->raycaster.castRay(ray, hits);

    if (hitOccurred) {
        Core::Hit& hit = hits[0];
        this->activeComponentMesh = hit.Object;
        if (this->activeComponentID != hit.ID) {
            this->resetColors();
            this->activeComponentID = hit.ID;
            if (hit.ID == this->xTranslateID || hit.ID == this->xRotateID) {
                this->xMaterial->setHighlightColor(this->highlightColor);
            }
            else if (hit.ID == this->yTranslateID || hit.ID == this->yRotateID) {
                this->yMaterial->setHighlightColor(this->highlightColor);
            }
            else if (hit.ID == this->zTranslateID || hit.ID == this->zRotateID) {
                this->zMaterial->setHighlightColor(this->highlightColor);
            }
        }
    }
    else {
        this->activeComponentID = -1;
        this->resetColors();
    }
}


void TransformWidget::updateAction(Core::Int32 x, Core::Int32 y) {
    static std::vector<Core::Point3r> newPositions;
    newPositions.resize(0);
    static std::vector<Core::WeakPointer<Core::Object3D>> roots;
    roots.resize(0);

    if (!this->actionInProgress) return;

    Core::WeakPointer<Core::Graphics> graphics = Core::Engine::instance()->getGraphicsSystem();
    Core::Vector3r camDir = Core::Vector3r::Forward;
    Core::WeakPointer<Core::Object3D> cameraObj = this->camera->getOwner();
    Core::Transform& cameraTransform = cameraObj->getTransform();
    Core::Transform& widgetTransform = this->rootObject->getTransform();
    Core::Point3r widgetPosition = widgetTransform.getWorldPosition();

    if (this->currentMode == TransformationMode::Translation) {
        Core::Point3r targetPosition;
        if (!this->getTranslationTargetPosition(x, y, this->actionStartPosition, targetPosition)) return;
        Core::Vector3r translation = targetPosition - widgetPosition +  this->actionOffset;

        SceneUtils::getRootObjects(this->targetObjects, roots);
        for (unsigned int i = 0; i < roots.size(); i ++) {
            Core::Transform& transform = roots[i]->getTransform();
            newPositions.push_back(transform.getWorldPosition() + translation);
        }
        for (unsigned int i = 0; i < roots.size(); i ++) {
            Core::Transform& transform = roots[i]->getTransform();
            transform.setWorldPosition(newPositions[i]);
        }
        widgetTransform.translate(translation, Core::TransformationSpace::World);

    }
    else if (this->currentMode == TransformationMode::Rotation) {
        Core::Real angle = this->getRotationAngleFromScreenPosition(x, y, widgetPosition, this->actionPerpendicularPosition);
        Core::Real angleDiff = angle - this->actionLastRotation;

        SceneUtils::getRootObjects(this->targetObjects, roots);
        for (unsigned int i = 0; i < roots.size(); i ++) {
            Core::Transform& transform = roots[i]->getTransform();
            transform.rotateAround(this->actionNormal, widgetPosition, angleDiff);
        }
        widgetTransform.rotateAround(this->actionNormal, widgetPosition, angleDiff);
        this->actionLastRotation = angle;
    }

}

Core::Real TransformWidget::getRotationAngleFromScreenPosition(Core::Int32 x, Core::Int32 y, Core::Point3r perpStartPos, Core::Point3r perpEndPos) {
    Core::WeakPointer<Core::Graphics> graphics = Core::Engine::instance()->getGraphicsSystem();
    Core::WeakPointer<Core::Object3D> cameraObj = this->camera->getOwner();
    Core::Transform& cameraTransform = cameraObj->getTransform();
    Core::Vector4u viewport = graphics->getViewport();

    Core::Matrix4x4 camMatrix = cameraTransform.getWorldMatrix();
    camMatrix.invert();
    Core::Point3r projectedWidgetPosition = perpStartPos;
    camMatrix.transform(projectedWidgetPosition);
    Core::Point3r projecteActionPerpPosition = perpEndPos;
    camMatrix.transform(projecteActionPerpPosition);

    this->targetCamera->project(projectedWidgetPosition);
    this->targetCamera->project(projecteActionPerpPosition);

    Core::Vector2r widgetScreenPos = Core::Camera::ndcToScreen(projectedWidgetPosition, viewport);
    Core::Vector2r actionPerpScreenPos = Core::Camera::ndcToScreen(projecteActionPerpPosition, viewport);

    Core::Vector2r actionVec((Core::Real)x - widgetScreenPos.x,  (Core::Real)y - widgetScreenPos.y);
    Core::Vector2r perpVec(actionPerpScreenPos.x - widgetScreenPos.x, actionPerpScreenPos.y - widgetScreenPos.y);
    perpVec.normalize();

    Core::Real angle = (-actionVec.dot(perpVec) / 100.0f);

    return angle;
}

bool TransformWidget::getTranslationTargetPosition(Core::Int32 x, Core::Int32 y, Core::Point3r origin, Core::Point3r& out) {
    Core::Ray ray = this->targetCamera->getRay(x, y);
    Core::Hit planeHit;
    Core::Bool intersects = ray.intersectPlane(this->actionPlane, planeHit);
    if (!intersects) return false;

    Core::Point3r intersection = planeHit.Origin;
    Core::Vector3r toIntersection = intersection - origin;
    Core::Real p = toIntersection.dot(this->actionNormal);
    Core::Vector3r offset = this->actionNormal * p;
    Core::Point3r targetPosition = origin + offset;
    out = targetPosition;
    return true;
}

void TransformWidget::resetColors() {
    this->xMaterial->setHighlightColor(xColor);
    this->yMaterial->setHighlightColor(yColor);
    this->zMaterial->setHighlightColor(zColor);
}

void TransformWidget::setChildObjectsActive(Core::WeakPointer<Core::Object3D> parent, bool active) {
    Core::UInt32 childCount = parent->childCount();
    for (Core::UInt32 i = 0; i < childCount; i++) {
        parent->getChild(i)->setActive(active);
    }
}

