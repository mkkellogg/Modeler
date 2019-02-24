#include "Core/render/Camera.h"

#include "TransformWidget.h"

using MeshContainer = Core::RenderableContainer<Core::Mesh>;

TransformWidget::TransformWidget(): coreScene(nullptr) {
    this->transformWidgetActiveComponentID = -1;
    this->transformWidgetActionInProgress = false;
}

void TransformWidget::init(Core::WeakPointer<Core::Camera> targetCamera, CoreScene& coreScene) {
    Core::WeakPointer<Core::Engine> engine = Core::Engine::instance();

    this->targetCamera = targetCamera;
    this->coreScene = &coreScene;

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
    zArrow->getTransform().getLocalMatrix().preRotate(1.0f, 0.0f, 0.0f, Core::Math::PI / 2.0f);
    zArrow->getTransform().getLocalMatrix().preTranslate(0.0f, 0.0f, halfLength);
    this->transformWidgetZTranslateID = this->transformWidgetRaycaster.addObject(zArrow, arrowColliderMesh);

    this->transformWidgetRoot->addChild(xArrow);
    this->transformWidgetRoot->addChild(yArrow);
    this->transformWidgetRoot->addChild(zArrow);

    this->transformWidgetCameraObj = engine->createObject3D();
    this->transformWidgetCamera = engine->createPerspectiveCamera(this->transformWidgetCameraObj, Core::Camera::DEFAULT_FOV, Core::Camera::DEFAULT_ASPECT_RATIO, 0.1f, 100);

}

void TransformWidget::render() {
    this->transformWidgetCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Color, false);
    this->transformWidgetCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Depth, true);
    this->transformWidgetCamera->setAutoClearRenderBuffer(Core::RenderBufferType::Stencil, true);
    Core::Engine::instance()->getGraphicsSystem()->getRenderer()->renderObjectBasic(this->transformWidgetRoot, this->transformWidgetCamera);
}

bool TransformWidget::handleDrag(Core::Int32 x, Core::Int32 y) {
    if (this->transformWidgetActionInProgress) {
        this->updateTransformWidgetAction(x, y);
        return true;
    }
    return false;
}

void TransformWidget::updateForObject(Core::WeakPointer<Core::Object3D> object) {
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

void TransformWidget::updateCamera() {

    this->transformWidgetCamera->copyFrom(this->targetCamera);

    Core::Point3r transformWidgetPosition;
    Core::Transform& transformWidgetTransform = this->transformWidgetRoot->getTransform();
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

    Core::WeakPointer<Core::Object3D> transformWidgetCameraObj = this->transformWidgetCamera->getOwner();
    transformWidgetCameraObj->getTransform().updateWorldMatrix();
    Core::Point3r transformWidgetCameraPosition;
    targetCameraTransform.transform(transformWidgetCameraPosition);
    Core::Vector3r translation = newTransformWidgetCameraPosition - transformWidgetCameraPosition;
    transformWidgetCameraObj->getTransform().getLocalMatrix().copy(targetCameraTransform.getLocalMatrix());
    transformWidgetCameraObj->getTransform().translate(translation, Core::TransformationSpace::World);
    transformWidgetCameraObj->getTransform().updateWorldMatrix();
}

void TransformWidget::rayCastForTransformWidgetSelection(Core::Int32 x, Core::Int32 y) {
    this->updateCamera();
    Core::WeakPointer<Core::Graphics> graphics = Core::Engine::instance()->getGraphicsSystem();
    Core::Vector4u viewport = graphics->getViewport();
    Core::Ray ray = this->transformWidgetCamera->getRay(viewport, x, y);
    std::vector<Core::Hit> hits;
    Core::Bool hitOccurred = this->transformWidgetRaycaster.castRay(ray, hits);

    if (hitOccurred) {
        Core::Hit& hit = hits[0];
        Core::WeakPointer<Core::Mesh> hitObject = hit.Object;
        if (this->transformWidgetActiveComponentID != hit.ID) {
            this->resetTransformWidgetColors();
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
        this->resetTransformWidgetColors();
    }
}

bool TransformWidget::startAction(Core::Int32 x, Core::Int32 y) {
    if (this->transformWidgetActionInProgress) return true;
    if (this->transformWidgetActiveComponentID == -1) return false;
    Core::Point3r transformWidgetPosition;
    Core::Transform& transformWidgetTransform = this->transformWidgetRoot->getTransform();
    transformWidgetTransform.updateWorldMatrix();
    transformWidgetTransform.transform(transformWidgetPosition);

    Core::Vector3r planeNormal;
    if (this->transformWidgetActiveComponentID == this->transformWidgetXTranslateID) {
        this->transformWidgetActionNormal.set(1.0f, 0.0f, 0.0f);
        planeNormal.set(0.0f, 0.0f, 1.0f);
    }
    else if (this->transformWidgetActiveComponentID == this->transformWidgetYTranslateID) {
        this->transformWidgetActionNormal.set(0.0f, 1.0f, 0.0f);
        planeNormal.set(0.0f, 0.0f, 1.0f);
    }
    else if (this->transformWidgetActiveComponentID == this->transformWidgetZTranslateID) {
        this->transformWidgetActionNormal.set(0.0f, 0.0f, 1.0f);
        planeNormal.set(0.0f, 1.0f, 0.0f);
    }
    transformWidgetTransform.transform(this->transformWidgetActionNormal);
    transformWidgetTransform.transform(planeNormal);

    Core::Real d = planeNormal.dot(transformWidgetPosition);
    this->transformWidgetPlane.set(planeNormal.x, planeNormal.y, planeNormal.z, -d);

    bool validTarget = this->getTransformWidgetTranslationTargetPosition(x, y, transformWidgetPosition, this->transformWidgetActionStartPosition);
    if (!validTarget) return false;
    this->transformWidgetActionOffset = transformWidgetPosition - this->transformWidgetActionStartPosition;
    this->transformWidgetActionInProgress = true;
    return true;
}

void TransformWidget::endAction(Core::Int32 x, Core::Int32 y) {
    this->transformWidgetActionInProgress = false;
    this->transformWidgetActiveComponentID = -1;
    this->rayCastForTransformWidgetSelection(x, y);
}

void TransformWidget::updateTransformWidgetAction(Core::Int32 x, Core::Int32 y) {
    if (!this->transformWidgetActionInProgress) return;
    Core::Point3r targetPosition;
    bool validTarget = this->getTransformWidgetTranslationTargetPosition(x, y, this->transformWidgetActionStartPosition, targetPosition);

    if (!validTarget) return;

    Core::Point3r transformWidgetPosition;
    Core::Transform& transformWidgetTransform = this->transformWidgetRoot->getTransform();
    transformWidgetTransform.updateWorldMatrix();
    transformWidgetTransform.transform(transformWidgetPosition);

    Core::Vector3r translation = targetPosition - transformWidgetPosition +  this->transformWidgetActionOffset;
    transformWidgetTransform.translate(translation, Core::TransformationSpace::World);

    this->coreScene->getSelectedObject()->getTransform().translate(translation, Core::TransformationSpace::World);
}

bool TransformWidget::getTransformWidgetTranslationTargetPosition(Core::Int32 x, Core::Int32 y, Core::Point3r origin, Core::Point3r& out) {
    Core::WeakPointer<Core::Graphics> graphics = Core::Engine::instance()->getGraphicsSystem();
    Core::Vector4u viewport = graphics->getViewport();
    Core::Ray ray = this->targetCamera->getRay(viewport, x, y);

    Core::Hit planeHit;
    Core::Bool intersects = ray.intersectPlane(this->transformWidgetPlane, planeHit);
    if (!intersects) return false;

    Core::Point3r intersection = planeHit.Origin;

    Core::Vector3r toIntersection = intersection - origin;
    Core::Real p = toIntersection.dot(this->transformWidgetActionNormal);
    Core::Vector3r offset = this->transformWidgetActionNormal * p;
    Core::Point3r targetPosition = origin + offset;
    out = targetPosition;
    return true;
}

void TransformWidget::resetTransformWidgetColors() {
    this->transformWidgetXMaterial->setHighlightColor(transformWidgetXColor);
    this->transformWidgetYMaterial->setHighlightColor(transformWidgetYColor);
    this->transformWidgetZMaterial->setHighlightColor(transformWidgetZColor);
}

