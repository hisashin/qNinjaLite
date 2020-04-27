/* Init wifi */

// #include "wifi_conf.h"
// #include "wifi_communicator.h"
// #include "thermocycler.h"
#include "ninjalamp_includes.h"
#include <EEPROM.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFiMulti.h>

/* For displaying previous errors */
#define WIFI_RESULT_CONNECTION_SUCCESS 0x01
#define WIFI_RESULT_CONNECTION_FAILURE 0x02
#define WIFI_RESULT_CONF_DONE 0x03
#define WIFI_RESULT_CONF_ERROR 0x04

ESP8266WiFiMulti wifiMulti;
bool isUpdateMode = false;
String DEFAULT_HOST_NAME = "ninjapcr";

char ssid[EEPROM_WIFI_SSID_MAX_LENGTH + 1];
char password[EEPROM_WIFI_PASSWORD_MAX_LENGTH + 1];
char host[EEPROM_WIFI_MDNS_HOST_MAX_LENGTH + 1];
// flag 1byte + 4bytes
ESP8266WebServer server(80);

/* t_network_receive_interface */
void wifi_receive() {
    server.handleClient();
}
/* t_network_send_interface */
void wifi_send(char *response, char *funcName) {
    String str = "DeviceResponse.";
    str += String(funcName);
    str += "(";
    str += String(response);
    str += ",\"";
    str += String(server.arg("x"));
    str += "\");";
    char *buff = (char*)malloc(sizeof(char) * (str.length()+1));
    str.toCharArray(buff, str.length()+1);
    server.send(200, "text/plain", buff);
    free(buff);
}

/* HTTP request handlers */
/* Handle request to "/" */
void requestHandlerTop() {
    server.send(200, "text/plain", "NinjaLAMP is alive!");
}
/* Handle request to "/command" */
void requestHandlerCommand() {
  /*
    wifi->ResetCommand();
    wifi->SendCommand();
    char buff[256];
    for (uint8_t i = 0; i < server.args(); i++) {
        String sKey = server.argName(i);
        String sValue = server.arg(i);
        char *key = (char *) malloc(sKey.length() + 1);
        char *value = (char *) malloc(sValue.length() + 1);
        sKey.toCharArray(key, sKey.length() + 1);
        sValue.toCharArray(value, sValue.length() + 1);
        wifi->AddCommandParam(key, value, buff); //TODO
        free(key);
        free(value);
    }
    */
}

void sendAcceptedMessage (bool isAccepted, char * commandName) {
  if (isAccepted) {
    wifi_send("true", commandName);
  } else {
    wifi_send("false", commandName);
  }
}
/* Handle request to "/pause" */
void requestHandlerStop() {
  sendAcceptedMessage(true,"pause");
}
/* Handle request to "/pause" */
void requestHandlerStart() {
  sendAcceptedMessage(true,"resume");
}
/* Handle request to "/status" */
int statusCount = 0;

void requestHandlerStatus() {
  /*
    wifi->ResetCommand();
    wifi->RequestStatus();
    */
}

/* Handle request to "/connect" */
void requestHandlerConnect() {
    String response = "{connected:true,version:\"";
    response += String(NINJALAMP_FIRMWARE_VERSION_STRING);
    response += "\"}";
    char *chResponse = (char *) malloc(sizeof(char) * (response.length()+1));
    response.toCharArray(chResponse, response.length()+1);
    wifi_send(chResponse, "connect");
    free(chResponse);
}

/* Handle 404 not found error */
void requestHandler404() {
    server.send(404, "text/plain", "404");
}

/* AP scanning (to show AP list) */
// Find nearby SSIDs
int apCount = 0;
const int AP_MAX_NUMBER = 54;
String SSIDs[AP_MAX_NUMBER];
void scanNearbyAP() {
    // WiFi.scanNetworks will return the number of networks found
    int n = WiFi.scanNetworks();
    if (n == 0) {
        PCR_NETWORK_DEBUG_LINE("no networks found");
    } else {
        PCR_NETWORK_DEBUG(n);
        PCR_NETWORK_DEBUG_LINE(" found");
        if (n > AP_MAX_NUMBER) {
            n = AP_MAX_NUMBER;
        }
        apCount = 0;
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            String ssid = WiFi.SSID(i);
            for (int j = 0; j < apCount; j++) {
                if (ssid == SSIDs[apCount]) {
                    PCR_NETWORK_DEBUG_LINE("Duplicate");
                    break;
                }
            }
            PCR_NETWORK_DEBUG(apCount + 1);
            PCR_NETWORK_DEBUG(": ");
            PCR_NETWORK_DEBUG(ssid);
            PCR_NETWORK_DEBUG(" (");
            PCR_NETWORK_DEBUG(WiFi.RSSI(i));
            PCR_NETWORK_DEBUG(")\n");
            SSIDs[apCount] = ssid;
            delay(1);
            apCount++;
            if (apCount >= AP_MAX_NUMBER) {
                break;
            }
        }
    }
    PCR_NETWORK_DEBUG_LINE("Scan done.");
}
void startScanning() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(10);
}
const IPAddress apIP(192, 168, 1, 1); // http://192.168.1.1
void stopScanMode() {
}

// NinjaPCR works as an AP with this SSID
const char* apSSID = "NinjaLAMP";
const char* apPassword = "ninjalamp";

void startAP() {
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(apSSID,apPassword);
}

void setPCROperationHandlers () {
  server.on("/connect", requestHandlerConnect);
  server.on("/start", requestHandlerStart);
  server.on("/status", requestHandlerStatus);
  server.on("/stop", requestHandlerStop);
}

/* Scan for nearby APs -> save APs -> start AP */
bool hasPrevWifiError = false;
String prevWifiError;

void startWiFiAPMode() {
    // Load user's WiFi config to fill config form.
    if (isWifiConfDone()) {
        loadWifiConfig();
    }
    startScanning();
    int scanCount = 0;
    while (apCount == 0 || scanCount < 2) {
        scanNearbyAP();
        delay(1000);
        scanCount++;
    }
    stopScanMode();
    startAP();

    server.begin();
    server.on("/", requestHandlerConfInit);
    server.on("/join", requestHandlerConfJoin);
    // Enable PCR control functionalities on AP mode
    setPCROperationHandlers ();
    server.onNotFound(requestHandler404);

    PCR_NETWORK_DEBUG_LINE("OK");
    PCR_NETWORK_DEBUG_LINE("1. Connect AP \"NinjaLAMP\"");
    PCR_NETWORK_DEBUG_LINE("2. Access http://192.168.1.1");
    if (isWiFiConnected()) {
        loopWiFiHTTPServer();
    }
}
boolean isWiFiConnected() {
    return (WiFi.status() == WL_CONNECTED);
}

/* Check WiFi Connection in EEPROM */
bool isWifiConfDone() {
    return EEPROM.read(EEPROM_WIFI_CONF_DONE_ADDR) == EEPROM_WIFI_CONF_DONE_VAL;
}

/*  Load user's WiFi settings from EEPROM  */
void loadWifiConfig () {
    readStringFromEEPROM(ssid, EEPROM_WIFI_SSID_ADDR, EEPROM_WIFI_SSID_MAX_LENGTH);
    readStringFromEEPROM(password, EEPROM_WIFI_PASSWORD_ADDR, EEPROM_WIFI_PASSWORD_MAX_LENGTH);
    readStringFromEEPROM(host, EEPROM_WIFI_MDNS_HOST_ADDR, EEPROM_WIFI_MDNS_HOST_MAX_LENGTH);
}

void saveStringToEEPROM(String str, int startAddress, int maxLength) {
    int len = min(str.length(), maxLength);
    PCR_NETWORK_DEBUG("to EEPROM ");
    PCR_NETWORK_DEBUG(len);
    PCR_NETWORK_DEBUG("bytes @");
    PCR_NETWORK_DEBUG_LINE(startAddress);
    for (int i = 0; i < len; i++) {
        EEPROM.write(startAddress + i, str.charAt(i));
    }
    EEPROM.write(startAddress + len, 0x00); // Write \0
}


void saveWifiResult (uint8_t value) {
  EEPROM.write(EEPROM_WIFI_RESULT, value);
  EEPROM.commit();
}

void saveWiFiConnectionInfo(String ssid, String password, String host) {
    // Flags
    EEPROM.write(EEPROM_WIFI_CONF_DONE_ADDR, EEPROM_WIFI_CONF_DONE_VAL);
    clearOTAFlag();
    // Values
    saveStringToEEPROM(ssid, EEPROM_WIFI_SSID_ADDR,
            EEPROM_WIFI_SSID_MAX_LENGTH);
    saveStringToEEPROM(password, EEPROM_WIFI_PASSWORD_ADDR,
            EEPROM_WIFI_PASSWORD_MAX_LENGTH);
    saveStringToEEPROM(host, EEPROM_WIFI_MDNS_HOST_ADDR,
            EEPROM_WIFI_MDNS_HOST_MAX_LENGTH);
    EEPROM.commit();
}

/* Handle access to software AP */
#define PAGE_INIT 1
#define PAGE_JOIN 2
// Returned string ends with <body> tag.
String getHTMLHeader() {
    return "<!DOCTYPE HTML>\r\n<html><head><title>NinjaLAMP</title></head><body>\r\n";
}
String getPrevWiFiStatusLabel (uint8_t val) {
  switch (val) {
    case WIFI_RESULT_CONNECTION_SUCCESS:
      return "Connected";
    case WIFI_RESULT_CONNECTION_FAILURE:
      return "Connection error";
    case WIFI_RESULT_CONF_DONE:
      return "Conf done";
    case WIFI_RESULT_CONF_ERROR:
      return "Conf error";
    default:
      return "No conf found";
  }
}

void requestHandlerConfInit() {
    // Send form
    boolean isConfDone = isWifiConfDone();
    String s = getHTMLHeader();
    byte *mac = (byte *)malloc(sizeof(byte) * 6);
    WiFi.macAddress(mac);
    if (hasPrevWifiError) {
        s += "<div style=\"color:red\">" + prevWifiError + "</div>";
    }
    s += "<form method=\"post\" action=\"join\">";
    s += "<div>SSID<select name=\"s\"><option value=\"\">----</option>";
    boolean ssidFoundInList = false;
    for (int i = 0; i < apCount; i++) {
        String optionSSID = SSIDs[i];
        optionSSID.replace("\"","\\\"");
        PCR_NETWORK_DEBUG_LINE(optionSSID);
        s += ("<option value=\"" + optionSSID + "\"");
        if (isConfDone && optionSSID.equals(String(ssid))) {
          s += " selected";
          ssidFoundInList = true;
        }
        s += ">" + optionSSID + "</option>";
    }

    s += "</select></div>";
    s += "(If not in the list above)<input name=\"st\"";
    if (isConfDone && !ssidFoundInList) {
      s += (" value=\"" + String(ssid) + "\"");
    }
    s += "/></div>";
    s += "<div>Password<input type=\"password\" name=\"wp\"";
    if (isConfDone) {
      s += (" value=\"" + String(password) + "\"");
    }
    s+="/></div>";
    s += "<div>Device name<input name=\"h\" required minlength=\"4\" maxlength=\"16\" pattern=\"[a-zA-Z0-9]+\" value=\"";
    if (isConfDone) {
      s += String(host);
    } else {
      s += DEFAULT_HOST_NAME;
    }
    s += "\"/>(Alphabetic letters and numbers)</div>";
    s += "<div><input type=\"submit\" value=\"Join\"/></div>";
    s += "</form>";
    s += "Previous WiFi status: ";
    uint8_t prevResult = EEPROM.read(EEPROM_WIFI_RESULT);
    s += getPrevWiFiStatusLabel(prevResult);
    s += " (" + String(prevResult) + ")";
    s += "<br/>MAC ";
    for (int i=5; i>=0; i--) {
      s += String(mac[i],HEX);
      if (i > 0) {
        s += String(":");
      }
    }
    s += "</body></html>\n";
    server.send(200, "text/html", s);
    free(mac);
}
String BR_TAG = "<br/>";
bool isValidHostName (String host) {
    // Length (1 to EEPROM_WIFI_MDNS_HOST_MAX_LENGTH) a-zA-Z0-9, "-"
    if (host.length() < 4 || host.length() > EEPROM_WIFI_MDNS_HOST_MAX_LENGTH) {
        PCR_NETWORK_DEBUG_LINE("Invalid Length");
        return false;
    }
    for (int i=0; i<host.length(); i++) {
        char c = host.charAt(i);
        if (!( ('A'<=c&&c<='Z') || ('a'<=c&&c<='z') || ('0'<=c&&c<='9'))) {
            PCR_NETWORK_DEBUG("Invalid char:");
            PCR_NETWORK_DEBUG_LINE(c);
            return false;
        }
    }
    // Characters
    return true;
}
void requestHandlerConfJoin() {
    PCR_NETWORK_DEBUG_LINE("requestHandlerConfJoin");
    String ssid = server.arg("s"); // Value of dropdown
    String password = server.arg("wp");
    String host = server.arg("h");
    String emptyField = "";
    bool isValid = true;
    if (ssid == "") {
        // Free text
        ssid = server.arg("st");
    }
    if (ssid == "") {
        emptyField = "WiFi SSID";
        isValid = false;
    }
    if (password == "") {
        emptyField = "WiFi Password";
        isValid = false;
    }
    if (!isValidHostName(host)) {
        emptyField = "Device name";
        isValid = false;
    }

    String s = getHTMLHeader();
    if (!isValid) {
        // Has error
        s += BR_TAG;
        s += "ERROR: " + emptyField + " is empty.";
    }
    else {
        s += "<div>SSID:" + ssid + BR_TAG;
        s += "Password: ******" + BR_TAG;
        s += "Device name: " + host + BR_TAG;
        s += "Restarting.";
    }
    s += "</body></html>\n";
    server.send(200, "text/html", s);

    if (isValid) {
        PCR_NETWORK_DEBUG_LINE("Valid input. Saving...");
        saveWiFiConnectionInfo(ssid, password, host);
        PCR_NETWORK_DEBUG_LINE("saved.");
        saveWifiResult(WIFI_RESULT_CONF_DONE);
        delay(4000);
        ESP.restart();
    }
    else {
        saveWifiResult(WIFI_RESULT_CONF_ERROR);
        PCR_NETWORK_DEBUG_LINE("Invalid input.");
    }
}

void readStringFromEEPROM(char *s, int startAddress, int maxLength) {
    int index = 0;
    while (index < maxLength) {
        char c = EEPROM.read(index + startAddress);
        s[index++] = c;
        if (c == 0x00)
            return;
    }
    s[maxLength] = 0x00;
}

String byteToHexStr(char c) {
    char s[3];
    sprintf(s, "%02X", c);
    return String(s);
}

#define WIFI_TIMEOUT_SEC 100

/* Return true if connected successfully */
#define CONNECTION_CHECK_INTERVAL 100 /* msec */

boolean connectToAP (int timeoutInMillis) {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect (true);
    
    PCR_NETWORK_DEBUG_LINE(ssid);
    PCR_NETWORK_DEBUG_LINE(password);
    wifiMulti.addAP(ssid, password);
    PCR_NETWORK_DEBUG_LINE("WiFi connecting.");
    bool noTimeout = (timeoutInMillis==0);
    saveWifiResult(WIFI_RESULT_CONNECTION_FAILURE);

    do {
      if (wifiMulti.run()==WL_CONNECTED) {
        saveWifiResult(WIFI_RESULT_CONNECTION_SUCCESS);
        return true;
      }
      PCR_NETWORK_DEBUG(".");
      delay(WIFI_TIMEOUT_SEC);
      timeoutInMillis -= WIFI_TIMEOUT_SEC;
    } while (timeoutInMillis > 0 || noTimeout);
    PCR_NETWORK_DEBUG_LINE("Connection timeout.");
    return false;

}
/* Start network as a HTTP server */
boolean startWiFiHTTPServerMode() {
    // Check existence of WiFi Config

    if (!isWifiConfDone()) {
        PCR_NETWORK_DEBUG_LINE("WiFi config not found.");
        return false;
    }

    // Load OTA mode from EEPROM
    loadOTAConfig();
    // Load connection info from EEPROM
    int ssidIndex = 0;
    loadWifiConfig();

    // Connect with saved SSID and password

    PCR_NETWORK_DEBUG("SSID:"); PCR_NETWORK_DEBUG_LINE(ssid);
    PCR_NETWORK_DEBUG_LINE(strlen(ssid));
    PCR_NETWORK_DEBUG("Pass:"); PCR_NETWORK_DEBUG_LINE(password);
    PCR_NETWORK_DEBUG_LINE(strlen(password));
    PCR_NETWORK_DEBUG("Host:"); PCR_NETWORK_DEBUG_LINE(host);

    if (isUpdateMode) {
        connectToAP(0); // No timeout
        while (WiFi.status() != WL_CONNECTED) {
          PCR_NETWORK_DEBUG(".");
          delay(500);
        }
        PCR_NETWORK_DEBUG_LINE("WiFi connected.");
        delay(2000);
        EEPROM.end();
        execUpdate();
        // startUpdaterServer();
    }
    else {
      // Timeout=10sec
      if (connectToAP(10 * 1000)) {
        PCR_NETWORK_DEBUG_LINE("Connected.");
        beginMDNS(host);
        PCR_NETWORK_DEBUG_LINE("MDNS ok.");
        PCR_NETWORK_DEBUG("\nConnected.IP=");
        PCR_NETWORK_DEBUG_LINE(WiFi.localIP());
        startNormalOperationServer();
        return true;
      } else {
        return false;
      }
    }

}
int beginMDNS (const char *hostName) {
    MDNS.begin(hostName);
    MDNS.addService("http", "tcp", 80);
    PCR_NETWORK_DEBUG_LINE("MDNS started.");
    PCR_NETWORK_DEBUG_LINE(hostName);
}

void startNormalOperationServer() {
    // Normal PCR operation mode
    /* Add handlers for paths */
    // Never remove it (for OTA configuration)
    server.on("/config", requestHandlerConfig);
    server.on("/update", requestHandlerFirmwareUpdate);

    server.on("/", requestHandlerTop);

    setPCROperationHandlers ();

    server.onNotFound(requestHandler404);
    server.begin();
    PCR_NETWORK_DEBUG_LINE("Server started");
}

/* Handle HTTP request as a server */
void loopWiFiHTTPServer() {
    server.handleClient();
}
/* Handle HTTP request as AP */
void loopWiFiAP() {
    server.handleClient();
}
