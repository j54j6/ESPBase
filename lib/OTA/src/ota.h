#ifndef J54J6_OTA
#define J54J6_OTA

class OTA_Manager : public ErrorSlave
{
    private:
        const char* configFile = "/config/ota/ota.json";
};
#endif