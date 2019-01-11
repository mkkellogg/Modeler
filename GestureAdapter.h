#pragma once

#include <memory>

#include "MouseAdapter.h"
#include "PipedEventAdapter.h"

class GestureAdapter {
public:

    enum class GestureEventType {
        Move = 0,
        Drag = 1,
        Scroll = 2,
    };

    enum class GesturePointer {
        Primary = 1,
        Secondary = 2,
        Tertiary = 4,
        PrimaryDouble = 3,
    };

    class GestureEvent {
    public:
        GestureEvent() {}
        GestureEvent(GestureEventType type): type(type) {}
        GestureEventType getType() {return  type;}
        void setType(GestureEventType type) {this->type = type;}
        GesturePointer pointer;
        Core::Vector2i start;
        Core::Vector2i end;
        Core::Real scrollDistance;
    private:
        GestureEventType type;
    };

    GestureAdapter();

    void setMouseAdapter(MouseAdapter& mouseAdapter);
    bool setPipedEventAdapter(Core::WeakPointer<PipedEventAdapter<GestureEvent>> adapter);

private:
    static const unsigned int MAX_POINTERS = 5;

    class PointerState {
    public:
        bool active = false;
        Core::Vector2i startPosition;
        Core::Vector2i position;
    };

    void onMouseEvent(MouseAdapter::MouseEvent event);

    std::shared_ptr<PipedEventAdapter<MouseAdapter::MouseEvent>> mouseEventAdapter;
    PointerState pointerStates[MAX_POINTERS];

    Core::WeakPointer<PipedEventAdapter<GestureEvent>> pipedEventAdapter;
};

