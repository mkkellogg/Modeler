
#include <memory>

#include "Core/geometry/Vector3.h"
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

        Core::Vector4u viewport = graphics->getViewport();
        Core::Real ndcStartX = (Core::Real)eventStartX / (Core::Real)viewport.z * 2.0f - 1.0f;
        Core::Real ndcStartY = (Core::Real)eventStartY / (Core::Real)viewport.w * 2.0f - 1.0f;
        Core::Real ndcEndX = (Core::Real)eventEndX / (Core::Real)viewport.z * 2.0f - 1.0f;
        Core::Real ndcEndY = (Core::Real)eventEndY / (Core::Real)viewport.w * 2.0f - 1.0f;
        Core::Real ndcZ = eventPointer == GestureAdapter::GesturePointer::Tertiary ? 0.5f : 0.0f;

        Core::WeakPointer<Core::Object3D> cameraObject = this->targetCamera->getOwner();
        cameraObject->getTransform().updateWorldMatrix();
        Core::Matrix4x4 camWorldMat = cameraObject->getTransform().getWorldMatrix();

        Core::Point3r worldStartP(ndcStartX, ndcEndY, ndcZ);
        Core::Point3r worldEndP(ndcEndX, ndcStartY, ndcZ);
        this->targetCamera->unProject(worldStartP);
        this->targetCamera->unProject(worldEndP);
        camWorldMat.transform(worldStartP);
        camWorldMat.transform(worldEndP);

        Core::Point3r cameraPos;
        camWorldMat.transform(cameraPos);
        Core::Vector3r cameraToOrigin = this->origin - cameraPos;
        Core::Point3r originRelativeCameraPos(-cameraToOrigin.x, -cameraToOrigin.y, -cameraToOrigin.z);

        Core::Vector3r cameraToAdjustedOrigin = cameraToOrigin;
        cameraToAdjustedOrigin.normalize();
        cameraToAdjustedOrigin = cameraToAdjustedOrigin * 5.0f;
        Core::Point3r adjustedOrigin = cameraPos + cameraToAdjustedOrigin;
        Core::Vector3r adjustedWorldStartV = worldStartP - adjustedOrigin;
        Core::Vector3r adjustedWorldEndV = worldEndP - adjustedOrigin;

        Core::Vector3r adjustedWorldStartVN = Core::Vector3r(adjustedWorldStartV.x, adjustedWorldStartV.y, adjustedWorldStartV.z);
        adjustedWorldStartVN.normalize();
        Core::Vector3r adjustedWorldEndVN = Core::Vector3r(adjustedWorldEndV.x, adjustedWorldEndV.y, adjustedWorldEndV.z);
        adjustedWorldEndVN.normalize();

        if (eventPointer == GestureAdapter::GesturePointer::Secondary) {
            Core::Real cosRotationAngle = Core::Vector3r::dot(adjustedWorldStartVN, adjustedWorldEndVN);
            Core::Real rotationAngle = 0.0f;
            if (cosRotationAngle < 1.0f && cosRotationAngle > -1.0f) {
                rotationAngle = Core::Math::aCos(cosRotationAngle);
            }
            Core::Vector3r rotAxis;
            Core::Vector3r::cross(adjustedWorldEndV, adjustedWorldStartV, rotAxis);
            rotAxis.normalize();

            if (rotationAngle > 0.0f) {

                Core::Vector3r cameraPosVec(originRelativeCameraPos.x, originRelativeCameraPos.y, originRelativeCameraPos.z);
                cameraPosVec.normalize();
                Core::Vector3r up(0.0f, 1.0f, 0.0f);
                Core::Real curDot = up.dot(cameraPosVec);

                Core::Vector3r endVec = worldEndP - this->origin;
                endVec.normalize();
                Core::Vector3r startVec = worldStartP - this->origin;
                startVec.normalize();

                Core::Real oldDot =  startVec.dot(up);
                Core::Real newDot =  endVec.dot(up);

                 Core::Real rotationScaleFactor = 30.0f * (1.0f - curDot/2.0f);
                if (curDot <= .999 || newDot  > oldDot) {

                    Core::Quaternion qA = Core::Quaternion::fromAngleAxis(rotationAngle * rotationScaleFactor, rotAxis);
                    Core::Matrix4x4 rot = qA.rotationMatrix();

                    Core::Matrix4x4 worldTransformation;
                    worldTransformation.preTranslate(-this->origin.x, -this->origin.y, -this->origin.z);
                    worldTransformation.preMultiply(rot);
                    worldTransformation.preTranslate(this->origin.x, this->origin.y, this->origin.z);

                    cameraObject->getTransform().transformBy(worldTransformation, Core::TransformationSpace::World);
                    cameraObject->getTransform().lookAt(this->origin, up);
                    this->lastMoveX = event.end.x;
                    this->lastMoveY = event.end.y;
                }
                else {
                    startVec = worldStartP - this->origin;
                    startVec.y = 0.0f;
                    endVec = worldEndP - this->origin;
                    endVec.y = 0.0f;

                    if(endVec.squareMagnitude() >= 0.00001 && startVec.squareMagnitude() >= .00001) {

                        endVec.normalize();
                        startVec.normalize();

                        Core::Real cosAngle = startVec.dot(endVec);

                        if (cosAngle < 1) {

                            Core::Vector3r crossDir;
                            Core::Vector3r::cross(endVec, startVec, crossDir);
                            Core::Real dirFactor = crossDir.y > 0 ? 1.0f : -1.0f;
                            Core::Real angle = Core::Math::aCos(cosAngle) * dirFactor;

                            Core::Quaternion qA = Core::Quaternion::fromAngleAxis(angle * rotationScaleFactor * 1.5f, up);
                            cameraObject->getTransform().transformBy(qA.rotationMatrix(), Core::TransformationSpace::World);
                            cameraObject->getTransform().lookAt(this->origin, up);

                            this->lastMoveX = event.end.x;
                            this->lastMoveY = event.end.y;
                        }
                    }

                }
            }

        }
        else if (eventPointer == GestureAdapter::GesturePointer::Tertiary) {
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

void OrbitControls::resetMove() {
    this->moveFrames = 0;
}
