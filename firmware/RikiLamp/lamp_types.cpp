#include "lamp_types.h"

#include <cstring>

namespace {

struct NamedColour {
    LampColour id;
    const char* name;
    CRGB rgb;
};

struct NamedEffect {
    LampEffect id;
    const char* name;
};

constexpr NamedColour kColours[] = {
    {LampColour::White, "WHITE", CRGB(255, 255, 255)},
    {LampColour::WarmWhite, "WARM_WHITE", CRGB(255, 197, 143)},
    {LampColour::Red, "RED", CRGB(255, 0, 0)},
    {LampColour::Green, "GREEN", CRGB(0, 255, 0)},
    {LampColour::Blue, "BLUE", CRGB(0, 0, 255)},
    {LampColour::Yellow, "YELLOW", CRGB(255, 255, 0)},
    {LampColour::Orange, "ORANGE", CRGB(255, 120, 0)},
    {LampColour::Pink, "PINK", CRGB(255, 105, 180)},
    {LampColour::Purple, "PURPLE", CRGB(128, 0, 128)},
    {LampColour::Cyan, "CYAN", CRGB(0, 255, 255)},
    {LampColour::Magenta, "MAGENTA", CRGB(255, 0, 255)},
    {LampColour::Lime, "LIME", CRGB(180, 255, 0)},
    {LampColour::Teal, "TEAL", CRGB(0, 128, 128)},
    {LampColour::SkyBlue, "SKY_BLUE", CRGB(135, 206, 235)},
    {LampColour::Violet, "VIOLET", CRGB(148, 0, 211)},
    {LampColour::Gold, "GOLD", CRGB(255, 215, 0)},
};

constexpr NamedEffect kEffects[] = {
    {LampEffect::Static, "STATIC"},
    {LampEffect::Breathing, "BREATHING"},
    {LampEffect::Heartbeat, "HEARTBEAT"},
    {LampEffect::Rainbow, "RAINBOW"},
    {LampEffect::Fire, "FIRE"},
    {LampEffect::Candle, "CANDLE"},
    {LampEffect::Aurora, "AURORA"},
    {LampEffect::Ocean, "OCEAN"},
    {LampEffect::Galaxy, "GALAXY"},
    {LampEffect::Meteor, "METEOR"},
    {LampEffect::Police, "POLICE"},
    {LampEffect::Lightning, "LIGHTNING"},
    {LampEffect::Sunrise, "SUNRISE"},
    {LampEffect::Twinkle, "TWINKLE"},
    {LampEffect::Wave, "WAVE"},
    {LampEffect::Pulse, "PULSE"},
};

bool equalsIgnoreCase(const char* a, const char* b) {
    if (!a || !b) {
        return false;
    }

    while (*a && *b) {
        if (tolower(static_cast<unsigned char>(*a)) != tolower(static_cast<unsigned char>(*b))) {
            return false;
        }
        ++a;
        ++b;
    }

    return *a == '\0' && *b == '\0';
}

} // namespace

const char* lampColourName(LampColour colour) {
    const uint8_t index = static_cast<uint8_t>(colour);
    if (index >= static_cast<uint8_t>(LampColour::Count)) {
        return "WHITE";
    }
    return kColours[index].name;
}

const char* lampEffectName(LampEffect effect) {
    const uint8_t index = static_cast<uint8_t>(effect);
    if (index >= static_cast<uint8_t>(LampEffect::Count)) {
        return "STATIC";
    }
    return kEffects[index].name;
}

CRGB lampColourRgb(LampColour colour) {
    const uint8_t index = static_cast<uint8_t>(colour);
    if (index >= static_cast<uint8_t>(LampColour::Count)) {
        return CRGB::White;
    }
    return kColours[index].rgb;
}

bool parseLampColour(const char* name, LampColour& out) {
    for (const auto& colour : kColours) {
        if (equalsIgnoreCase(name, colour.name)) {
            out = colour.id;
            return true;
        }
    }
    return false;
}

bool parseLampEffect(const char* name, LampEffect& out) {
    for (const auto& effect : kEffects) {
        if (equalsIgnoreCase(name, effect.name)) {
            out = effect.id;
            return true;
        }
    }
    return false;
}

LampColour nextLampColour(LampColour colour) {
    const uint8_t next = (static_cast<uint8_t>(colour) + 1) % static_cast<uint8_t>(LampColour::Count);
    return static_cast<LampColour>(next);
}

LampEffect nextLampEffect(LampEffect effect) {
    const uint8_t next = (static_cast<uint8_t>(effect) + 1) % static_cast<uint8_t>(LampEffect::Count);
    return static_cast<LampEffect>(next);
}

LampColour lampColourFromIndex(uint8_t index) {
    if (index >= static_cast<uint8_t>(LampColour::Count)) {
        return LampColour::White;
    }
    return static_cast<LampColour>(index);
}

LampEffect lampEffectFromIndex(uint8_t index) {
    if (index >= static_cast<uint8_t>(LampEffect::Count)) {
        return LampEffect::Static;
    }
    return static_cast<LampEffect>(index);
}
