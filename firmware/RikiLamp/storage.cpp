#include "storage.h"

StorageManager storage;

void StorageManager::begin() {
    prefs.begin("riki", false);
}

bool StorageManager::hasWiFi() {
    return prefs.isKey("ssid");
}

void StorageManager::saveWiFi(const String& ssid, const String& password) {
    prefs.putString("ssid", ssid);
    prefs.putString("pass", password);
}

String StorageManager::getSSID() {
    return prefs.getString("ssid", "");
}

String StorageManager::getPassword() {
    return prefs.getString("pass", "");
}

void StorageManager::saveColour(LampColour colour) {
    prefs.putUChar("colour", static_cast<uint8_t>(colour));
}

LampColour StorageManager::getColour() {
    return lampColourFromIndex(prefs.getUChar("colour", static_cast<uint8_t>(LampColour::White)));
}

void StorageManager::saveEffect(LampEffect effect) {
    prefs.putUChar("effect", static_cast<uint8_t>(effect));
}

LampEffect StorageManager::getEffect() {
    return lampEffectFromIndex(prefs.getUChar("effect", static_cast<uint8_t>(LampEffect::Static)));
}

void StorageManager::savePower(bool on) {
    prefs.putBool("power", on);
}

bool StorageManager::getPower() {
    return prefs.getBool("power", true);
}

void StorageManager::saveBrightness(uint8_t brightness) {
    prefs.putUChar("brightness", brightness);
}

uint8_t StorageManager::getBrightness() {
    return prefs.getUChar("brightness", 255);
}

void StorageManager::clear() {
    prefs.clear();
}
