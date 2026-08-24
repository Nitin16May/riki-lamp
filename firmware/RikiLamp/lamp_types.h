#pragma once

#include <Arduino.h>
#include <FastLED.h>

enum class LampColour : uint8_t {
    White = 0,
    WarmWhite,
    Red,
    Green,
    Blue,
    Yellow,
    Orange,
    Pink,
    Purple,
    Cyan,
    Magenta,
    Lime,
    Teal,
    SkyBlue,
    Violet,
    Gold,
    Count
};

enum class LampEffect : uint8_t {
    Static = 0,
    Breathing,
    Heartbeat,
    Rainbow,
    Fire,
    Candle,
    Aurora,
    Ocean,
    Galaxy,
    Meteor,
    Police,
    Lightning,
    Sunrise,
    Twinkle,
    Wave,
    Pulse,
    Count
};

const char* lampColourName(LampColour colour);
const char* lampEffectName(LampEffect effect);
CRGB lampColourRgb(LampColour colour);

bool parseLampColour(const char* name, LampColour& out);
bool parseLampEffect(const char* name, LampEffect& out);

LampColour nextLampColour(LampColour colour);
LampEffect nextLampEffect(LampEffect effect);

LampColour lampColourFromIndex(uint8_t index);
LampEffect lampEffectFromIndex(uint8_t index);
