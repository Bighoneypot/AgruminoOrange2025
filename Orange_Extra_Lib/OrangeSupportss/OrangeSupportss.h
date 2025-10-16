/*
  Created by gabriele.foddis@lifely.cc on July 2022.
  
  Latest changes and updates August 2022 by:
  
  Gabriele Foddis gabriele.foddis@lifely.cc
  for a better experience use only Visual Studio Code

*/

#ifndef ORANGESUPPORTSS_H
#define ORANGESUPPORTSS_H

extern const char* WEB_SERVER_HOST;
extern const String WEB_SERVER_API_SEND_DATA;
extern const String WEB_SERVER_API_REGISTER_DEVICE;
extern int16_t sleepTimeSec;
extern String supportSiteLink;
extern String officialSupportLink;
extern uint32 flashChipId;
extern String convFlashchipId;
extern uint32 espChipId;
extern String convEspChipId;
extern String deviceType;
extern String FULLNAME;
extern String DEVICE_KEY;
extern String DEVICE_TOKEN; 
extern String FIRMWARE_SHORT_NAME;
extern String REL;
extern String FIRMWARE_RELEASE;
extern String TOKEN_CODE;
extern String ID_DEVICE;
extern String VARIANT;
extern String REVISION;
extern String DATEPROD;
extern String RESELLERPROD;
extern String supportMail;
extern String textMail;
extern String textMail2;
extern String textMail3;
extern String textMail4;

void printSupport();

#endif