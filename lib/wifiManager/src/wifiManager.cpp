#include "wifiManager.h"
#include "defines.h"


WiFiManager::WiFiManager()
{
  LED voidLed(-1);
  this->wifiLed = &voidLed;
  disableWiFi();
  WiFi.persistent(false);
  lastCall = millis();
}

WiFiManager::WiFiManager(LED *newWifiLed)
{
    this->wifiLed = newWifiLed; //Reference to WiFi LED to Control it with Handler class
    disableWiFi();
    WiFi.persistent(false);
    lastCall = millis();
}

/*
  Get Stuff
*/

bool WiFiManager::getShieldState()
{
  return shieldState;
}

bool WiFiManager::getOverrideSettingsToPreventError()
{
  return overrideSettingsToPreventError;
}

bool WiFiManager::getLockClass()
{
  return lockClass;
}

uint32_t WiFiManager::getDeviceMac()
{
  return deviceMac;
}

int WiFiManager::getCheckDelay()
{
  return checkDelay;
}

ulong WiFiManager::getCallPerSecond()
{
  return callPerSecond;
}

classErrorReport WiFiManager::getCurrentErrorState()
{
  return error;
}

bool WiFiManager::getWiFiAutoConnect()
{
  return WiFi.getAutoConnect();
}

bool WiFiManager::getWiFiAutoReconnect()
{
  return WiFi.getAutoReconnect();
}

const char* WiFiManager::getWiFiHostname()
{
  return WiFi.hostname().c_str();
}

String WiFiManager::getWiFiHostnameAsString()
{
  return WiFi.hostname();
}

macAdress WiFiManager::getStationMac()
{
  static uint8_t macAddr[6];
    WiFi.macAddress(macAddr);
    macAdress array;
    for(int i = 0; i < 6; i++)
    {
        array.macAddr[i] = macAddr[i];
    }
    return array;
}

String WiFiManager::getStationMacAsString()
{
  return WiFi.macAddress();
}

const char* WiFiManager::getStationMacAsChar()
{
  return WiFi.macAddress().c_str();
}

wl_status_t WiFiManager::getWiFiState()
{
  return WiFi.status();
}

String WiFiManager::getBSSID()
{
  return WiFi.BSSIDstr();
}

uint8_t* WiFiManager::getBSSIDAsInt()
{
  return WiFi.BSSID();
}

int32_t WiFiManager::getRSSI()
{
  return WiFi.RSSI();
}

WiFiClient* WiFiManager::getWiFiClient()
{
  return &localWiFiClient;
}

String WiFiManager::getLocalIP()
{
  return WiFi.localIP().toString();
}

/*
  Set stuff
*/

void WiFiManager::setShieldState(bool newState)
{
  #ifdef J54J6_LOGGING_H
      logger logging;
      String message = "Update ShieldState: ";
      message += shieldState;
      message += " -> ";
      message += newState;
      logging.SFLog(className, "setShieldState", message.c_str());
  #endif
  shieldState = newState;
}

void WiFiManager::setOverrideSettingsToPreventError(bool newValue)
{
  #ifdef J54J6_LOGGING_H
      logger logging;
      String message = "Update OverrideSettingsToPreventError: ";
      message += overrideSettingsToPreventError;
      message += " -> ";
      message += newValue;
      logging.SFLog(className, "setOverrideSettingsToPreventError", message.c_str());
  #endif
  overrideSettingsToPreventError = newValue;
}

void WiFiManager::setCheckDelay(int newValue)
{
  #ifdef J54J6_LOGGING_H
      logger logging;
      String message = "Update CheckDelay: ";
      message += checkDelay;
      message += " -> ";
      message += newValue;
      logging.SFLog(className, "setCheckDelay", message.c_str());
  #endif
  checkDelay = newValue;
}

void WiFiManager::setLockClass(bool newValue)
{
  #ifdef J54J6_LOGGING_H
      logger logging;
      String message = "Update LockClass: ";
      message += lockClass;
      message += " -> ";
      message += newValue;

      logging.SFLog(className, "setLockClass", message.c_str());
  #endif
  if(newValue)
  {
    error.error = true;
    error.ErrorCode = 214;
    error.message = "WiFiManager Class locked!";
    error.priority = 5;
    #ifdef J54J6_LOGGING_H
     logger logging;
     logging.SFLog(className, "setLockClass", "Report lock to ErrorHandler!", 0);
    #endif
  }
  lockClass = newValue;
}

bool WiFiManager::setWiFiAutoConnect(bool newValue)
{
  if(WiFi.setAutoConnect(newValue))
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      String message = "Update WiFi->AutoConnect: ";
      message += getWiFiAutoConnect();
      message += " -> ";
      message += newValue;
      logging.SFLog(className, "setWiFiAutoConnect", message.c_str());
    #endif
    return true;
  }
  else
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      String message = "Can't Update WiFi->AutoConnect: ";
      message += getWiFiAutoConnect();
      message += " -> ";
      message += newValue;
      logging.SFLog(className, "setWiFiAutoConnect", message.c_str(), 2);
    #endif
    error.ErrorCode = 99;
    error.message = "Can't update Autoconnect!";
    error.priority = 0;
    return false;
  }  
}

bool WiFiManager::setWiFiAutoReconnect(bool newValue)
{
  if(WiFi.setAutoReconnect(newValue))
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      String message = "Update WiFi->AutoReconnect: ";
      message += getWiFiAutoReconnect();
      message += " -> ";
      message += newValue;
      logging.SFLog(className, "setWiFiAutoReconnect", message.c_str());
    #endif
    return true;
  }
  else
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      String message = "Can't Update WiFi->AutoReconnect: ";
      message += getWiFiAutoReconnect();
      message +=" -> ";
      message += newValue;
      logging.SFLog(className, "setWiFiAutoReconnect", message.c_str(), 2);
    #endif
    error.ErrorCode = 99;
    error.message = "Can't update AutoReconnect!";
    error.priority = 0;
    return false;
  }
}

bool WiFiManager::setWiFiHostname(const char* hostname)
{
  if(hostname == WiFi.hostname().c_str())
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "setWiFiHostname", "Hostname already set! - SKIP", 1);
    #endif
    return true;
  }

  if(WiFi.hostname(hostname))
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      String message = "Update WiFi->Hostname: ";
      message += WiFi.hostname();
      message += " -> ";
      message += hostname;
      logging.SFLog(className, "setWiFiHostname", message.c_str());
    #endif
    return true;
  }
  else
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      String message = "Can't Update WiFi->Hostname: ";
      message += WiFi.hostname();
      message += " -> ";
      message += hostname;
      logging.SFLog(className, "setWiFiHostname", message.c_str(), 2);
    #endif
    error.ErrorCode = 99;
    error.message = "Can't update Hostname!";
    error.priority = 0;
    return false;
  }
}

bool WiFiManager::setWiFiConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2)
{
  
  if(WiFi.config(local_ip, gateway, subnet, dns1, dns2))
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "setWiFiConfig", "WiFi Config successfully updated!", 0);
    #endif
    return true;
  }
  else
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "setWiFiConfig", "Can't update WiFi Config - Report!", 2);
    #endif
    error.error = true;
    error.ErrorCode = 432;
    error.message = "can't update WiFi Config - class return 'false'!";
    error.priority = 5;
    setLockClass(true);
  }
  return false;
}

/*
  General functionalities
*/
void WiFiManager::enableWiFi(WiFiMode_t mode)
{
  if(!shieldState)
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "enableWiFi", "enable WiFi");
    #endif
    shieldState = true; //wifi enabled
    wifi_fpm_do_wakeup();
    wifi_fpm_close();
    WiFi.mode(mode);
  }
  else
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "enableWiFi", "WiFi already enabled - SKIP", 1);
    #endif
  }
  
}

void WiFiManager::disableWiFi()
{
  if(shieldState)
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "disableWiFi", "disable WiFi");
    #endif
    shieldState = false; //wifi disabled
    wifi_station_disconnect();
    WiFi.mode( WIFI_OFF );
    WiFi.forceSleepBegin();
  }
  else
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "disableWiFi", "WiFi already disabled - SKIP", 1);
    #endif
  }
}

void WiFiManager::setWiFiMode(WiFiMode_t mode)
{
  WiFi.mode(mode);
}


bool WiFiManager::startWifiAP(const char *ssid, const char *passwd, int hidden, int channel)
{
  if(apActive)
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "startWifiAP", "AP already enabled - SKIP", 1);
    #endif
    return true;
  }

  if(!this->shieldState) //if wifiIsDisabled and overrideSettings is enabled - enableWifi and then start WifiAP
  {
    if(this->overrideSettingsToPreventError)
    {
      #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "startWifiAP", "Override setting - enable WiFi before starting WiFi AP!", 1);
      #endif
      enableWiFi();
    }
    else
    {
      #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "startWifiAP", "Can't start WiFi AP - WiFi is not enabled - Overriding is disabled!", 2);
        logging.SFLog(className, "startWifiAP", "Report to ErrorHandler!", 2);
      #endif
      error.error = true;
      error.ErrorCode = 0;
      error.message = "WiFi disabled - overriding disabled!";
      error.priority = 4;
      setLockClass(true); //lock Class to prevent further Errors in this Class - to prevent this "Error" - just keeo overrideSettingsToPreventError as true
      return false;
    }
    
    
  }
  
  if(WiFi.softAP(ssid, passwd, channel, hidden))
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "startWifiAP", "AP successfully enabled!");
    #endif
    apActive = true;
    return true;
  }
  else
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "startWifiAP", "Can't start WiFi AP - softAP return 'false'", 2);
    #endif
    error.error = true;
    error.ErrorCode = 1;
    error.message = "WiFi Class -> softAP return false";
    error.priority = 4;
    setLockClass(true); //lock Class to prevent further Errors in this Class - to prevent this "Error" - just keeo overrideSettingsToPreventError as true
    return false;
  }
}

bool WiFiManager::stopWifiAP(bool wifioff) //if wifiOff = true - softAP mode will stopped otherwise softap creds only set to NULL
{
  if(!apActive)
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "stopWifiAP", "AP already disabled - SKIP", 1);
    #endif
  }


  if(WiFi.softAPdisconnect(wifioff))
  {
    apActive = false;
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "stopWifiAP", "AP successfully disabled");
    #endif
    return true;
  }
  else
  {
    if(overrideSettingsToPreventError)
    {
      #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "stopWifiAP", "Can't disable AP - Try to Fix Problem - invert wifiOff value", 1);
      #endif

      if(WiFi.softAPdisconnect(!wifioff))
      {
        #ifdef J54J6_LOGGING_H
          logger logging;
          logging.SFLog(className, "stopWifiAP", "AP successfully disabled - with inverted settings! - AutoFix used!");
        #endif
        return true;
      }
      else
      {
        #ifdef J54J6_LOGGING_H
          logger logging;
          logging.SFLog(className, "stopWifiAP", "Can't disable AP - Report to ErrorHandler", 1);
        #endif
        error.error = true;
        error.ErrorCode = 3;
        error.message = "Cann't disable AP - softAPDisc. returns false - with inverted settings too - AutoFix failed!";
        error.priority = 1;
        return false;
      }
      
    }
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "stopWifiAP", "Can't disable AP - Report to ErrorHandler", 1);
    #endif
    error.error = true;
    error.ErrorCode = 4;
    error.message = "Cann't disable AP - softAPDisc. returns false";
    error.priority = 1;
    return false;
  }
  return false; //normaly this can't be reached - only for better syntax ^^
}

bool WiFiManager::configWiFiAP(IPAddress localIp, IPAddress gateway, IPAddress subnet)
{
  return WiFi.softAPConfig(localIp, gateway, subnet);
}

uint8_t WiFiManager::getConnectedStations()
{
  if(!apActive)
  {
    /*
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "getConnectedStations", "AP is disabled - return 0", 1);
    #endif
    */
    return 0;
  }
  /*
  #ifdef J54J6_LOGGING_H
      logger logging;
      String message = "Return connected Stations: ";
      message += WiFi.softAPgetStationNum();
      message += " devices.";
      logging.SFLog(className, "getConnectedStations", message.c_str());
    #endif
  */
  return WiFi.softAPgetStationNum();
}

/*
  Station Stuff
*/

bool WiFiManager::startWifiStation(const char* ssid, const char* passwd,  WiFiMode_t mode, bool restart, int32_t channel, const uint8_t *bssid, bool connect)
{
  #ifdef J54J6_LOGGING_H
    logger logging;
    logging.SFLog(className, "startWifiStation", "Start Wifi Station");
  #endif

  if(staActive)
  {
    if(!restart)
    {
      #ifdef J54J6_LOGGING_H
        logging.SFLog(className, "startWifiStation", "Station already enabled - SKIP", 1);
      #endif
      return true;
    }
    else
    {
      #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "startWifiStation", "Station Already enabled - Restart!", 1);
      #endif
      delay(50);
    }
    
  }

  if(!this->shieldState) //if wifiIsDisabled and overrideSettings is enabled - enableWifi and then start WifiAP
  {
    if(this->overrideSettingsToPreventError)
    {
      #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "startWifiStation", "Override setting - enable WiFi before starting WiFi Station!", 1);
      #endif
      enableWiFi();
    }
    else
    {
      #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "startWifiStation", "Can't start WiFi Station - WiFi is not enabled - Overriding is disabled!", 2);
        logging.SFLog(className, "startWifiStation", "Report to ErrorHandler!", 2);
      #endif
      error.error = true;
      error.ErrorCode = 0;
      error.message = "WiFi disabled - overriding disabled!";
      error.priority = 5;
      setLockClass(true); //lock Class to prevent further Errors in this Class - to prevent this "Error" - just keeo overrideSettingsToPreventError as true
      return false;
    }
    
    
  }
  
  if(WiFi.mode(mode))
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "startWifiStation", "Station successfully enabled - try to connect...");
    #endif
  }
  else
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "startWifiStation", "Can't enable WiFi Station - Wifi.mode(sta) return 'false'", 2);
    #endif
    error.error = true;
    error.ErrorCode = 1;
    error.message = "WiFi Class ->  Wifi.mode() return false";
    error.priority = 6;
    setLockClass(true); //lock Class to prevent further Errors in this Class - to prevent this "Error" - just keeo overrideSettingsToPreventError as true
    return false;
  }

  #ifdef J54J6_LOGGING_H
    logging.SFLog(className, "startWifiStation", "WiFi Station successfully started");
  #endif
  staActive = true;
  WiFi.begin(ssid, passwd, channel, bssid, connect);
  return true;
}

bool WiFiManager::stopWifiStation(bool wifioff)
{
  if(!staActive)
  {
    #ifdef J54J6_LOGGING_H
    logger logging;
    logging.SFLog(className, "stopWifiStation", "WiFi Station not enabled - SKIP", 1);
  #endif
  return true;
  }


  if(WiFi.disconnect(wifioff))
  {
    staActive = false;
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "stopWifiStation", "Station successfully disabled");
    #endif
    return true;
  }
  else
  {
    if(overrideSettingsToPreventError)
    {
      #ifdef J54J6_LOGGING_H
        logger logging;
        logging.SFLog(className, "stopWifiStation", "Can't disable Station - Try to Fix Problem - invert wifiOff value", 1);
      #endif

      if(WiFi.disconnect(!wifioff))
      {
        #ifdef J54J6_LOGGING_H
          logger logging;
          logging.SFLog(className, "stopWifiStation", "Station successfully disabled - with inverted settings! - AutoFix used!");
        #endif
        return true;
      }
      else
      {
        #ifdef J54J6_LOGGING_H
          logger logging;
          logging.SFLog(className, "stopWifiStation", "Can't disable Station - Report to ErrorHandler", 1);
        #endif
        error.error = true;
        error.ErrorCode = 7;
        error.message = "Cann't disable Station - AP.Disc. returns false - with inverted settings too - AutoFix failed!";
        error.priority = 1;
        return false;
      }
      
    }
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "stopWifiStation", "Can't disable Station - Report to ErrorHandler", 1);
    #endif
    error.error = true;
    error.ErrorCode = 8;
    error.message = "Cann't disable Station - AP.Disc. returns false";
    error.priority = 1;
    return false;
  }
  return false; //normaly this can't be reached - only for better syntax ^^
}

bool WiFiManager::wifiSTAIsConnected()
{
  if(WiFi.status() == WL_CONNECTED)
   {
    return true;
  }
  else
  {
    return false;
  }
}

bool WiFiManager::wifiAPUserConnected()
{
  if(WiFi.softAPgetStationNum() > 0)
  {
    return true;
  }
  else
  {
    return false;
  }
}

void WiFiManager::setOpticalMessage(wl_status_t currentState)
{
  if(!this->shieldState)
  {
    wifiLed->ledOff();
    return;
  }
  if(!apActive && !staActive)
  {
    wifiLed->blink(100);
    return;
  }
  else if(apActive && !staActive)
  {
    if(getConnectedStations() > 0)
    {
      wifiLed->ledOn();
      return;
    }
    else
    {
      wifiLed->blink(500);
      return;
    }
  }
  else if(staActive && !apActive)
  {
    if(currentState == WL_NO_SHIELD)
    {
      error.error = true;
      error.message = "No WiFI Shield found!";
      error.priority = 6;
      wifiLed->ledOff();
      return;
    }
    else if(currentState == WL_IDLE_STATUS)
    {
      wifiLed->blink(250);
      return;
    }
    else if(currentState == WL_NO_SSID_AVAIL)
    {
      wifiLed->blink(500);
      return;
    }
    else if(currentState == WL_CONNECTED)
    {
      wifiLed->ledOn();
      return;
    }
    else if(currentState == WL_CONNECT_FAILED)
    {
      wifiLed->blink(1500);
      return;
    }
    else if(currentState == WL_CONNECTION_LOST)
    {
      wifiLed->blink(500);
    }
    else if(currentState == WL_DISCONNECTED)
    {
      wifiLed->blink(500);
    }
  }
  else
  {
    if(getConnectedStations() > 0 && currentState == WL_CONNECTED)
    {
      wifiLed->ledOn();
      return;
    }
    else if(getConnectedStations() > 0 && currentState != WL_CONNECTED)
    {
      wifiLed->blink(2000);
      return;
    }
    else if(currentState == WL_CONNECTED && getConnectedStations() <= 0)
    {
      wifiLed->blink(4000);
      return;
    }
    else
    {
      wifiLed->blink(750);
      return;
    }
    
  }
  
}


/*
  RUN
*/
void WiFiManager::run()
{
  if(millis() >= (lastCall + checkDelay) && !lockClass)
  {
    internalControl();
    setOpticalMessage(this->getWiFiState());
    lastCall = millis();
  }
}


/*
  Internal Class functions
*/
void WiFiManager::internalControl()
{
  if(!shieldState && (apActive || staActive))
  {
    #ifdef J54J6_LOGGING_H
      logger logging;
      logging.SFLog(className, "checkVarIntegrity", "Undefined State! - Check Code! - shieldState was false but Station or AP is active -> Fixed -> Report to ErrorHandler", 1);
    #endif
    error.ErrorCode = 104;
    error.message = "undefied Class State reported! - state fixed";
    error.priority = 1;
    shieldState = true;
  }

  callPerSecond = 1000/(millis() - lastCall);
}


/*
  inherited ErrorSlave
*/

void WiFiManager::startClass()
{
  this->setLockClass(false);
  this->run();
}

void WiFiManager::stopClass()
{
  this->setLockClass(true);
}

void WiFiManager::pauseClass()
{
  this->stopClass();
}

void WiFiManager::restartClass()
{
  this->startClass();
}

void WiFiManager::continueClass()
{
  this->startClass();
}

