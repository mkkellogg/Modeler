#include <functional>

#include "Core/util/WeakPointer.h"

#include "GestureAdapter.h"


GestureAdapter::GestureAdapter() {
    mouseEventAdapter = std::make_shared<PipedEventAdapter<MouseAdapter::MouseEvent>>(std::bind(&GestureAdapter::onMouseEvent, this, std::placeholders::_1));
}

void GestureAdapter::setMouseAdapter(MouseAdapter& mouseAdapter) {
    mouseAdapter.setPipedEventAdapter(this->mouseEventAdapter);
}

bool GestureAdapter::setPipedEventAdapter(Core::WeakPointer<PipedEventAdapter<GestureEvent>> adapter) {
    if(!this->pipedEventAdapter) {
        this->pipedEventAdapter = adapter;
        return true;
    }
    else {
        return false;
    }
}

void GestureAdapter::onMouseEvent(MouseAdapter::MouseEvent event) {

    unsigned int pointerIndex = event.buttons;

    if (pointerIndex >= MAX_POINTERS) return;

    PointerState& pointerState = pointerStates[pointerIndex];
    switch(event.getType()){
        case MouseAdapter::MouseEventType::ButtonPress:
                pointerState.active = true;
                pointerState.startPosition = event.position;
                pointerState.position = event.position;
        break;
        case MouseAdapter::MouseEventType::ButtonRelease:
                pointerState.active = false;
                pointerState.position = event.position;
        break;
        case MouseAdapter::MouseEventType::MouseMove:
            if (pointerState.active) {
                GestureEvent gestureEvent(GestureEventType::Drag);
                gestureEvent.start = pointerState.position;
                gestureEvent.end = event.position;
                gestureEvent.pointer = (GesturePointer)pointerIndex;
                if (this->pipedEventAdapter) {
                    this->pipedEventAdapter->accept(gestureEvent);
                }
                pointerState.position = event.position;
            }
        break;
        case MouseAdapter::MouseEventType::WheelScroll:
            GestureEvent gestureEvent(GestureEventType::Scroll);
            gestureEvent.scrollDistance = event.scrollDelta;
            if (this->pipedEventAdapter) {
                this->pipedEventAdapter->accept(gestureEvent);
            }
        break;
    }
}
