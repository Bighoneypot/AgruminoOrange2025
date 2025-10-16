/*
  Created by gabriele.foddis@lifely.cc on July 2022.
  
  Latest changes and updates September 2022 by:
  
  Gabriele Foddis gabriele.foddis@lifely.cc
  for a better experience use only Visual Studio Code

*/


#include <Arduino.h>
#include "OrangeSupportss.h"
#include <ArduinoJson.h>

int16_t sleepTimeSec = 3600; //60 min (3600 is MAX VALUE)
const char* WEB_SERVER_HOST = "api.lifely.cc";
const String WEB_SERVER_API_SEND_DATA = "/api/v1/objects/device_observation/";
const String WEB_SERVER_API_REGISTER_DEVICE = "/api/v1/objects/device_token/";


uint32 flashChipId = ESP.getFlashChipId();
String convFlashchipId = String(flashChipId);
uint32 espChipId = ESP.getChipId();
String convEspChipId = String(espChipId);
String deviceType = "agrumino-";
String FULLNAME = deviceType + espChipId;
String DEVICE_KEY = FULLNAME; /// fullname device
String DEVICE_TOKEN = "Ymh$9C%Ex&741@^Up$zqTdq!Lvb"; // Write Token of APP (Key) 
String FIRMWARE_SHORT_NAME= "AOCPSS";////SHORT FW NAME
String REL ="315";//TAG RELEASE NUMBER
String FIRMWARE_RELEASE = FIRMWARE_SHORT_NAME + REL; //FIRMWARE NAME + TAG RELEASE VERSION
String TOKEN_CODE = "LTO1";///REF TOKEN FOR DEVICE
String ID_DEVICE = convEspChipId;///CHIP_ID
String VARIANT = "Orange";//VARIANT DEVICE
String REVISION = "1.1"; ///REVISION DEVICE
String DATEPROD = "1025"; //DATA TO BURN FIRMWARE 
String RESELLERPROD = "LIFELY.CC";///NAME CUSTOMER IF PRESENT (NN IS NOT PRESENT)
String supportSiteLink = "https://www.lifely.cc/it/setup/";
String officialSupportLink = "<a href="+supportSiteLink+">Official Lifely Support!</a>";
String supportMail = "support@lifely.cc";///send support request
String textMail = "<a href='mailto:"+supportMail+"?subject=I%20need%20support%20for%20device  "+FULLNAME+"&body=Dear%20Lifely%20Team%2C%0AI%20need%20support%20for%20my%20device%0A%0AMy%20device%20specs%3A%0A%7B%0A%20%20%22DT:%22%3A%22"+deviceType+"%22%2C%0A%20%20%22ID%22%3A%22"+convEspChipId+"%22%2C%0A%20%20%22FN%22%3A%22"+FULLNAME+"%22%2C%0A%20%20%22VR%22%3A%22"+VARIANT+"%22%2C%0A%20%20%22FR%22%3A%22"+FIRMWARE_RELEASE+"%22%2C%0A%20%20%22TC%22%3A%22"+TOKEN_CODE+"%22%2C%0A%20%20%22RV%22%3A%22"+REVISION+"%22%2C%0A%20%20%22DP%22%3A%22"+DATEPROD+"%22%2C%0A%20%20%22RS%22%3A%22"+RESELLERPROD+"%22%0A%7D%0A%0AMy%20question%20is%3A%0A%0A'>"+""+supportMail+""+"</a>";


void printSupport()
{
DynamicJsonDocument suppInfo(1024);
 
suppInfo["DT"] = deviceType;
suppInfo["ID"] = convEspChipId;
suppInfo["FN"] = FULLNAME;
suppInfo["VR"] = VARIANT;
suppInfo["FR"] = FIRMWARE_RELEASE;
suppInfo["TC"] = TOKEN_CODE;
suppInfo["RV"] = REVISION;
suppInfo["DP"] = DATEPROD;
suppInfo["RS"] = RESELLERPROD;
Serial.println("/////////////////////////////////////////////");
Serial.println("Please, for Customer Care send mail to " + supportMail);
Serial.println("with subject: I need support for my "+ FULLNAME);
Serial.println("write a problem statement and ");
Serial.println("send this json data: ");
Serial.println("");
serializeJsonPretty(suppInfo, Serial);
Serial.println("");
Serial.println("/////////////////////////////////////////////");

}