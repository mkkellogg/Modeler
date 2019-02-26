#pragma once

class KeyboardAdapter
{
public:

    enum class Modifier {
        Shift = 0,
        Ctrl = 1,
        Alt = 2,
    };

    KeyboardAdapter();

    static bool isModifierActive(Modifier modifier);
};
