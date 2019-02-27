
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

        Core::Real ndcZ = 0.0f;
        if (eventPointer == GestureAdapter::GesturePointer::Tertiary) ndcZ = 0.5f;

        Core::Point3r viewStartP(ndcStartX, ndcEndY, ndcZ);
        Core::Point3r viewEndP(ndcEndX, ndcStartY, ndcZ);
        this->targetCamera->unProject(viewStartP);
        this->targetCamera->unProject(viewEndP);

        Core::WeakPointer<Core::Object3D> cameraObjPtr = this->targetCamera->getOwner();

        cameraObjPtr->getTransform().updateWorldMatrix();
        Core::Matrix4x4 viewMat = cameraObjPtr->getTransform().getWorldMatrix();
        viewMat.transform(viewStartP);
        viewMat.transform(viewEndP);

        Core::Vector3r viewStart(viewStartP.x, viewStartP.y, viewStartP.z);
        Core::Vector3r viewEnd(viewEndP.x, viewEndP.y, viewEndP.z);

        Core::Point3r camPos;
        cameraObjPtr->getTransform().getWorldMatrix().transform(camPos);

        Core::Vector3r toOrigin = this->origin - camPos;
        toOrigin.normalize();
        toOrigin = toOrigin * 5.0f;

        Core::Point3r tempOrigin = camPos + toOrigin;

        viewStart = Core::Point3r(viewStart.x, viewStart.y, viewStart.z) - tempOrigin;
        viewEnd = Core::Point3r(viewEnd.x, viewEnd.y, viewEnd.z) - tempOrigin;

        Core::Vector3r viewStartN = Core::Vector3r(viewStart.x, viewStart.y, viewStart.z);
        viewStartN.normalize();

        Core::Vector3r viewEndN = Core::Vector3r(viewEnd.x, viewEnd.y, viewEnd.z);
        viewEndN.normalize();

        Core::Real dot = Core::Vector3r::dot(viewStartN, viewEndN);
        Core::Real angle = 0.0f;
        if (dot < 1.0f && dot > -1.0f) {
            angle = Core::Math::aCos(dot);
        }
        else if (dot <= -1.0f) {
            angle = 180.0f;
        }

        Core::Vector3r rotAxis;
        Core::Vector3r::cross(viewEnd, viewStart, rotAxis);
        rotAxis.normalize();

        Core::Point3r cameraPos;
        cameraPos.set(0, 0, 0);
        cameraObjPtr->getTransform().applyTransformationTo(cameraPos);
        cameraPos.set(cameraPos.x - this->origin.x, cameraPos.y - this->origin.y, cameraPos.z - this->origin.z);
        Core::Real distanceFromOrigin = cameraPos.magnitude();

        if (eventPointer == GestureAdapter::GesturePointer::Secondary) {
            if (angle > 0.0f) {
                Core::Real rotationScaleFactor = 30.0f;//Core::Math::max(distanceFromOrigin, 1.0f);
                Core::Quaternion qA;
                qA.fromAngleAxis(angle * rotationScaleFactor, rotAxis);
                Core::Matrix4x4 rot = qA.rotationMatrix();

                Core::Vector3r orgVec(this->origin.x, this->origin.y, this->origin.z);
                orgVec.invert();
                Core::Matrix4x4 worldTransformation;
                worldTransformation.setIdentity();
                worldTransformation.preTranslate(orgVec);
                worldTransformation.preMultiply(rot);
                orgVec.invert();
                worldTransformation.preTranslate(orgVec);
                cameraObjPtr->getTransform().transformBy(worldTransformation, Core::TransformationSpace::World);
                cameraObjPtr->getTransform().lookAt(this->origin);

                this->lastMoveX = event.end.x;
                this->lastMoveY = event.end.y;
            }
        }
        else if (eventPointer == GestureAdapter::GesturePointer::Tertiary) {
            Core::Real translationScaleFactor = distanceFromOrigin;
            Core::Vector3r viewDragVector = viewEnd - viewStart;
            viewDragVector.invert();
            viewDragVector = viewDragVector * translationScaleFactor;
            this->origin = this->origin + viewDragVector;
            cameraObjPtr->getTransform().translate(viewDragVector, Core::TransformationSpace::World);
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
