#pragma once

#include <vector>
#include <functional>

#include "Core/Engine.h"
#include "Core/scene/Object3D.h"

class CoreScene {
public:
    using SceneUpdatedCallback = std::function<void(Core::WeakPointer<Core::Object3D>)>;
    using OnObjectSelectedCallback = std::function<void(Core::WeakPointer<Core::Object3D>)>;

    CoreScene();
    CoreScene(Core::WeakPointer<Core::Object3D> sceneRoot);
    Core::WeakPointer<Core::Object3D> getSceneRoot() const;
    void setSceneRoot(Core::WeakPointer<Core::Object3D> sceneRoot);
    void addObjectToScene(Core::WeakPointer<Core::Object3D> object);
    void addObjectToScene(Core::WeakPointer<Core::Object3D> object, Core::WeakPointer<Core::Object3D> parent);
    void onSceneUpdated(SceneUpdatedCallback callback);
    std::vector<Core::WeakPointer<Core::Object3D>>& getSelectedObjects();
    void addSelectedObject(Core::WeakPointer<Core::Object3D> newSelectedObject);
    void removeSelectedObject(Core::WeakPointer<Core::Object3D> objectToRemove);
    void onSelectedObjectAdded(OnObjectSelectedCallback callback);
    void onSelectedObjectRemoved(OnObjectSelectedCallback callback);
    bool isObjectSelected(Core::WeakPointer<Core::Object3D> candidateObject);

private:


    Core::WeakPointer<Core::Object3D> sceneRoot;
    std::vector<SceneUpdatedCallback> sceneUpdatedCallbacks;
    std::vector<Core::WeakPointer<Core::Object3D>> selectedObjects;
    std::vector<OnObjectSelectedCallback> selectedObjectAddedCallbacks;
    std::vector<OnObjectSelectedCallback> selectedObjectRemovedCallbacks;
};
