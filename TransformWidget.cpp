#include "Core/render/Camera.h"

#include "TransformWidget.h"

using MeshContainer = Core::RenderableContainer<Core::Mesh>;

TransformWidget::TransformWidget(): coreScene(nullptr) {
    this->activeComponentID = -1;
    this->actionInProgress = false;
}

void TransformWidget::init(Core::WeakPointer<Core::Camera> targetCamera, CoreScene& coreScene) {
    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();

    this->targetCamera = targetCamera;
    this->coreScene = &coreScene;

    xMaterial = engine->createMaterial<BasicRimShadowMaterial>();
    xMaterial->setHighlightLowerBound(0.6f);
    xMaterial->setHighlightScale(1.25f);
    xMaterial->setDepthTestEnabled(true);
    yMaterial = Core::WeakPointer<Core::Material>::dynamicPointerCast<BasicRimShadowMaterial>(xMaterial->clone());
    zMaterial = Core::WeakPointer<Core::Material>::dynamicPointerCast<BasicRimShadowMaterial>(xMaterial->clone());

    this->rootObject = engine->createObject3D();
    this->rootObject->setName("TransformWidget");

    highlightColor.set(1.0f, 1.0f, 1.0f, 1.0f);
    Core::Real baseLength = 2.0f;
    Core::Real coneLength = 0.4f;
    Core::Real halfLength = (baseLength + coneLength) / 2.0f;
    Core::WeakPointer<Core::Mesh> arrowMesh = GeometryUtils::buildArrowMesh(baseLength, 0.035f, coneLength, 0.15f, 16, highlightColor);
    Core::WeakPointer<Core::Mesh> arrowColliderMesh = GeometryUtils::buildBoxMesh(.15f, baseLength + coneLength, .15f, highlightColor);

    xColor.set(1.0f, 0.0f, 0.0f, 1.0f);
    xMaterial->setHighlightColor(xColor);
    Core::WeakPointer<MeshContainer> xArrow = GeometryUtils::buildMeshContainer(arrowMesh, xMaterial, "XArrow");
    xArrow->getTransform().getLocalMatrix().preRotate(0.0f, 0.0f, 1.0f, -Core::Math::PI / 2.0f);
    xArrow->getTransform().getLocalMatrix().preTranslate(halfLength, 0.0f, 0.0f);
    this->xTranslateID = this->raycaster.addObject(xArrow, arrowColliderMesh);

    yColor.set(0.0f, 1.0f, 0.0f, 1.0f);
    yMaterial->setHighlightColor(yColor);
    Core::WeakPointer<MeshContainer> yArrow = GeometryUtils::buildMeshContainer(arrowMesh, yMaterial, "YArrow");
    yArrow->getTransform().getLocalMatrix().preTranslate(0.0f, halfLength, 0.0f);
    this->yTranslateID = this->raycaster.addObject(yArrow, arrowColliderMesh);

    zColor.set(0.0f, 0.0f, 1.0f, 1.0f);
    zMaterial->setHighlightColor(zColor);
    Core::WeakPointer<MeshContainer> zArrow = GeometryUtils::buildMeshContainer(arrowMesh, zMaterial, "ZArrow");
    zArrow->getTransform().getLocalMatrix().preRotate(1.0f, 0.0f, 0.0f, Core::Math::PI / 2.0f);
    zArrow->getTransform().getLocalMatrix().preTranslate(0.0f, 0.0f, halfLength);
    this->zTranslateID = this->raycaster.addObject(zArrow, arrowColliderMesh);

    this->rootObject->addChild(xArrow);
    this->rootObject->addChild(yArrow);
    this->rootObject->addChild(zArrow);

    this->cameraObj = engine->createObject3D();
    this->camera = engine->createPerspectiveCamera(this->cameraObj, Core::Camera::DEFAULT_FOV, Core::Camera::DEFAULT_ASPECT_RATIO, 0.1f, 100);

}

void TransformWidget::updateCamera() {

    this->camera->copyFrom(this->targetCamera);

    Core::Point3r transformWidgetPosition;
    Core::Transform& transformWidgetTransform = this->rootObject->getTransform();
    transformWidgetTransform.updateWorldMatrix();
    transformWidgetTransform.transform(transformWidgetPosition);

    Core::Point3r targetCameraPosition;
    Core::Transform& targetCameraTransform = this->targetCamera->getOwner()->getTransform();
    targetCameraTransform.updateWorldMatrix();
    targetCameraTransform.transform(targetCameraPosition);

    Core::Vector3r transformWidgetTotargetCamera = targetCameraPosition - transformWidgetPosition;
    transformWidgetTotargetCamera.normalize();
    transformWidgetTotargetCamera.scale(18.0f);

    Core::Point3r newTransformWidgetCameraPosition = transformWidgetPosition + transformWidgetTotargetCamera;

    Core::WeakPointer<Core::Object3D> transformWidgetCameraObj = this->camera->getOwner();
    transformWidgetCameraObj->getTransform().updateWorldMatrix();
    Core::Point3r transformWidgetCameraPosition;
    targetCameraTransform.transform(transformWidgetCameraPosition);
    Core::Vector3r translation = newTransformWidgetCameraPosition - transformWidgetCameraPosition;
    transformWidgetCameraObj->getTransform().getLocalMatrix().copy(targetCameraTransform.getLocalMatrix());
    transformWidgetCameraObj->getTransform().translate(translation, Core::TransformationSpace::World);
    transformWidgetCameraObj->getTransform().updateWorldMatrix();
}

void TransformWidget::render() {
    this->camera->setAutoClearRenderBuffer(Core::RenderBufferType::Color, false);
    this->camera->setAutoClearRenderBuffer(Core::RenderBufferType::Depth, true);
    this->camera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, true);
    Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderObjectBasic(this->rootObject, this->camera);
}

bool TransformWidget::startAction(Core::Int32 x, Core::Int32 y) {
    if (this->actionInProgress) return true;
    if (this->activeComponentID == -1) return false;
    Core::Point3r transformWidgetPosition;
    Core::Transform& transformWidgetTransform = this->rootObject->getTransform();
    transformWidgetTransform.updateWorldMatrix();
    transformWidgetTransform.transform(transformWidgetPosition);

    Core::Vector3r planeNormal;
    if (this->activeComponentID == this->xTranslateID) {
        this->actionNormal.set(1.0f, 0.0f, 0.0f);
        planeNormal.set(0.0f, 0.0f, 1.0f);
    }
    else if (this->activeComponentID == this->yTranslateID) {
        this->actionNormal.set(0.0f, 1.0f, 0.0f);
        planeNormal.set(0.0f, 0.0f, 1.0f);
    }
    else if (this->activeComponentID == this->zTranslateID) {
        this->actionNormal.set(0.0f, 0.0f, 1.0f);
        planeNormal.set(0.0f, 1.0f, 0.0f);
    }
    transformWidgetTransform.transform(this->actionNormal);
    transformWidgetTransform.transform(planeNormal);

    Core::Real d = planeNormal.dot(transformWidgetPosition);
    this->actionPlane.set(planeNormal.x, planeNormal.y, planeNormal.z, -d);

    bool validTarget = this->getTransformWidgetTranslationTargetPosition(x, y, transformWidgetPosition, this->actionStartPosition);
    if (!validTarget) return false;
    this->actionOffset = transformWidgetPosition - this->actionStartPosition;
    this->actionInProgress = true;
    return true;
}

void TransformWidget::endAction(Core::Int32 x, Core::Int32 y) {
    this->actionInProgress = false;
    this->activeComponentID = -1;
    this->rayCastForTransformWidgetSelection(x, y);
}

bool TransformWidget::handleDrag(Core::Int32 x, Core::Int32 y) {
    if (this->actionInProgress) {
        this->updateTransformWidgetAction(x, y);
        return true;
    }
    return false;
}

void TransformWidget::setTargetObject(Core::WeakPointer<Core::Object3D> object) {
     Core::Transform& objectTransform = object->getTransform();
     objectTransform.updateWorldMatrix();

     Core::Point3r origin;
     Core::Vector3r forward = Core::Vector3r::Forward;
     Core::Vector3r up = Core::Vector3r::Up;

     objectTransform.transform(origin);
     objectTransform.transform(forward);
     objectTransform.transform(up);

     Core::Transform& transformWidgetTransform = this->rootObject->getTransform();
     transformWidgetTransform.getLocalMatrix().lookAt(origin, origin + forward, up);
}

void TransformWidget::rayCastForTransformWidgetSelection(Core::Int32 x, Core::Int32 y) {
    this->updateCamera();
    Core::WeakPointer<Core::Graphics> graphics = Core::Engine::instance()->getGraphicsSystem();
    Core::Vector4u viewport = graphics->getViewport();
    Core::Ray ray = this->camera->getRay(viewport, x, y);
    std::vector<Core::Hit> hits;
    Core::Bool hitOccurred = this->raycaster.castRay(ray, hits);

    if (hitOccurred) {
        Core::Hit& hit = hits[0];
        Core::WeakPointer<Core::Mesh> hitObject = hit.Object;
        if (this->activeComponentID != hit.ID) {
            this->resetTransformWidgetColors();
            this->activeComponentID = hit.ID;
            if (hit.ID == this->xTranslateID) {
                this->xMaterial->setHighlightColor(this->highlightColor);
            }
            else if (hit.ID == this->yTranslateID) {
                this->yMaterial->setHighlightColor(this->highlightColor);
            }
            else if (hit.ID == this->zTranslateID) {
                this->zMaterial->setHighlightColor(this->highlightColor);
            }
        }
    }
    else {
        this->activeComponentID = -1;
        this->resetTransformWidgetColors();
    }
}


void TransformWidget::updateTransformWidgetAction(Core::Int32 x, Core::Int32 y) {
    if (!this->actionInProgress) return;
    Core::Point3r targetPosition;
    bool validTarget = this->getTransformWidgetTranslationTargetPosition(x, y, this->actionStartPosition, targetPosition);

    if (!validTarget) return;

    Core::Point3r transformWidgetPosition;
    Core::Transform& transformWidgetTransform = this->rootObject->getTransform();
    transformWidgetTransform.updateWorldMatrix();
    transformWidgetTransform.transform(transformWidgetPosition);

    Core::Vector3r translation = targetPosition - transformWidgetPosition +  this->actionOffset;
    transformWidgetTransform.translate(translation, Core::TransformationSpace::World);

    this->coreScene->getSelectedObject()->getTransform().translate(translation, Core::TransformationSpace::World);
}

bool TransformWidget::getTransformWidgetTranslationTargetPosition(Core::Int32 x, Core::Int32 y, Core::Point3r origin, Core::Point3r& out) {
    Core::WeakPointer<Core::Graphics> graphics = Core::Engine::instance()->getGraphicsSystem();
    Core::Vector4u viewport = graphics->getViewport();
    Core::Ray ray = this->targetCamera->getRay(viewport, x, y);

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

void TransformWidget::resetTransformWidgetColors() {
    this->xMaterial->setHighlightColor(xColor);
    this->yMaterial->setHighlightColor(yColor);
    this->zMaterial->setHighlightColor(zColor);
}

