#pragma once

#include "Core/Engine.h"
#include "Core/scene/Object3D.h"
#include "Core/scene/RayCaster.h"
#include "Core/color/Color.h"

#include "BasicRimShadowMaterial.h"
#include "GeometryUtils.h"
#include "CoreScene.h"

class TransformWidget
{
public:
    TransformWidget();
    void init(Core::WeakPointer<Core::Camera> targetCamera);
    void updateCamera();
    void render();
    bool startAction(Core::Int32 x, Core::Int32 y);
    void endAction(Core::Int32 x, Core::Int32 y);
    bool handleDrag(Core::Int32 x, Core::Int32 y);
    void rayCastForSelection(Core::Int32 x, Core::Int32 y);
    void addTargetObject(Core::WeakPointer<Core::Object3D> object);
    void removeTargetObject(Core::WeakPointer<Core::Object3D> object);
    bool hasTargetObject(Core::WeakPointer<Core::Object3D> candidateObject);
    void update();

private:
    void removeTargetObjectAtIndex(unsigned int index);
    void updateAction(Core::Int32 x, Core::Int32 y);
    bool getTranslationTargetPosition(Core::Int32 x, Core::Int32 y, Core::Point3r origin, Core::Point3r& out);
    void resetColors();

    CoreScene* coreScene;
    Core::WeakPointer<Core::Object3D> rootObject;
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
    Core::UInt32 xTranslateID;
    Core::UInt32 yTranslateID;
    Core::UInt32 zTranslateID;

    Core::Int32 activeComponentID;
    bool actionInProgress;
    Core::Point3r actionStartPosition;
    Core::Vector3r actionNormal;
    Core::Vector3r actionOffset;
    Core::Vector4r actionPlane;
};
