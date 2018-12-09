
#include <memory>

#include "Core/geometry/Vector3.h"
#include "Core/math/Matrix4x4.h"
#include "Core/math/Quaternion.h"
#include "Core/util/WeakPointer.h"
#include "Core/Graphics.h"
#include "Core/render/Camera.h"
#include "OrbitControls.h"


OrbitControls::OrbitControls(Core::WeakPointer<Core::Engine> engine, Core::WeakPointer<Core::Camera> targetCamera, Core::WeakPointer<CoreSync> coreSync):
    engine(engine), targetCamera(targetCamera), coreSync(coreSync) {

}

void OrbitControls::handleGesture(GestureAdapter::GestureEvent event) {

    if (event.getType() == GestureAdapter::GestureEventType::Scroll) {

        CoreSync::Runnable runnable = [this, event](Core::WeakPointer<Core::Engine> engine) {
            Core::WeakPointer<Core::Object3D> cameraObjPtr = this->targetCamera->getOwner();
            Core::Vector3r cameraVec;
            cameraVec.set(0, 0, -1);
            cameraObjPtr->getTransform().transform(cameraVec);
            cameraVec = cameraVec * event.scrollDistance;
            cameraObjPtr->getTransform().translate(cameraVec, Core::TransformationSpace::World);
        };
        if (this->coreSync) {
            this->coreSync->run(runnable);
        }

    }
    else if (event.getType() == GestureAdapter::GestureEventType::Drag) {

        Core::Int32 eventStartX = event.start.x;
        Core::Int32 eventStartY = event.start.y;
        Core::Int32 eventEndX = event.end.x;
        Core::Int32 eventEndY = event.end.y;
        GestureAdapter::GesturePointer eventPointer = event.pointer;

        CoreSync::Runnable runnable = [this, eventStartX, eventStartY, eventEndX, eventEndY, eventPointer](Core::WeakPointer<Core::Engine> engine) {

            Core::WeakPointer<Core::Graphics> graphics = this->engine->getGraphicsSystem();
            Core::WeakPointer<Core::Renderer> rendererPtr = graphics->getRenderer();

            Core::Vector4u viewport = graphics->getViewport();
            Core::Real ndcStartX = (Core::Real)eventStartX / (Core::Real)viewport.z * 2.0f - 1.0f;
            Core::Real ndcStartY = (Core::Real)eventStartY / (Core::Real)viewport.w * 2.0f - 1.0f;
            Core::Real ndcEndX = (Core::Real)eventEndX / (Core::Real)viewport.z * 2.0f - 1.0f;
            Core::Real ndcEndY = (Core::Real)eventEndY / (Core::Real)viewport.w * 2.0f - 1.0f;

            Core::Point3r viewStartP(ndcStartX, ndcEndY, 0.25f);
            Core::Point3r viewEndP(ndcEndX, ndcStartY, 0.25f);
            this->targetCamera->unProject(viewStartP);
            this->targetCamera->unProject(viewEndP);

            Core::WeakPointer<Core::Object3D> cameraObjPtr = this->targetCamera->getOwner();

            Core::Matrix4x4 viewMat = cameraObjPtr->getTransform().getWorldMatrix();
            viewMat.transform(viewStartP);
            viewMat.transform(viewEndP);

            Core::Vector3r viewStart(viewStartP.x, viewStartP.y, viewStartP.z);
            Core::Vector3r viewEnd(viewEndP.x, viewEndP.y, viewEndP.z);

            viewStart = Core::Point3r(viewStart.x, viewStart.y, viewStart.z) - this->origin;
            viewEnd = Core::Point3r(viewEnd.x, viewEnd.y, viewEnd.z) - this->origin;

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
            cameraObjPtr->getTransform().transform(cameraPos);
            cameraPos.set(cameraPos.x - this->origin.x, cameraPos.y - this->origin.y, cameraPos.z - this->origin.z);
            Core::Real distanceFromOrigin = cameraPos.magnitude();

            using GesturePointer = GestureAdapter::GesturePointer;
            if (eventPointer == GesturePointer::Secondary) {

                Core::Real rotationScaleFactor = Core::Math::max(distanceFromOrigin, 1.0f);
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

            }
            else if (eventPointer == GesturePointer::Tertiary) {

                Core::Real translationScaleFactor = distanceFromOrigin;
                Core::Vector3r viewDragVector = viewEnd - viewStart;
                viewDragVector.invert();
                viewDragVector = viewDragVector * translationScaleFactor;
                this->origin = this->origin + viewDragVector;
                cameraObjPtr->getTransform().translate(viewDragVector, Core::TransformationSpace::World);
            }
       };
       if (this->coreSync) {
           this->coreSync->run(runnable);
       }
    }

}
