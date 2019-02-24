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
    void init(Core::WeakPointer<Core::Camera> targetCamera, CoreScene& coreScene);
    void updateCamera();
    bool handleDrag(Core::Int32 x, Core::Int32 y);
    void rayCastForTransformWidgetSelection(Core::Int32 x, Core::Int32 y);
    bool startAction(Core::Int32 x, Core::Int32 y);
    void endAction(Core::Int32 x, Core::Int32 y);
    void render();
    void updateForObject(Core::WeakPointer<Core::Object3D> object);

private:
    void updateTransformWidgetAction(Core::Int32 x, Core::Int32 y);
    bool getTransformWidgetTranslationTargetPosition(Core::Int32 x, Core::Int32 y, Core::Point3r origin, Core::Point3r& out);
    void resetTransformWidgetColors();

    CoreScene* coreScene;
    Core::WeakPointer<Core::Camera> targetCamera;
    Core::RayCaster transformWidgetRaycaster;
    Core::WeakPointer<Core::Object3D> transformWidgetCameraObj;
    Core::WeakPointer<Core::Camera> transformWidgetCamera;
    Core::WeakPointer<Core::Object3D> transformWidgetRoot;
    Core::Color transformWidgetHighlightColor;
    Core::Color transformWidgetXColor;
    Core::Color transformWidgetYColor;
    Core::Color transformWidgetZColor;
    Core::WeakPointer<BasicRimShadowMaterial> transformWidgetXMaterial;
    Core::WeakPointer<BasicRimShadowMaterial> transformWidgetYMaterial;
    Core::WeakPointer<BasicRimShadowMaterial> transformWidgetZMaterial;
    Core::UInt32 transformWidgetXTranslateID;
    Core::UInt32 transformWidgetYTranslateID;
    Core::UInt32 transformWidgetZTranslateID;

    Core::Int32 transformWidgetActiveComponentID;
    bool transformWidgetActionInProgress;
    Core::Point3r transformWidgetActionStartPosition;
    Core::Vector3r transformWidgetActionNormal;
    Core::Vector3r transformWidgetActionOffset;
    Core::Vector4r transformWidgetPlane;
};
