#include "led.h"

#include "storage.h"

#include <FastLED.h>

LEDManager led;

namespace {

CRGB scaleColour(const CRGB& colour, uint8_t brightness) {
    CRGB scaled = colour;
    scaled.nscale8(brightness);
    return scaled;
}

} // namespace

void LEDManager::begin() {
    FastLED.addLeds<LED_TYPE, LED_PIN, LED_COLOR_ORDER>(_leds, LED_COUNT);
    FastLED.setBrightness(255);
    FastLED.clear(true);

    restoreFromStorage();
    _effectStartMs = millis();
    _dirty = true;
}

void LEDManager::restoreFromStorage() {
    _colour = storage.getColour();
    _effect = storage.getEffect();
    _brightness = storage.getBrightness();
    _powerOn = storage.getPower();
    FastLED.setBrightness(_brightness);
    _dirty = true;
}

void LEDManager::loop() {
    const unsigned long now = millis();
    if (now - _lastFrameMs < LED_FPS_MS && !_dirty) {
        return;
    }

    _lastFrameMs = now;
    _dirty = false;

    if (!_powerOn) {
        fillInstant(CRGB::Black);
        FastLED.show();
        return;
    }

    renderFrame(now);
    FastLED.show();
}

void LEDManager::markDirty() {
    _dirty = true;
}

void LEDManager::fillInstant(const CRGB& colour) {
    fill_solid(_leds, LED_COUNT, colour);
}

void LEDManager::showInstant(const CRGB& colour) {
    fillInstant(colour);
    FastLED.show();
}

void LEDManager::setSolidColour(LampColour colour) {
    if (_colour == colour && _effect == LampEffect::Static && _powerOn) {
        return;
    }
    _powerOn = true;
    _colour = colour;
    _effect = LampEffect::Static;
    _effectStartMs = millis();
    storage.savePower(true);
    storage.saveColour(_colour);
    storage.saveEffect(_effect);
    markDirty();
}

void LEDManager::setEffect(LampEffect effect) {
    if (_effect == effect && _powerOn) {
        return;
    }
    _powerOn = true;
    _effect = effect;
    _effectStartMs = millis();
    storage.savePower(true);
    storage.saveEffect(_effect);
    markDirty();
}

void LEDManager::nextColour() {
    setSolidColour(nextLampColour(_colour));
}

void LEDManager::nextEffect() {
    setEffect(nextLampEffect(_effect));
}

void LEDManager::nextBrightness() {
    constexpr uint8_t kBrightnessLevels[] = {26, 51, 77, 102, 128, 153, 179, 204, 230, 255};

    uint8_t next = kBrightnessLevels[0];
    for (uint8_t level : kBrightnessLevels) {
        if (_brightness < level) {
            next = level;
            break;
        }
    }

    _brightness = next;
    FastLED.setBrightness(_brightness);
    storage.saveBrightness(_brightness);
    markDirty();
}

void LEDManager::setBrightness(uint8_t value) {
    if (value < 5) value = 5;
    _brightness = value;
    FastLED.setBrightness(_brightness);
    storage.saveBrightness(_brightness);
    markDirty();
}

void LEDManager::lampOn() {
    _powerOn = true;
    storage.savePower(true);
    markDirty();
}

void LEDManager::lampOff() {
    _powerOn = false;
    storage.savePower(false);
    markDirty();
}

bool LEDManager::isOn() const {
    return _powerOn;
}

uint8_t LEDManager::brightness() const {
    return _brightness;
}

LampColour LEDManager::currentColour() const {
    return _colour;
}

LampEffect LEDManager::currentEffect() const {
    return _effect;
}

LEDManager::EffectRenderer LEDManager::effectRenderer(LampEffect effect) {
    static const EffectRenderer kRenderers[] = {
        &LEDManager::renderStatic,
        &LEDManager::renderBreathing,
        &LEDManager::renderHeartbeat,
        &LEDManager::renderRainbow,
        &LEDManager::renderFire,
        &LEDManager::renderCandle,
        &LEDManager::renderAurora,
        &LEDManager::renderOcean,
        &LEDManager::renderGalaxy,
        &LEDManager::renderMeteor,
        &LEDManager::renderPolice,
        &LEDManager::renderLightning,
        &LEDManager::renderSunrise,
        &LEDManager::renderTwinkle,
        &LEDManager::renderWave,
        &LEDManager::renderPulse,
    };

    const uint8_t index = static_cast<uint8_t>(effect);
    if (index >= static_cast<uint8_t>(LampEffect::Count)) {
        return &LEDManager::renderStatic;
    }
    return kRenderers[index];
}

void LEDManager::renderFrame(unsigned long now) {
    (this->*effectRenderer(_effect))(now);
}

void LEDManager::renderStatic(unsigned long now) {
    (void)now;
    fillInstant(lampColourRgb(_colour));
}

void LEDManager::renderBreathing(unsigned long now) {
    const uint8_t brightness = beatsin8(12, 30, 255, now - _effectStartMs, 0);
    fillInstant(scaleColour(lampColourRgb(_colour), brightness));
}

void LEDManager::renderHeartbeat(unsigned long now) {
    const unsigned long t = now - _effectStartMs;
    const unsigned long phase = t % 1200;
    uint8_t brightness = 20;

    if (phase < 120 || (phase >= 220 && phase < 320)) {
        brightness = 255;
    } else if (phase < 220) {
        brightness = 80;
    }

    fillInstant(scaleColour(lampColourRgb(_colour), brightness));
}

void LEDManager::renderRainbow(unsigned long now) {
    const uint8_t hue = (now - _effectStartMs) / 8;
    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        _leds[i] = CHSV(hue + (i * 12), 255, 255);
    }
}

void LEDManager::renderFire(unsigned long now) {
    (void)now;
    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        _noiseSeed = _noiseSeed * 2053 + 13849;
        const uint8_t heat = 96 + (_noiseSeed & 159);
        _leds[i] = HeatColor(heat);
    }
}

void LEDManager::renderCandle(unsigned long now) {
    const uint8_t flicker = beatsin8(24, 140, 255, now - _effectStartMs, 0);
    fillInstant(scaleColour(CRGB(255, 120, 20), flicker));
}

void LEDManager::renderAurora(unsigned long now) {
    const unsigned long t = now - _effectStartMs;
    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        const uint8_t hue = 96 + beatsin8(6, 0, 48, t, i * 40);
        const uint8_t brightness = beatsin8(8, 80, 255, t, i * 25);
        _leds[i] = scaleColour(CHSV(hue, 255, 255), brightness);
    }
}

void LEDManager::renderOcean(unsigned long now) {
    const unsigned long t = now - _effectStartMs;
    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        const uint8_t wave = beatsin8(4, 60, 255, t, i * 50);
        _leds[i] = scaleColour(CRGB(0, 80, 220), wave);
    }
}

void LEDManager::renderGalaxy(unsigned long now) {
    fillInstant(CRGB(8, 0, 24));
    const unsigned long t = now - _effectStartMs;
    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        if (((t / 120) + i) % 3 == 0) {
            const uint8_t sparkle = beatsin8(20, 40, 255, t, i * 90);
            _leds[i] = scaleColour(CRGB(180, 120, 255), sparkle);
        }
    }
}

void LEDManager::renderMeteor(unsigned long now) {
    fillInstant(CRGB(0, 0, 30));
    const uint16_t head = ((now - _effectStartMs) / 60) % (LED_COUNT + 4);
    if (head < LED_COUNT) {
        _leds[head] = CRGB::White;
    }
    if (head > 0 && head - 1 < LED_COUNT) {
        _leds[head - 1] = CRGB(80, 80, 255);
    }
}

void LEDManager::renderPolice(unsigned long now) {
    const bool redPhase = ((now - _effectStartMs) / 250) % 2 == 0;
    fillInstant(redPhase ? CRGB(255, 0, 0) : CRGB(0, 0, 255));
}

void LEDManager::renderLightning(unsigned long now) {
    fillInstant(CRGB(5, 5, 20));
    const unsigned long t = now - _effectStartMs;
    if ((t % 1800) < 60 || (t % 1800) > 120 && (t % 1800) < 150) {
        fillInstant(CRGB(240, 240, 255));
    }
}

void LEDManager::renderSunrise(unsigned long now) {
    const unsigned long t = (now - _effectStartMs) % 6000;
    const uint8_t progress = map(t, 0, 5999, 0, 255);
    const CRGB start(20, 10, 0);
    const CRGB end(255, 120, 30);
    fillInstant(blend(start, end, progress));
}

void LEDManager::renderTwinkle(unsigned long now) {
    const CRGB base = lampColourRgb(_colour);
    fillInstant(scaleColour(base, 40));
    const unsigned long t = now - _effectStartMs;
    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        if (((t / 100) + i) % 4 == 0) {
            _leds[i] = scaleColour(base, beatsin8(16, 120, 255, t, i * 70));
        }
    }
}

void LEDManager::renderWave(unsigned long now) {
    const CRGB base = lampColourRgb(_colour);
    const unsigned long t = now - _effectStartMs;
    for (uint16_t i = 0; i < LED_COUNT; ++i) {
        const uint8_t brightness = beatsin8(5, 40, 255, t, i * 35);
        _leds[i] = scaleColour(base, brightness);
    }
}

void LEDManager::renderPulse(unsigned long now) {
    const uint8_t brightness = beatsin8(8, 0, 255, now - _effectStartMs, 0);
    fillInstant(scaleColour(lampColourRgb(_colour), brightness));
}
