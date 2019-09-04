#pragma once

#include <vector>
#include <functional>
#include <unordered_map>

#include "Core/Engine.h"
#include "Core/scene/Object3D.h"
#include "Core/scene/RayCaster.h"
#include "Core/geometry/Mesh.h"

class CoreScene {
public:
    using SceneUpdatedCallback = std::function<void(Core::WeakPointer<Core::Object3D>)>;
    using OnObjectSelectedCallback = std::function<void(Core::WeakPointer<Core::Object3D>)>;

    CoreScene();
    void setEngine(Core::WeakPointer<Core::Engine> engine);
    Core::WeakPointer<Core::Object3D> getSceneRoot() const;
    void setSceneRoot(Core::WeakPointer<Core::Object3D> sceneRoot);
    void addObjectToScene(Core::WeakPointer<Core::Object3D> object);
    void addObjectToScene(Core::WeakPointer<Core::Object3D> object, Core::WeakPointer<Core::Object3D> parent);
    void onSceneUpdated(SceneUpdatedCallback callback);
    std::vector<Core::WeakPointer<Core::Object3D>>& getSelectedObjects();
    void addSelectedObject(Core::WeakPointer<Core::Object3D> newSelectedObject);
    void removeSelectedObject(Core::WeakPointer<Core::Object3D> objectToRemove);
    void clearSelectedObjects();
    void onSelectedObjectAdded(OnObjectSelectedCallback callback);
    void onSelectedObjectRemoved(OnObjectSelectedCallback callback);
    bool isObjectSelected(Core::WeakPointer<Core::Object3D> candidateObject);
    void addObjectToSceneRaycaster(Core::WeakPointer<Core::Object3D> object, Core::WeakPointer<Core::Mesh> mesh);
    void rayCastForObjectSelection(Core::WeakPointer<Core::Camera> camera, Core::Int32 x, Core::Int32 y, bool setSelectedObject, bool multiSelect);

private:
    void removeSelectedObjectAtIndex(unsigned int index);

    Core::WeakPointer<Core::Engine> engine;
    Core::RayCaster sceneRaycaster;
    std::unordered_map<Core::UInt64, Core::WeakPointer<Core::Object3D>> meshToObjectMap;
    Core::WeakPointer<Core::Object3D> sceneRoot;
    std::vector<SceneUpdatedCallback> sceneUpdatedCallbacks;
    std::vector<Core::WeakPointer<Core::Object3D>> selectedObjects;
    std::vector<OnObjectSelectedCallback> selectedObjectAddedCallbacks;
    std::vector<OnObjectSelectedCallback> selectedObjectRemovedCallbacks;
};
