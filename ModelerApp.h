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

class RenderWindow;
class MainGUI;

class ModelerApp {
public:

    using ModelerAppLifecycleEventCallback = std::function<void()>;

    ModelerApp();
    void init();
    void setRenderWindow(RenderWindow* renderWindow);
    void loadModel(const std::string& path, float scale, float smoothingThreshold, const bool zUp);
    CoreScene& getCoreScene();
    void onUpdate(ModelerAppLifecycleEventCallback callback);
    std::shared_ptr<CoreSync> getCoreSync();

private:
    void engineReady(Core::WeakPointer<Core::Engine> engine);
    void gesture(GestureAdapter::GestureEvent event);
    void mouseButton(MouseAdapter::MouseEventType type, Core::UInt32 button, Core::UInt32 x, Core::UInt32 y);
    void rayCastForObjectSelection(Core::UInt32 x, Core::UInt32 y, bool setSelectedObject = true);

    void resolveOnUpdateCallbacks();

    RenderWindow* renderWindow;
    bool engineIsReady = false;
    CoreScene coreScene;
    Core::WeakPointer<Core::Camera> renderCamera;
    Core::WeakPointer<Core::Engine> engine;
    Core::RayCaster sceneRaycaster;
    Core::RayCaster transformationWidgetRaycaster;
    std::unordered_map<Core::UInt64, Core::WeakPointer<Core::Object3D>> meshToObjectMap;
    Core::WeakPointer<Core::BasicColoredMaterial> highlightMaterial;
    Core::WeakPointer<Core::OutlineMaterial> outlineMaterial;
    std::shared_ptr<CoreSync> coreSync;
    std::shared_ptr<GestureAdapter> gestureAdapter;
    std::shared_ptr<PipedEventAdapter<GestureAdapter::GestureEvent>> pipedGestureAdapter;
    std::shared_ptr<OrbitControls> orbitControls;

    Core::Color highlightColor;
    Core::Color outlineColor;
    Core::Color darkOutlineColor;

    QMutex onUpdateMutex;
    std::vector<ModelerAppLifecycleEventCallback> onUpdates;
};
