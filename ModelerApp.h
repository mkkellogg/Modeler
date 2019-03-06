#pragma once

#include <functional>

#include "Core/Engine.h"
#include "Core/scene/Object3D.h"
#include "Core/material/BasicTexturedMaterial.h"
#include "Core/material/BasicColoredMaterial.h"
#include "Core/material/BasicExtrusionMaterial.h"
#include "Core/material/OutlineMaterial.h"
#include "Core/material/Shader.h"
#include "Core/scene/RayCaster.h"
#include "Core/color/Color.h"

#include "CoreScene.h"
#include "CoreSync.h"
#include "MouseAdapter.h"
#include "GestureAdapter.h"
#include "PipedEventAdapter.h"
#include "OrbitControls.h"
#include "BasicRimShadowMaterial.h"
#include "TransformWidget.h"


class RenderWindow;
class MainGUI;

class ModelerApp {
public:

    using ModelerAppLifecycleEventCallback = std::function<void()>;

    ModelerApp();
    void init();
    void setRenderWindow(RenderWindow* renderWindow);
    void loadModel(const std::string& path, float scale, float smoothingThreshold, bool zUp, bool usePhysicalMaterial);
    CoreScene& getCoreScene();
    void onUpdate(ModelerAppLifecycleEventCallback callback);
    std::shared_ptr<CoreSync> getCoreSync();

private:
    void engineReady(Core::WeakPointer<Core::Engine> engine);
    void setupHighlightMaterials();
    void setupEnvironmentMaps();
    void gesture(GestureAdapter::GestureEvent event);
    void mouseButton(MouseAdapter::MouseEventType type, Core::UInt32 button, Core::Int32 x, Core::Int32 y);
    void rayCastForObjectSelection(Core::Int32 x, Core::Int32 y, bool setSelectedObject, bool multiSelect);
    void setupRenderCamera();
    void setupDefaultObjects();
    void setupLights();
    void updateLights();
    void resolveOnUpdateCallbacks();
    void addObjectToSceneRaycaster(Core::WeakPointer<Core::Object3D> object, Core::WeakPointer<Core::Mesh> mesh);
    void preRenderCallback();
    void postRenderCallback();
    void renderOnce(const std::vector<Core::WeakPointer<Core::Object3D>>& objects, Core::WeakPointer<Core::Camera> camera, Core::WeakPointer<Core::Material> material);

    RenderWindow* renderWindow;
    bool engineIsReady = false;
    CoreScene coreScene;
    Core::WeakPointer<Core::Camera> renderCamera;
    Core::WeakPointer<Core::Engine> engine;
    Core::RayCaster sceneRaycaster;
    std::unordered_map<Core::UInt64, Core::WeakPointer<Core::Object3D>> meshToObjectMap;
    Core::WeakPointer<Core::Object3D> ambientLightObject;
    Core::WeakPointer<Core::Object3D> directionalLightObject;
    Core::WeakPointer<Core::RenderableContainer<Core::Mesh>>  pointLightObject;

    std::shared_ptr<CoreSync> coreSync;
    std::shared_ptr<GestureAdapter> gestureAdapter;
    std::shared_ptr<PipedEventAdapter<GestureAdapter::GestureEvent>> pipedGestureAdapter;
    std::shared_ptr<OrbitControls> orbitControls;

    Core::Color highlightColor;
    Core::Color outlineColor;
    Core::Color darkOutlineColor;
    Core::WeakPointer<Core::BasicColoredMaterial> highlightMaterial;
    Core::WeakPointer<Core::OutlineMaterial> outlineMaterial;

    QMutex onUpdateMutex;
    std::vector<ModelerAppLifecycleEventCallback> onUpdates;

    TransformWidget transformWidget;

};
