#pragma once

#include <functional>

#include "Core/Engine.h"
#include "Core/scene/Object3D.h"
#include "Core/material/BasicTexturedMaterial.h"
#include "Core/material/BasicColoredMaterial.h"
#include "Core/material/Shader.h"
#include "Core/scene/RayCaster.h"

#include "CoreScene.h"
#include "CoreSync.h"
#include "MouseAdapter.h"
#include "GestureAdapter.h"
#include "PipedEventAdapter.h"
#include "OrbitControls.h"

class RenderWindow;
class MainContainer;

class ModelerApp {
public:

    ModelerApp();
    void init();
    void setRenderWindow(RenderWindow* renderWindow);
    void loadModel(const std::string& path, float scale, float smoothingThreshold, const bool zUp);
    CoreScene& getCoreScene();

private:
    void onEngineReady(Core::WeakPointer<Core::Engine> engine);
    void onGesture(GestureAdapter::GestureEvent event);
    void onMouseButtonAction(MouseAdapter::MouseEventType type, Core::UInt32 button, Core::UInt32 x, Core::UInt32 y);

    RenderWindow* renderWindow;
    bool engineReady = false;
    CoreScene coreScene;
    Core::WeakPointer<Core::Camera> renderCamera;
    Core::WeakPointer<Core::Engine> engine;
    Core::RayCaster rayCaster;
    std::unordered_map<Core::UInt64, Core::WeakPointer<Core::Object3D>> meshToObjectMap;
    Core::WeakPointer<Core::BasicColoredMaterial> highlightMaterial;
    std::shared_ptr<CoreSync> coreSync;
    std::shared_ptr<GestureAdapter> gestureAdapter;
    std::shared_ptr<PipedEventAdapter<GestureAdapter::GestureEvent>> pipedGestureAdapter;
    std::shared_ptr<OrbitControls> orbitControls;
};
