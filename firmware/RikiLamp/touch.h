#pragma once

#include <cstdint>

enum class TouchGesture {
    SingleTap,
    DoubleTap,
    TripleTap,
    LongPress,
    VeryLongPress
};

class TouchManager {
public:
    using GestureCallback = void (*)(TouchGesture gesture);

    /// Configure the redundant touch inputs and debounce state.
    void begin();

    /// Poll touch input and dispatch gestures.
    void loop();

    /// Register a callback for recognised gestures.
    void onGesture(GestureCallback callback);

private:
    /// Return true when any configured touch input is active.
    bool readTouch() const;
    void resetTapCounter();
    void finalizeTapGesture();
    void dispatch(TouchGesture gesture);

    GestureCallback _callback = nullptr;

    bool _stablePressed = false;
    bool _lastReading = false;
    unsigned long _lastChangeMs = 0;

    bool _pressActive = false;
    unsigned long _pressStartMs = 0;
    bool _longPressSent = false;
    bool _factoryResetSent = false;

    uint8_t _tapCount = 0;
    unsigned long _lastTapMs = 0;
};

extern TouchManager touch;
