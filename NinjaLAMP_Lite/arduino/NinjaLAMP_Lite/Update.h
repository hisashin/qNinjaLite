#ifndef UPDATE_H_
#define UPDATE_H_
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <WiFiClientSecure.h>
#include "ESP8266httpUpdateCustom.h"
class NinjaUpdate: public ESP8266HTTPUpdateCustom {
public:
    NinjaUpdate(void);
    t_httpUpdate_return update(const String& url, const String& md5,
            const String& currentVersion = "");
protected:
    t_httpUpdate_return handleUpdate(HTTPClient& http,
            const String& currentVersion, bool spiffs = false);
    bool runUpdate(Stream& in, uint32_t size, String md5,
            int command = U_FLASH);
private:
    String expectedMD5;

};
#endif /* UPDATE_H_ */
