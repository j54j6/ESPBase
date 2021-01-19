#include "wifiManager.h"
#include "defines.h"


WiFiManager::WiFiManager(Filemanager* FM)
{
  LED voidLed(-1);
  this->wifiLed = &voidLed;
  disableWiFi();
  WiFi.persistent(false);
  lastCall = millis();
  this->logging = SysLogger(FM, "WiFiManager");
}

WiFiManager::WiFiManager(LED *newWifiLed, Filemanager* FM)
{
    this->wifiLed = newWifiLed; //Reference to WiFi LED to Control it with Handler class
    disableWiFi();
    WiFi.persistent(false);
    lastCall = millis();
    this->logging = SysLogger(FM, "WiFiManager");
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

String WiFiManager::getDeviceMac()
{
  return WiFi.macAddress();
}

int WiFiManager::getCheckDelay()
{
  return checkDelay;
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

WiFiClient& WiFiManager::getRefWiFiClient()
{
  return localWiFiClient;
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
  #ifdef J54J6_SysLogger
      
      String message = "Update ShieldState: ";
      message += shieldState;
      message += " -> ";
      message += newState;
      logging.logIt("setShieldState", message.c_str());
  #endif
  shieldState = newState;
}

void WiFiManager::setOverrideSettingsToPreventError(bool newValue)
{
  #ifdef J54J6_SysLogger
      
      String message = "Update OverrideSettingsToPreventError: ";
      message += overrideSettingsToPreventError;
      message += " -> ";
      message += newValue;
      logging.logIt("setOverrideSettingsToPreventError", message.c_str());
  #endif
  overrideSettingsToPreventError = newValue;
}

void WiFiManager::setCheckDelay(int newValue)
{
  #ifdef J54J6_SysLogger
      
      String message = "Update CheckDelay: ";
      message += checkDelay;
      message += " -> ";
      message += newValue;
      logging.logIt("setCheckDelay", message.c_str());
  #endif
  checkDelay = newValue;
}

void WiFiManager::setLockClass(bool newValue)
{
  #ifdef J54J6_SysLogger
      
      String message = "Update LockClass: ";
      message += lockClass;
      message += " -> ";
      message += newValue;

      logging.logIt("setLockClass", message.c_str());
  #endif
  if(newValue)
  {
    classControl.newReport("WiFiManager Class locked!", 214, true, true);
    #ifdef J54J6_SysLogger
     logging.logIt("setLockClass", "Report lock to ErrorHandler!", 0);
    #endif
  }
  lockClass = newValue;
}

bool WiFiManager::setWiFiAutoConnect(bool newValue)
{
  if(WiFi.setAutoConnect(newValue))
  {
    #ifdef J54J6_SysLogger
      
      String message = "Update WiFi->AutoConnect: ";
      message += getWiFiAutoConnect();
      message += " -> ";
      message += newValue;
      logging.logIt("setWiFiAutoConnect", message.c_str());
    #endif
    return true;
  }
  else
  {
    #ifdef J54J6_SysLogger
      
      String message = "Can't Update WiFi->AutoConnect: ";
      message += getWiFiAutoConnect();
      message += " -> ";
      message += newValue;
      logging.logIt("setWiFiAutoConnect", message.c_str(), 2);
    #endif
    classControl.newReport("Can't update Autoconnect!", 99, false, true);
    return false;
  }  
}

bool WiFiManager::setWiFiAutoReconnect(bool newValue)
{
  if(WiFi.setAutoReconnect(newValue))
  {
    #ifdef J54J6_SysLogger
      
      String message = "Update WiFi->AutoReconnect: ";
      message += getWiFiAutoReconnect();
      message += " -> ";
      message += newValue;
      logging.logIt("setWiFiAutoReconnect", message.c_str());
    #endif
    return true;
  }
  else
  {
    #ifdef J54J6_SysLogger
      
      String message = "Can't Update WiFi->AutoReconnect: ";
      message += getWiFiAutoReconnect();
      message +=" -> ";
      message += newValue;
      logging.logIt("setWiFiAutoReconnect", message.c_str(), 2);
    #endif
    classControl.newReport("Can't update AutoReconnect!", 93, false, true);
    return false;
  }
}

bool WiFiManager::setWiFiHostname(const char* hostname)
{
  if(hostname == WiFi.hostname().c_str())
  {
    #ifdef J54J6_SysLogger
      logging.logIt("setWiFiHostname", "Hostname already set! - SKIP", 1);
    #endif
    return true;
  }

  if(WiFi.hostname(hostname))
  {
    #ifdef J54J6_SysLogger
      
      String message = "Update WiFi->Hostname: ";
      message += WiFi.hostname();
      message += " -> ";
      message += hostname;
      logging.logIt("setWiFiHostname", message.c_str());
    #endif
    return true;
  }
  else
  {
    #ifdef J54J6_SysLogger
      
      String message = "Can't Update WiFi->Hostname: ";
      message += WiFi.hostname();
      message += " -> ";
      message += hostname;
      logging.logIt("setWiFiHostname", message.c_str(), 2);
    #endif
    classControl.newReport("Can't update Hostname!", 56, 1, false, true);
    return false;
  }
}

bool WiFiManager::setWiFiConfig(IPAddress local_ip, IPAddress gateway, IPAddress subnet, IPAddress dns1, IPAddress dns2)
{
  if(WiFi.config(local_ip, gateway, subnet, dns1, dns2))
  {
    #ifdef J54J6_SysLogger
      
      logging.logIt("setWiFiConfig", "WiFi Config successfully updated!", 0);
    #endif
    return true;
  }
  else
  {
    #ifdef J54J6_SysLogger
      
      logging.logIt("setWiFiConfig", "Can't update WiFi Config - Report!", 2);
    #endif
    classControl.newReport("Can't update WiFi Config - class return 'false'!", 432, true, true);
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
    #ifdef J54J6_SysLogger
      
      logging.logIt("enableWiFi", "enable WiFi");
    #endif
    shieldState = true; //wifi enabled
    wifi_fpm_do_wakeup();
    wifi_fpm_close();
    WiFi.mode(mode);
  }
  else
  {
    #ifdef J54J6_SysLogger
      
      logging.logIt("enableWiFi", "WiFi already enabled - SKIP", 1);
    #endif
  }
}

void WiFiManager::disableWiFi()
{
  if(shieldState)
  {
    #ifdef J54J6_SysLogger
      
      logging.logIt("disableWiFi", "disable WiFi");
    #endif
    shieldState = false; //wifi disabled
    wifi_station_disconnect();
    WiFi.mode( WIFI_OFF );
    WiFi.forceSleepBegin();
  }
  else
  {
    #ifdef J54J6_SysLogger
      
      logging.logIt("disableWiFi", "WiFi already disabled - SKIP", 1);
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
    #ifdef J54J6_SysLogger
      
      logging.logIt("startWifiAP", "AP already enabled - SKIP", 1);
    #endif
    return true;
  }

  if(!this->shieldState) //if wifiIsDisabled and overrideSettings is enabled - enableWifi and then start WifiAP
  {
    if(this->overrideSettingsToPreventError)
    {
      #ifdef J54J6_SysLogger
        
        logging.logIt("startWifiAP", "Override setting - enable WiFi before starting WiFi AP!", 1);
      #endif
      enableWiFi();
    }
    else
    {
      #ifdef J54J6_SysLogger
        
        logging.logIt("startWifiAP", "Can't start WiFi AP - WiFi is not enabled - Overriding is disabled!", 2);
        logging.logIt("startWifiAP", "Report to ErrorHandler!", 2);
      #endif

      classControl.newReport("WiFi disabled - overriding disabled!", 653, true, true);
      return false;
    } 
  }
  
  if(WiFi.softAP(ssid, passwd, channel, hidden))
  {
    #ifdef J54J6_SysLogger
      
      logging.logIt("startWifiAP", "AP successfully enabled!");
    #endif
    apActive = true;
    return true;
  }
  else
  {
    #ifdef J54J6_SysLogger
      logging.logIt("startWifiAP", "Can't start WiFi AP - softAP return 'false'", 2);
    #endif
    Serial.println(ssid);
    Serial.println(passwd);
    Serial.println(channel);
    Serial.println(hidden);
    #ifdef J54J6_SysLogger
        String message = "SSID: " + String(ssid) + " Passwd: " + String(passwd) + " Channel: " + String(channel) + " Hidden: " + String(hidden);
        logging.logIt("startWifiAP", message, 2);
    #endif

    classControl.newReport("WiFi Class -> softAP return false", 1, true, true);
    setLockClass(true); //lock Class to prevent further Errors in this Class - to prevent this "Error" - just keeo overrideSettingsToPreventError as true
    return false;
  }
}

bool WiFiManager::stopWifiAP(bool wifioff) //if wifiOff = true - softAP mode will stopped otherwise softap creds only set to NULL
{
  if(!apActive)
  {
    #ifdef J54J6_SysLogger
      
      logging.logIt("stopWifiAP", "AP already disabled - SKIP", 1);
    #endif
  }


  if(WiFi.softAPdisconnect(wifioff))
  {
    apActive = false;
    #ifdef J54J6_SysLogger
      
      logging.logIt("stopWifiAP", "AP successfully disabled");
    #endif
    return true;
  }
  else
  {
    if(overrideSettingsToPreventError)
    {
      #ifdef J54J6_SysLogger
        
        logging.logIt("stopWifiAP", "Can't disable AP - Try to Fix Problem - invert wifiOff value", 1);
      #endif

      if(WiFi.softAPdisconnect(!wifioff))
      {
        #ifdef J54J6_SysLogger
          
          logging.logIt("stopWifiAP", "AP successfully disabled - with inverted settings! - AutoFix used!");
        #endif
        return true;
      }
      else
      {
        #ifdef J54J6_SysLogger
          
          logging.logIt("stopWifiAP", "Can't disable AP - Report to ErrorHandler", 1);
        #endif
        classControl.newReport("Cann't disable AP - softAPDisc. returns false - with inverted settings too - AutoFix failed!", 3, true, true);
        return false;
      }
    }
    #ifdef J54J6_SysLogger
      
      logging.logIt("stopWifiAP", "Can't disable AP - Report to ErrorHandler", 1);
    #endif
    classControl.newReport("Can't disable AP - softAPDisc. returns false", 4, false, true);
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
    #ifdef J54J6_SysLogger
      
      logging.logIt("getConnectedStations", "AP is disabled - return 0", 1);
    #endif
    */
    return 0;
  }
  /*
  #ifdef J54J6_SysLogger
      
      String message = "Return connected Stations: ";
      message += WiFi.softAPgetStationNum();
      message += " devices.";
      logging.logIt("getConnectedStations", message.c_str());
    #endif
  */
  return WiFi.softAPgetStationNum();
}

/*
  Station Stuff
*/

bool WiFiManager::startWifiStation(const char* ssid, const char* passwd,  WiFiMode_t mode, bool restart, int32_t channel, const uint8_t *bssid, bool connect)
{
  #ifdef J54J6_SysLogger
    
    logging.logIt("startWifiStation", "Start Wifi Station");
  #endif

  if(staActive)
  {
    if(!restart)
    {
      #ifdef J54J6_SysLogger
        logging.logIt("startWifiStation", "Station already enabled - SKIP", 1);
      #endif
      return true;
    }
    else
    {
      #ifdef J54J6_SysLogger
        
        logging.logIt("startWifiStation", "Station Already enabled - Restart!", 1);
      #endif
      delay(50);
    }
  }

  if(!this->shieldState) //if wifiIsDisabled and overrideSettings is enabled - enableWifi and then start WifiAP
  {
    if(this->overrideSettingsToPreventError)
    {
      #ifdef J54J6_SysLogger
        
        logging.logIt("startWifiStation", "Override setting - enable WiFi before starting WiFi Station!", 1);
      #endif
      enableWiFi();
    }
    else
    {
      #ifdef J54J6_SysLogger
        
        logging.logIt("startWifiStation", "Can't start WiFi Station - WiFi is not enabled - Overriding is disabled!", 2);
        logging.logIt("startWifiStation", "Report to ErrorHandler!", 2);
      #endif
      classControl.newReport("WiFi disabled - overriding disabled!", 312, false, true); 
      //lock Class to prevent further Errors in this Class - to prevent this "Error" - just keeo overrideSettingsToPreventError as true
      return false;
    }
  }
  
  if(WiFi.mode(mode))
  {
    #ifdef J54J6_SysLogger
      logging.logIt("startWifiStation", "Station successfully enabled - try to connect...");
    #endif
  }
  else
  {
    #ifdef J54J6_SysLogger
      
      logging.logIt("startWifiStation", "Can't enable WiFi Station - Wifi.mode(sta) return 'false'", 2);
    #endif
    classControl.newReport("WiFi Class ->  Wifi.mode() return false", 176, true, true);
    setLockClass(true); //lock Class to prevent further Errors in this Class - to prevent this "Error" - just keeo overrideSettingsToPreventError as true
    return false;
  }

  #ifdef J54J6_SysLogger
    logging.logIt("startWifiStation", "WiFi Station successfully started");
  #endif
  staActive = true;
  WiFi.begin(ssid, passwd, channel, bssid, connect);
  return true;
}

bool WiFiManager::stopWifiStation(bool wifioff)
{
  if(!staActive)
  {
    #ifdef J54J6_SysLogger
    
    logging.logIt("stopWifiStation", "WiFi Station not enabled - SKIP", 1);
  #endif
  return true;
  }


  if(WiFi.disconnect(wifioff))
  {
    staActive = false;
    #ifdef J54J6_SysLogger
      
      logging.logIt("stopWifiStation", "Station successfully disabled");
    #endif
    return true;
  }
  else
  {
    if(overrideSettingsToPreventError)
    {
      #ifdef J54J6_SysLogger
        
        logging.logIt("stopWifiStation", "Can't disable Station - Try to Fix Problem - invert wifiOff value", 1);
      #endif

      if(WiFi.disconnect(!wifioff))
      {
        #ifdef J54J6_SysLogger
          
          logging.logIt("stopWifiStation", "Station successfully disabled - with inverted settings! - AutoFix used!");
        #endif
        return true;
      }
      else
      {
        #ifdef J54J6_SysLogger
          
          logging.logIt("stopWifiStation", "Can't disable Station - Report to ErrorHandler", 1);
        #endif
        classControl.newReport("Can't disable Station - AP.Disc. returns false - with inverted settings too - AutoFix failed!", 7,1, true, true);
        return false;
      }
    }
    #ifdef J54J6_SysLogger
      
      logging.logIt("stopWifiStation", "Can't disable Station - Report to ErrorHandler", 1);
    #endif
    classControl.newReport("Can't disable Station - AP.Disc. returns false", 8, 1, true, true);
    return false;
  }
  return false; //normaly this can't be reached - only for better syntax ^^
}

bool WiFiManager::isConnected()
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

bool WiFiManager::isAPUserConnected()
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
      classControl.newReport("No WiFI Shield found!", 45, 6, true, true);
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
    setOpticalMessage(this->getWiFiState());
    lastCall = millis();
  }
  classControl.run();
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

void WiFiManager::restartClass()
{
  this->startClass();
}

