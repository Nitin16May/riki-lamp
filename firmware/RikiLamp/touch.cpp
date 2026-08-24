#include "touch.h"

#include "config.h"

#include <Arduino.h>

TouchManager touch;

namespace {

constexpr uint8_t kTouchPins[] = {
    TOUCH_PIN_1,
    TOUCH_PIN_2,
    TOUCH_PIN_3,
    TOUCH_PIN_4,
};

} // namespace

bool TouchManager::readTouch() const {
    for (const uint8_t pin : kTouchPins) {
        if (digitalRead(pin) == TOUCH_ACTIVE) {
            return true;
        }
    }

    return false;
}

void TouchManager::begin() {
    for (const uint8_t pin : kTouchPins) {
        pinMode(pin, INPUT);
    }

    _stablePressed = readTouch();
    _lastReading = _stablePressed;
    _lastChangeMs = millis();
}

void TouchManager::onGesture(GestureCallback callback) {
    _callback = callback;
}

void TouchManager::dispatch(TouchGesture gesture) {
    if (_callback) {
        _callback(gesture);
    }
}

void TouchManager::resetTapCounter() {
    _tapCount = 0;
    _lastTapMs = 0;
}

void TouchManager::finalizeTapGesture() {
    if (_tapCount == 0) {
        return;
    }

    if (_tapCount == 1) {
        dispatch(TouchGesture::SingleTap);
    } else if (_tapCount == 2) {
        dispatch(TouchGesture::DoubleTap);
    } else {
        dispatch(TouchGesture::TripleTap);
    }

    resetTapCounter();
}

void TouchManager::loop() {
    const unsigned long now = millis();
    const bool reading = readTouch();

    if (reading != _lastReading) {
        _lastReading = reading;
        _lastChangeMs = now;
    }

    if (reading != _stablePressed && now - _lastChangeMs >= TOUCH_DEBOUNCE_MS) {
        _stablePressed = reading;

        if (_stablePressed) {
            _pressActive = true;
            _pressStartMs = now;
            _longPressSent = false;
            _factoryResetSent = false;
        } else {
            _pressActive = false;

            if (!_longPressSent && !_factoryResetSent) {
                if (_tapCount < 3) {
                    ++_tapCount;
                }
                _lastTapMs = now;
            }
        }
    }

    if (_pressActive) {
        const unsigned long held = now - _pressStartMs;

        if (!_longPressSent && held >= TOUCH_LONG_PRESS_MS) {
            _longPressSent = true;
            resetTapCounter();
            dispatch(TouchGesture::LongPress);
        }

        if (!_factoryResetSent && held >= TOUCH_FACTORY_RESET_MS) {
            _factoryResetSent = true;
            resetTapCounter();
            dispatch(TouchGesture::VeryLongPress);
        }
    }

    if (_tapCount > 0 && now - _lastTapMs >= TOUCH_MULTI_TAP_TIMEOUT_MS) {
        finalizeTapGesture();
    }
}
