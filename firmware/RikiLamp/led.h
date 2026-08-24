#pragma once

#include "config.h"
#include "lamp_types.h"

class LEDManager {
public:
    /// Initialise LEDs and restore saved colour, effect, brightness, and power state.
    void begin();

    /// Render the active effect. Call every loop iteration.
    void loop();

    /// Set a solid colour instantly and switch to the Static effect.
    void setSolidColour(LampColour colour);

    /// Activate a lighting effect instantly.
    void setEffect(LampEffect effect);

    /// Advance to the next colour in the cycle.
    void nextColour();

    /// Advance to the next effect in the cycle.
    void nextEffect();

    /// Advance through the local 10%, 20%, 30%, ..., and 100% brightness levels.
    void nextBrightness();

    /// Turn lamp output on while preserving colour and effect.
    void lampOn();

    /// Turn lamp output off while preserving colour and effect.
    void lampOff();

    bool isOn() const;
    uint8_t brightness() const;
    LampColour currentColour() const;
    LampEffect currentEffect() const;

    /// Apply persisted preferences without publishing or saving again.
    void restoreFromStorage();

private:
    void showInstant(const CRGB& colour);
    void fillInstant(const CRGB& colour);
    void renderFrame(unsigned long now);
    void markDirty();

    void renderStatic(unsigned long now);
    void renderBreathing(unsigned long now);
    void renderHeartbeat(unsigned long now);
    void renderRainbow(unsigned long now);
    void renderFire(unsigned long now);
    void renderCandle(unsigned long now);
    void renderAurora(unsigned long now);
    void renderOcean(unsigned long now);
    void renderGalaxy(unsigned long now);
    void renderMeteor(unsigned long now);
    void renderPolice(unsigned long now);
    void renderLightning(unsigned long now);
    void renderSunrise(unsigned long now);
    void renderTwinkle(unsigned long now);
    void renderWave(unsigned long now);
    void renderPulse(unsigned long now);

    using EffectRenderer = void (LEDManager::*)(unsigned long);

    static EffectRenderer effectRenderer(LampEffect effect);

    CRGB _leds[LED_COUNT];

    LampColour _colour = LampColour::White;
    LampEffect _effect = LampEffect::Static;
    uint8_t _brightness = 255;
    bool _powerOn = true;
    bool _dirty = true;

    unsigned long _lastFrameMs = 0;
    unsigned long _effectStartMs = 0;
    uint8_t _noiseSeed = 42;
};

extern LEDManager led;
