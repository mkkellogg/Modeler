#include "KeyboardAdapter.h"

#include <iostream>

#include <QGuiApplication>

KeyboardAdapter::KeyboardAdapter() {

}

bool KeyboardAdapter::isModifierActive(Modifier modifier) {
    switch (modifier) {
        case Modifier::Ctrl:
            return (QGuiApplication::queryKeyboardModifiers() & Qt::ControlModifier) != 0;
        break;
    }
    return false;
}
