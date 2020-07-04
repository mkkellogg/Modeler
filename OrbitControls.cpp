
#include <memory>

#include "Core/geometry/Vector3.h"
#include "Core/geometry/Plane.h"
#include "Core/math/Matrix4x4.h"
#include "Core/math/Quaternion.h"
#include "Core/util/WeakPointer.h"
#include "Core/Graphics.h"
#include "Core/render/Camera.h"
#include "OrbitControls.h"


OrbitControls::OrbitControls(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Camera> targetCamera, Core::WeakPointer<CoreSync> coreSync):
    engine(engine), targetCamera(targetCamera), coreSync(coreSync), moveFrames(0) {

}

void OrbitControls::handleGesture(GestureAdapter::GestureEvent event) {

    if (event.getType() == GestureAdapter::GestureEventType::Scroll) {
        Core::WeakPointer<Core::Object3D> cameraObjPtr = this->targetCamera->getOwner();
        Core::Vector3r cameraVec;
        cameraVec.set(0, 0, -1);
        cameraObjPtr->getTransform().applyTransformationTo(cameraVec);
        cameraVec = cameraVec * event.scrollDistance;
        cameraObjPtr->getTransform().translate(cameraVec, Core::TransformationSpace::World);
    }
    else if (event.getType() == GestureAdapter::GestureEventType::Drag) {

        Core::Int32 eventStartX = event.start.x;
        Core::Int32 eventStartY = event.start.y;

        if (this->moveFrames > 0) {
            eventStartX = this->lastMoveX;
            eventStartY = this->lastMoveY;
        }

        Core::Int32 eventEndX = event.end.x;
        Core::Int32 eventEndY = event.end.y;
        GestureAdapter::GesturePointer eventPointer = event.pointer;

        Core::WeakPointer<Core::Graphics> graphics = this->engine->getGraphicsSystem();
        Core::WeakPointer<Core::Renderer> rendererPtr = graphics->getRenderer();

        Core::WeakPointer<Core::Object3D> cameraObject = this->targetCamera->getOwner();
        cameraObject->getTransform().updateWorldMatrix();
        Core::Matrix4x4 camWorldMat = cameraObject->getTransform().getWorldMatrix();

        Core::Point3r cameraPos;
        camWorldMat.transform(cameraPos);
        Core::Vector3r cameraToOrigin = this->origin - cameraPos;
        Core::Point3r originRelativeCameraPos(-cameraToOrigin.x, -cameraToOrigin.y, -cameraToOrigin.z);

        if (eventPointer == GestureAdapter::GesturePointer::Secondary) {

            Core::Real theta = ((Core::Real)eventEndX - (Core::Real)eventStartX) * -.0001f;
            Core::Real phi = ((Core::Real)eventEndY - (Core::Real)eventStartY) * -.0001f;
            Core::Real cosPhi = Core::Math::cos(phi);

            Core::Vector3r cameraPosVec(originRelativeCameraPos.x, originRelativeCameraPos.y, originRelativeCameraPos.z);
            cameraPosVec.normalize();
            Core::Real cameraPosUpDot = cameraPosVec.dot(Core::Vector3r::Up);

            if (theta != 0.0f || phi != 0.0f) {

                Core::Real epsilon = 0.01f;
                Core::Real rotationScaleFactor = 25.0f;

                Core::Quaternion quatTheta = Core::Quaternion::fromAngleAxis(theta * rotationScaleFactor, Core::Vector3r::Up);
                Core::Vector3r phiAxis = Core::Vector3r::Right;
                camWorldMat.transform(phiAxis);
                Core::Real finalPhi = phi * rotationScaleFactor;
                Core::Quaternion quatPhi = Core::Quaternion::fromAngleAxis(finalPhi, phiAxis);
                Core::Matrix4x4 rotTheta = quatTheta.rotationMatrix();
                Core::Matrix4x4 rotPhi = quatPhi.rotationMatrix();

                Core::Matrix4x4 worldTransformation;
                worldTransformation.preTranslate(-this->origin.x, -this->origin.y, -this->origin.z);
                Core::Real totalPhi = Core::Math::aCos(cameraPosUpDot) + finalPhi;
                if ((totalPhi < Core::Math::PI - epsilon || phi < 0.0f) && (totalPhi > epsilon || phi > 0.0f)) worldTransformation.preMultiply(rotPhi);
                worldTransformation.preMultiply(rotTheta);
                worldTransformation.preTranslate(this->origin.x, this->origin.y, this->origin.z);

                cameraObject->getTransform().transformBy(worldTransformation, Core::TransformationSpace::World);
                cameraObject->getTransform().lookAt(this->origin, Core::Vector3r::Up);
                this->lastMoveX = event.end.x;
                this->lastMoveY = event.end.y;
            }
        }
        else if (eventPointer == GestureAdapter::GesturePointer::Tertiary) {
            Core::Real ndcZ = eventPointer == GestureAdapter::GesturePointer::Tertiary ? 0.5f : 0.0f;

            Core::Point3r worldStartP = this->targetCamera->unProject(eventStartX, eventStartY, ndcZ);
            Core::Point3r worldEndP = this->targetCamera->unProject(eventEndX, eventEndY, ndcZ);
            camWorldMat.transform(worldStartP);
            camWorldMat.transform(worldEndP);

            Core::Vector3r cameraToAdjustedOrigin = cameraToOrigin;
            cameraToAdjustedOrigin.normalize();
            cameraToAdjustedOrigin = cameraToAdjustedOrigin * 5.0f;
            Core::Point3r adjustedOrigin = cameraPos + cameraToAdjustedOrigin;

            Core::Vector3r adjustedWorldStartV = worldStartP - adjustedOrigin;
            Core::Vector3r adjustedWorldEndV = worldEndP - adjustedOrigin;

            Core::Real translationScaleFactor = originRelativeCameraPos.magnitude();
            Core::Vector3r viewDragVector = adjustedWorldEndV - adjustedWorldStartV;
            viewDragVector.invert();
            viewDragVector = viewDragVector * translationScaleFactor;
            this->origin = this->origin + viewDragVector;
            cameraObject->getTransform().translate(viewDragVector, Core::TransformationSpace::World);
            this->lastMoveX = event.end.x;
            this->lastMoveY = event.end.y;
        }
    }

    if (this->moveFrames == 0) {
        this->lastMoveX = event.start.x;
        this->lastMoveY = event.start.y;
    }

    this->moveFrames++;
}

void OrbitControls::setOrigin(Core::Real x, Core::Real y, Core::Real z) {
    this->origin.set(x, y, z);
}

Core::Point3r OrbitControls::getOrigin() {
    Core::Point3r temp = this->origin;
    return temp;
}

void OrbitControls::resetMove() {
    this->moveFrames = 0;
}
