#pragma once

#include <functional>
#include <vector>

#include <QMutex>

#include "Core/Engine.h"


// forward declarations
class RenderWindow;

class CoreSync final {
public:
    typedef std::function<void(Core::WeakPointer<Core::Engine>)> Runnable;

    CoreSync();
    ~CoreSync();
    void run(Runnable runnable);

private:

};

