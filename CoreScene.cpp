#include "CoreScene.h"

#include "Core/render/Camera.h"

CoreScene::CoreScene() {

}

void CoreScene::setEngine(Core::WeakPointer<Core::Engine> engine) {
    this->engine = engine;
}

Core::WeakPointer<Core::Object3D> CoreScene::getSceneRoot() const {
    return this->sceneRoot;
}

void CoreScene::setSceneRoot(Core::WeakPointer<Core::Object3D> sceneRoot) {
    this->sceneRoot = sceneRoot;
}

void CoreScene::addObjectToScene(Core::WeakPointer<Core::Object3D> object) {
    this->addObjectToScene(object, this->sceneRoot);
}

void CoreScene::addObjectToScene(Core::WeakPointer<Core::Object3D> object, Core::WeakPointer<Core::Object3D> parent) {
    parent->addChild(object);
    for (auto& callback : this->sceneUpdatedCallbacks) {
        callback(object);
    }
}

void CoreScene::onSceneUpdated(SceneUpdatedCallback callback) {
    this->sceneUpdatedCallbacks.push_back(callback);
}

std::vector<Core::WeakPointer<Core::Object3D>>& CoreScene::getSelectedObjects() {
    return this->selectedObjects;
}

void CoreScene::addSelectedObject(Core::WeakPointer<Core::Object3D> newSelectedObject) {
    if (newSelectedObject.isValid() && !this->isObjectSelected(newSelectedObject)) {
        this->selectedObjects.push_back(newSelectedObject);
        for (OnObjectSelectedCallback callback : this->selectedObjectAddedCallbacks) {
            callback(newSelectedObject);
        }
    }
}

void CoreScene::removeSelectedObject(Core::WeakPointer<Core::Object3D> objectToRemove) {
    if (this->selectedObjects.size() > 0) {
        for (unsigned int i = 0; i < this->selectedObjects.size(); i++) {
            Core::WeakPointer<Core::Object3D> selectedObject = this->selectedObjects[i];
            if(selectedObject.get() == objectToRemove.get()) {
                this->removeSelectedObjectAtIndex(i);
                return;
            }
        }
    }
}

void CoreScene::removeSelectedObjectAtIndex(unsigned int index) {
    Core::WeakPointer<Core::Object3D> objectToRemove = this->selectedObjects[index];
    this->selectedObjects[index] = this->selectedObjects[this->selectedObjects.size() - 1];
    this->selectedObjects.pop_back();
    for (OnObjectSelectedCallback callback : this->selectedObjectRemovedCallbacks) {
        callback(objectToRemove);
    }
    return;
}

void CoreScene::clearSelectedObjects() {
    while (this->selectedObjects.size() > 0) {
        removeSelectedObjectAtIndex(0);
    }
}

void CoreScene::onSelectedObjectAdded(OnObjectSelectedCallback callback) {
    this->selectedObjectAddedCallbacks.push_back(callback);
}

void CoreScene::onSelectedObjectRemoved(OnObjectSelectedCallback callback) {
    this->selectedObjectRemovedCallbacks.push_back(callback);
}

bool CoreScene::isObjectSelected(Core::WeakPointer<Core::Object3D> candidateObject) {
    for (std::vector<Core::WeakPointer<Core::Object3D>>::iterator itr = this->selectedObjects.begin(); itr != this->selectedObjects.end(); ++itr) {
        Core::WeakPointer<Core::Object3D> selectedObject = *itr;
        if(selectedObject.get() == candidateObject.get()) return true;
    }
    return false;
}

void CoreScene::rayCastForObjectSelection(Core::WeakPointer<Core::Camera> camera, Core::Int32 x, Core::Int32 y, bool setSelectedObject,  bool multiSelect) {
    Core::WeakPointer<Core::Graphics> graphics = this->engine->getGraphicsSystem();
    Core::Ray ray = camera->getRay(x, y);
    std::vector<Core::Hit> hits;
    Core::Bool hitOccurred = this->sceneRaycaster.castRay(ray, hits);

    if (hitOccurred) {
        Core::Hit& hit = hits[0];
        Core::WeakPointer<Core::Mesh> hitObject = hit.Object;
        Core::WeakPointer<Core::Object3D> rootObject =this->meshToObjectMap[hitObject->getObjectID()];

        if (setSelectedObject) {
            if (multiSelect) {
                this->addSelectedObject(rootObject);
            }
            else {
                this->clearSelectedObjects();
                this->addSelectedObject(rootObject);
            }
        }
    }
}

void CoreScene::addObjectToSceneRaycaster(Core::WeakPointer<Core::Object3D> object, Core::WeakPointer<Core::Mesh> mesh) {
    this->sceneRaycaster.addObject(object, mesh);
    this->meshToObjectMap[mesh->getObjectID()] = object;
}
