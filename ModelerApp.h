#pragma once

#include <functional>

#include "Core/Engine.h"
#include "Core/scene/Object3D.h"
#include "Core/material/BasicTexturedMaterial.h"
#include "Core/material/BasicColoredMaterial.h"
#include "Core/material/BasicExtrusionMaterial.h"
#include "Core/material/EquirectangularMaterial.h"
#include "Core/material/OutlineMaterial.h"
#include "Core/material/Shader.h"
#include "Core/scene/RayCaster.h"
#include "Core/color/Color.h"
#include "Core/render/ReflectionProbe.h"

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
    bool isSceneObjectHidden(Core::WeakPointer<Core::Object3D> object);
    void setSceneObjectHidden(Core::WeakPointer<Core::Object3D> object, bool hidden);

private:
    void engineReady(Core::WeakPointer<Core::Engine> engine);
    void setupHighlightMaterials();
    void gesture(GestureAdapter::GestureEvent event);
    void mouseButton(MouseAdapter::MouseEventType type, Core::UInt32 button, Core::Int32 x, Core::Int32 y);
    void setupRenderCamera();
    void loadScene();
    void setupSkyboxes();
    void setupDefaultObjects();
    void setupLights();
    void resolveOnUpdateCallbacks();
    void preRenderCallback();
    void postRenderCallback();
    void renderOnce(const std::vector<Core::WeakPointer<Core::Object3D>>& objects, Core::WeakPointer<Core::Camera> camera, Core::WeakPointer<Core::Material> material);

    RenderWindow* renderWindow;
    bool engineIsReady = false;
    Core::WeakPointer<Core::Engine> engine;

    CoreScene coreScene;
    Core::WeakPointer<Core::Camera> renderCamera;
    std::unordered_map<Core::UInt64, bool> hiddenSceneObjects;

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

    Core::WeakPointer<Core::ReflectionProbe> centerProbe;

    unsigned int frameCount = 0;
};
