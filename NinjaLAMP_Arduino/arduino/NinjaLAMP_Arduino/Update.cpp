#include "ESP8266HTTPUpdateCustom.h"
#include "Update.h"

NinjaUpdate::NinjaUpdate() :
        ESP8266HTTPUpdateCustom() {
}

HTTPUpdateResult NinjaUpdate::update(const String& url, const String& md5,
        const String& currentVersion) {
    DEBUG_HTTP_UPDATE("[NinjaLAMP] NinjaUpdate::update\n");
    expectedMD5 = md5;
    return ESP8266HTTPUpdateCustom::update(url, currentVersion);
}
/* Override */
HTTPUpdateResult NinjaUpdate::handleUpdate(HTTPClient& http,
        const String& currentVersion, bool spiffs) {
    DEBUG_HTTP_UPDATE("[NinjaLAMP] handleUpdate start\n");
    ESP8266HTTPUpdateCustom::handleUpdate(http, currentVersion, spiffs);
    DEBUG_HTTP_UPDATE("[NinjaLAMP] handleUpdate end\n");
}

/* Override */
bool NinjaUpdate::runUpdate(Stream& in, uint32_t size, String md5,
        int command) {

    // If you get compilation error, "virtual" is removed from ESP8266HTTPUpdate::runUpdate() and ESP8266HTTPUpdate::handleUpdate()
    ESP8266HTTPUpdateCustom::CheckNinjaPCR();
    DEBUG_HTTP_UPDATE("[NinjaLAMP] runUpdate start\n");
    bool result = ESP8266HTTPUpdateCustom::runUpdate(in, size, expectedMD5, command);
    DEBUG_HTTP_UPDATE("[NinjaLAMP] runUpdate end\n");
    return result;

}
