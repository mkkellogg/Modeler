#pragma once

#include "Core/Engine.h"
#include "Core/scene/Object3D.h"
#include "Core/scene/RayCaster.h"
#include "Core/color/Color.h"

#include "BasicRimShadowMaterial.h"
#include "Core/geometry/GeometryUtils.h"
#include "CoreScene.h"

class TransformWidget
{
public:

    enum class TransformationMode {
        Translation = 0,
        Rotation = 1,
        Scale = 2
    };

    TransformWidget();
    void init(Core::WeakPointer<Core::Camera> targetCamera);
    void updateTransformationForTargetObjects();
    void updateCamera();
    void render();
    bool startAction(Core::Int32 x, Core::Int32 y);
    void endAction(Core::Int32 x, Core::Int32 y);
    bool handleDrag(Core::Int32 x, Core::Int32 y);
    void rayCastForSelection(Core::Int32 x, Core::Int32 y);
    void addTargetObject(Core::WeakPointer<Core::Object3D> object);
    void removeTargetObject(Core::WeakPointer<Core::Object3D> object);
    bool hasTargetObject(Core::WeakPointer<Core::Object3D> candidateObject);
    void activateTranslationMode();
    void activateRotationMode();

private:
    void buildTranslationObject();
    void buildRotationObject();
    void removeTargetObjectAtIndex(unsigned int index);
    void updateAction(Core::Int32 x, Core::Int32 y);
    Core::Real getRotationAngleFromScreenPosition(Core::Int32 x, Core::Int32 y, Core::Point3r perpStartPos, Core::Point3r perpEndPos);
    bool getTranslationTargetPosition(Core::Int32 x, Core::Int32 y, Core::Point3r origin, Core::Point3r& out);
    void resetColors();
    void setChildObjectsActive(Core::WeakPointer<Core::Object3D> parent, bool active);

    CoreScene* coreScene;
    Core::WeakPointer<Core::Object3D> rootObject;
    Core::WeakPointer<Core::Object3D> rootTranslateObject;
    Core::WeakPointer<Core::Object3D> rootRotateObject;
    std::vector<Core::WeakPointer<Core::Object3D>> targetObjects;
    Core::WeakPointer<Core::Camera> targetCamera;
    Core::RayCaster raycaster;
    Core::WeakPointer<Core::Object3D> cameraObj;
    Core::WeakPointer<Core::Camera> camera;
    Core::Color highlightColor;
    Core::Color xColor;
    Core::Color yColor;
    Core::Color zColor;
    Core::WeakPointer<BasicRimShadowMaterial> xMaterial;
    Core::WeakPointer<BasicRimShadowMaterial> yMaterial;
    Core::WeakPointer<BasicRimShadowMaterial> zMaterial;

    TransformationMode currentMode;

    Core::UInt32 xTranslateID;
    Core::UInt32 yTranslateID;
    Core::UInt32 zTranslateID;

    Core::UInt32 xRotateID;
    Core::UInt32 yRotateID;
    Core::UInt32 zRotateID;

    Core::Int32 activeComponentID;
    Core::WeakPointer<Core::Mesh> activeComponentMesh;

    bool actionInProgress;
    Core::Point3r actionStartPosition;
    Core::Point3r actionPerpendicularPosition;
    Core::Vector3r actionNormal;
    Core::Vector3r actionOffset;
    Core::Vector4r actionPlane;
    Core::Real actionLastRotation;
};
