/**************************************************************
   WiFiManager is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/master/libraries/DNSServer/examples/CaptivePortalAdvanced
   Built by AlexT https://github.com/tzapu
   Licensed under MIT license


  Last custom and update August 2022 gabriele.foddis@lifely.cc
 **************************************************************/

#ifndef WiFiManagerWithAPI_h
#define WiFiManagerWithAPI_h


//#include <supportandsectrets.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <memory>



extern "C" {
  #include "user_interface.h"
  
}
const char HTTP_HEADER[] PROGMEM          = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{}</title><img title=' alt=' src='data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAMgAAABNCAYAAADjJSv1AAAEsmlUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4KPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUgNS41LjAiPgogPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4KICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIgogICAgeG1sbnM6dGlmZj0iaHR0cDovL25zLmFkb2JlLmNvbS90aWZmLzEuMC8iCiAgICB4bWxuczpleGlmPSJodHRwOi8vbnMuYWRvYmUuY29tL2V4aWYvMS4wLyIKICAgIHhtbG5zOnBob3Rvc2hvcD0iaHR0cDovL25zLmFkb2JlLmNvbS9waG90b3Nob3AvMS4wLyIKICAgIHhtbG5zOnhtcD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wLyIKICAgIHhtbG5zOnhtcE1NPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvbW0vIgogICAgeG1sbnM6c3RFdnQ9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZUV2ZW50IyIKICAgdGlmZjpJbWFnZUxlbmd0aD0iNzciCiAgIHRpZmY6SW1hZ2VXaWR0aD0iMjAwIgogICB0aWZmOlJlc29sdXRpb25Vbml0PSIyIgogICB0aWZmOlhSZXNvbHV0aW9uPSI3Mi8xIgogICB0aWZmOllSZXNvbHV0aW9uPSI3Mi8xIgogICBleGlmOlBpeGVsWERpbWVuc2lvbj0iMjAwIgogICBleGlmOlBpeGVsWURpbWVuc2lvbj0iNzciCiAgIGV4aWY6Q29sb3JTcGFjZT0iMSIKICAgcGhvdG9zaG9wOkNvbG9yTW9kZT0iMyIKICAgcGhvdG9zaG9wOklDQ1Byb2ZpbGU9InNSR0IgSUVDNjE5NjYtMi4xIgogICB4bXA6TW9kaWZ5RGF0ZT0iMjAyMi0w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' />"; 
const char HTTP_STYLE[] PROGMEM           = "<style>.c{text-align: center;} dt{color:#0a0a0a} dd{color:#636769;text-align:center;margin:0px} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:calibri;} button{border:0;border-radius:0.3rem;background-color:#165914;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAIAAAD8GO2jAAAEr2lUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4KPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUgNS41LjAiPgogPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4KICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIgogICAgeG1sbnM6dGlmZj0iaHR0cDovL25zLmFkb2JlLmNvbS90aWZmLzEuMC8iCiAgICB4bWxuczpleGlmPSJodHRwOi8vbnMuYWRvYmUuY29tL2V4aWYvMS4wLyIKICAgIHhtbG5zOnBob3Rvc2hvcD0iaHR0cDovL25zLmFkb2JlLmNvbS9waG90b3Nob3AvMS4wLyIKICAgIHhtbG5zOnhtcD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wLyIKICAgIHhtbG5zOnhtcE1NPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvbW0vIgogICAgeG1sbnM6c3RFdnQ9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZUV2ZW50IyIKICAgdGlmZjpJbWFnZUxlbmd0aD0iMzIiCiAgIHRpZmY6SW1hZ2VXaWR0aD0iMzIiCiAgIHRpZmY6UmVzb2x1dGlvblVuaXQ9IjIiCiAgIHRpZmY6WFJlc29sdXRpb249IjcyLjAiCiAgIHRpZmY6WVJlc29sdXRpb249IjcyLjAiCiAgIGV4aWY6UGl4ZWxYRGltZW5zaW9uPSIzMiIKICAgZXhpZjpQaXhlbFlEaW1lbnNpb249IjMyIgogICBleGlmOkNvbG9yU3BhY2U9IjEiCiAgIHBob3Rvc2hvcDpDb2xvck1vZGU9IjMiCiAgIHBob3Rvc2hvcDpJQ0NQcm9maWxlPSJzUkdCIElFQzYxOTY2LTIuMSIKICAgeG1wOk1vZGlmeURhdGU9IjIwMjEtMDYtMjRUMTI6Mzg6MTgrMDI6MDAiCiAgIHhtcDpNZXRhZGF0YURhdGU9IjIwMjEtMDYtMjRUMTI6Mzg6MTgrMDI6MDAiPgogICA8eG1wTU06SGlzdG9yeT4KICAgIDxyZGY6U2VxPgogICAgIDxyZGY6bGkKICAgICAgc3RFdnQ6YWN0aW9uPSJwcm9kdWNlZCIKICAgICAgc3RFdnQ6c29mdHdhcmVBZ2VudD0iQWZmaW5pdHkgUGhvdG8gMS45LjIiCiAgICAgIHN0RXZ0OndoZW49IjIwMjEtMDYtMjRUMTI6Mzg6MTgrMDI6MDAiLz4KICAgIDwvcmRmOlNlcT4KICAgPC94bXBNTTpIaXN0b3J5PgogIDwvcmRmOkRlc2NyaXB0aW9uPgogPC9yZGY6UkRGPgo8L3g6eG1wbWV0YT4KPD94cGFja2V0IGVuZD0iciI/PitgGxsAAAGCaUNDUHNSR0IgSUVDNjE5NjYtMi4xAAAokXWRu0sDQRCHv8QnGlHQwsIiaLQQIzGCaGOR4AvUIongq0kuLyGP4+5ExFawDSiINr4K/Qu0FawFQVEEsROsFW00nHMmkCBmltn59rc7w+4s2EMpJa1XeyCdMbTAhM85v7DorHuhlnYa6KI3rOjqTHA8REX7vMdmxVu3VavyuX+tMRrTFbDVC48qqmYITwpPrxmqxTvCbUoyHBU+E+7T5ILCd5YeKfCrxYkCf1ushQJ+sLcIOxNlHCljJamlheXluNKpVaV4H+sljlhmLiixU7wDnQAT+HAyxRh+hhhgROYh3HjplxUV8j2/+bNkJVeRWWUdjRUSJDHoE3VVqsckxkWPyUixbvX/b1/1+KC3UN3hg5pn03zvhrptyOdM8+vINPPHUPUEl5lSfvYQhj9Ez5U01wE0b8L5VUmL7MLFFrQ/qmEt/CtVidvjcXg7haYFaL2BhqVCz4r7nDxAaEO+6hr29qFHzjcv/wCTM2f6ggEw6QAAAAlwSFlzAAALEwAACxMBAJqcGAAAAHtJREFUSIlj/P//PwMtARNNTWdgYGAhSpWcCnbxR3cIaiXCB7hMxy9FrAUEjSCkAK8FRDiQoDJGnKkIUxtyiOOXRQJEpyI0/URELykWYDWOODtong9GLRh4CzDyAZGZCw9ATV1DP4hGLRi1YNSCUQtGhAW420VUAjT3AQAs1xy3OF9rzgAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEADER_END[] PROGMEM      = "</head><body><div style='text-align:left;display:inline-block;min-width:260px; color:#000000'>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (manually)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info Device</button></form><br/><form action=\"/api-customerCare\" method=\"get\"><button>Customer Care</button></form><br/><br/><form action=\"/r\" method=\"post\"><button id='btn'>!!!! Reset !!!!</button></form><br/>";
const char HTTP_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM      = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID(type your network name)'><br/><input id='p' name='p' length=64 type='text' placeholder='type password'><br/>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<br/><input id='{i}' name='{n}' maxlength={l} placeholder='{p}' value='{v}' {c}>";
const char HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>Save</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">Tap to Find all WiFi networks available</a></div>";
const char HTTP_SAVED[] PROGMEM           = "<div><b>Credentials Saved</b><br />Trying to connect Agrumino to network.<br />If it fails reconnect to AP and try again</div>"; ///Ga Mod
const char HTTP_END[] PROGMEM             = "</div></body></html>";
const char API_SCAN_ITEM[] PROGMEM        = "{\"ssid\":\"{v}\",\"rssi\":{r},\"enc\":{i}}";
const char HTTP_STYLE_CUSTOMER_CARE[] PROGMEM  = "<style>.c{text-align: center;} #btn{border-style:solid; border-color:#000000; border:1;border-radius:1.2rem;background-color:#ff0000;color:#ffffff;line-height:2.4rem;font-size:1.2rem;width:100%;} p.outset {border-style:outset} h2{color:#000000; text-align:center } h3{color:#000000; border-style:outset} mark{background-color:#ffffb3; color:#000000} dt{color:#0a0a0a;text-align:center} dd{color:#636769;text-align:center;margin:0px} div,input{padding:5px;font-size:1em;} input{width:95%;} body{text-align: center;font-family:calibri;} button{border:1;border-radius:1.0rem;background-color:#165914;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAIAAAD8GO2jAAAEr2lUWHRYTUw6Y29tLmFkb2JlLnhtcAAAAAAAPD94cGFja2V0IGJlZ2luPSLvu78iIGlkPSJXNU0wTXBDZWhpSHpyZVN6TlRjemtjOWQiPz4KPHg6eG1wbWV0YSB4bWxuczp4PSJhZG9iZTpuczptZXRhLyIgeDp4bXB0az0iWE1QIENvcmUgNS41LjAiPgogPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4KICA8cmRmOkRlc2NyaXB0aW9uIHJkZjphYm91dD0iIgogICAgeG1sbnM6dGlmZj0iaHR0cDovL25zLmFkb2JlLmNvbS90aWZmLzEuMC8iCiAgICB4bWxuczpleGlmPSJodHRwOi8vbnMuYWRvYmUuY29tL2V4aWYvMS4wLyIKICAgIHhtbG5zOnBob3Rvc2hvcD0iaHR0cDovL25zLmFkb2JlLmNvbS9waG90b3Nob3AvMS4wLyIKICAgIHhtbG5zOnhtcD0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wLyIKICAgIHhtbG5zOnhtcE1NPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvbW0vIgogICAgeG1sbnM6c3RFdnQ9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZUV2ZW50IyIKICAgdGlmZjpJbWFnZUxlbmd0aD0iMzIiCiAgIHRpZmY6SW1hZ2VXaWR0aD0iMzIiCiAgIHRpZmY6UmVzb2x1dGlvblVuaXQ9IjIiCiAgIHRpZmY6WFJlc29sdXRpb249IjcyLjAiCiAgIHRpZmY6WVJlc29sdXRpb249IjcyLjAiCiAgIGV4aWY6UGl4ZWxYRGltZW5zaW9uPSIzMiIKICAgZXhpZjpQaXhlbFlEaW1lbnNpb249IjMyIgogICBleGlmOkNvbG9yU3BhY2U9IjEiCiAgIHBob3Rvc2hvcDpDb2xvck1vZGU9IjMiCiAgIHBob3Rvc2hvcDpJQ0NQcm9maWxlPSJzUkdCIElFQzYxOTY2LTIuMSIKICAgeG1wOk1vZGlmeURhdGU9IjIwMjEtMDYtMjRUMTI6Mzg6MTgrMDI6MDAiCiAgIHhtcDpNZXRhZGF0YURhdGU9IjIwMjEtMDYtMjRUMTI6Mzg6MTgrMDI6MDAiPgogICA8eG1wTU06SGlzdG9yeT4KICAgIDxyZGY6U2VxPgogICAgIDxyZGY6bGkKICAgICAgc3RFdnQ6YWN0aW9uPSJwcm9kdWNlZCIKICAgICAgc3RFdnQ6c29mdHdhcmVBZ2VudD0iQWZmaW5pdHkgUGhvdG8gMS45LjIiCiAgICAgIHN0RXZ0OndoZW49IjIwMjEtMDYtMjRUMTI6Mzg6MTgrMDI6MDAiLz4KICAgIDwvcmRmOlNlcT4KICAgPC94bXBNTTpIaXN0b3J5PgogIDwvcmRmOkRlc2NyaXB0aW9uPgogPC9yZGY6UkRGPgo8L3g6eG1wbWV0YT4KPD94cGFja2V0IGVuZD0iciI/PitgGxsAAAGCaUNDUHNSR0IgSUVDNjE5NjYtMi4xAAAokXWRu0sDQRCHv8QnGlHQwsIiaLQQIzGCaGOR4AvUIongq0kuLyGP4+5ExFawDSiINr4K/Qu0FawFQVEEsROsFW00nHMmkCBmltn59rc7w+4s2EMpJa1XeyCdMbTAhM85v7DorHuhlnYa6KI3rOjqTHA8REX7vMdmxVu3VavyuX+tMRrTFbDVC48qqmYITwpPrxmqxTvCbUoyHBU+E+7T5ILCd5YeKfCrxYkCf1ushQJ+sLcIOxNlHCljJamlheXluNKpVaV4H+sljlhmLiixU7wDnQAT+HAyxRh+hhhgROYh3HjplxUV8j2/+bNkJVeRWWUdjRUSJDHoE3VVqsckxkWPyUixbvX/b1/1+KC3UN3hg5pn03zvhrptyOdM8+vINPPHUPUEl5lSfvYQhj9Ez5U01wE0b8L5VUmL7MLFFrQ/qmEt/CtVidvjcXg7haYFaL2BhqVCz4r7nDxAaEO+6hr29qFHzjcv/wCTM2f6ggEw6QAAAAlwSFlzAAALEwAACxMBAJqcGAAAAHtJREFUSIlj/P//PwMtARNNTWdgYGAhSpWcCnbxR3cIaiXCB7hMxy9FrAUEjSCkAK8FRDiQoDJGnKkIUxtyiOOXRQJEpyI0/URELykWYDWOODtong9GLRh4CzDyAZGZCw9ATV1DP4hGLRi1YNSCUQtGhAW420VUAjT3AQAs1xy3OF9rzgAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char HTTP_HEADER_CUSTOMER_CARE_END[] PROGMEM = "</head><body><dt style='color:#0a0a0a'><div style='text-align:center;display:inline-block;min-width:260px; color:#000000'>";
const char HTTP_HEADER_END_MAIN[] PROGMEM      = "</head><body><div style='text-align:center;display:inline-block;min-width:260px; color:#000000'>"; //DD JUPYERT
#ifndef WIFI_MANAGER_MAX_PARAMS
#define WIFI_MANAGER_MAX_PARAMS 10
#endif

class WiFiManagerWithAPIParameter {
  public:
    /** 
        Create custom parameters that can be added to the WiFiManagerWithAPI setup web page
        @id is used for HTTP queries and must not contain spaces nor other special characters
    */
    WiFiManagerWithAPIParameter(const char *custom);
    WiFiManagerWithAPIParameter(const char *id, const char *placeholder, const char *defaultValue, int length);
    WiFiManagerWithAPIParameter(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);
    ~WiFiManagerWithAPIParameter();

    const char *getID();
    const char *getValue();
    const char *getPlaceholder();
    int         getValueLength();
    const char *getCustomHTML();
  private:
    const char *_id;
    const char *_placeholder;
    char       *_value;
    int         _length;
    const char *_customHTML;

    void init(const char *id, const char *placeholder, const char *defaultValue, int length, const char *custom);

    friend class WiFiManagerWithAPI;
};


class WiFiManagerWithAPI
{
  public:
    WiFiManagerWithAPI();
    ~WiFiManagerWithAPI();

    boolean       autoConnect();
    boolean       autoConnect(char const *apName, char const *apPassword = NULL);

    //if you want to always start the config portal, without trying to connect first
    boolean       startConfigPortal();
    boolean       startConfigPortal(char const *apName, char const *apPassword = NULL);

    // get the AP name of the config portal, so it can be used in the callback
    String        getConfigPortalSSID();

    void          resetSettings();

    //sets timeout before webserver loop ends and exits even if there has been no setup.
    //useful for devices that failed to connect at some point and got stuck in a webserver loop
    //in seconds setConfigPortalTimeout is a new name for setTimeout
    void          setConfigPortalTimeout(unsigned long seconds);
    void          setTimeout(unsigned long seconds);

    //sets timeout for which to attempt connecting, useful if you get a lot of failed connects
    void          setConnectTimeout(unsigned long seconds);


    void          setDebugOutput(boolean debug);
    //defaults to not showing anything under 8% signal quality if called
    void          setMinimumSignalQuality(int quality = 8);
    //sets a custom ip /gateway /subnet configuration
    void          setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //sets config for a static IP
    void          setSTAStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
    //called when AP mode and config portal is started
    void          setAPCallback( void (*func)(WiFiManagerWithAPI*) );
    //called when settings have been changed and connection was successful
    void          setSaveConfigCallback( void (*func)(void) );
    //adds a custom parameter, returns false on failure
    bool          addParameter(WiFiManagerWithAPIParameter *p);
    //if this is set, it will exit after config, even if connection is unsuccessful.
    void          setBreakAfterConfig(boolean shouldBreak);
    //if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)
    //TODO
    //if this is set, customise style
    void          setCustomHeadElement(const char* element);
    //if this is true, remove duplicated Access Points - defaut true
    void          setRemoveDuplicateAPs(boolean removeDuplicates);

  private:
    std::unique_ptr<DNSServer>        dnsServer;
    std::unique_ptr<ESP8266WebServer> server;

    //const int     WM_DONE                 = 0;
    //const int     WM_WAIT                 = 10;

    //const String  HTTP_HEADER = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

    void          setupConfigPortal();
    void          startWPS();

    const char*   _apName                 = "no-net";
    const char*   _apPassword             = NULL;
    String        _ssid                   = "";
    String        _pass                   = "";
    unsigned long _configPortalTimeout    = 0;
    unsigned long _connectTimeout         = 0;
    unsigned long _configPortalStart      = 0;

    IPAddress     _ap_static_ip;
    IPAddress     _ap_static_gw;
    IPAddress     _ap_static_sn;
    IPAddress     _sta_static_ip;
    IPAddress     _sta_static_gw;
    IPAddress     _sta_static_sn;

    int           _paramsCount            = 0;
    int           _minimumQuality         = -1;
    boolean       _removeDuplicateAPs     = true;
    boolean       _shouldBreakAfterConfig = false;
    boolean       _tryWPS                 = false;

    const char*   _customHeadElement      = "";

    //String        getEEPROMString(int start, int len);
    //void          setEEPROMString(int start, int len, String string);

    int           status = WL_IDLE_STATUS;
    int           connectWifi(String ssid, String pass);
    uint8_t       waitForConnectResult();

    void          handleRoot();
    void          handleWifi(boolean scan);
    void          handleWifiSave();
    void          handleInfo();
    void          handleReset();
    void          handleNotFound();
    void          handle204();
    boolean       captivePortal();
    boolean       configPortalHasTimeout();

    void          handleApiScan();
    void          handleApiInfo();
    void          handleApiWifiSave();
    void          handleCustomerCare();

    // DNS server
    const byte    DNS_PORT = 53;

    //helpers
    int           getRSSIasQuality(int RSSI);
    boolean       isIp(String str);
    String        toStringIp(IPAddress ip);

    boolean       connect;
    boolean       _debug = true;

    void (*_apcallback)(WiFiManagerWithAPI*) = NULL;
    void (*_savecallback)(void) = NULL;

    int                    _max_params;
    WiFiManagerWithAPIParameter** _params;

    template <typename Generic>
    void          DEBUG_WM(Generic text);

    template <class T>
    auto optionalIPFromString(T *obj, const char *s) -> decltype(  obj->fromString(s)  ) {
      return  obj->fromString(s);
    }
    auto optionalIPFromString(...) -> bool {
      DEBUG_WM("NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work.");
      return false;
    }
};

#endif
