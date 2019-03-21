#include "ModelerScene.h"

ModelerScene::ModelerScene(ModelerApp& modelerApp): modelerApp(modelerApp), sceneHelper(modelerApp){
    this->loadComplete = false;
}

void ModelerScene::load() {
    this->loadComplete = true;
}
