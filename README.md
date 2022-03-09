# ESPBase
[![CodeFactor](https://www.codefactor.io/repository/github/j54j6/espbase/badge?s=7e40bd2d9cb851587b67c77431662160073df281)](https://www.codefactor.io/repository/github/j54j6/espbase)



IMPORTANT: To get this program working you need to pre flash the files saved in Data/* to your ESP with LittleFS Flasher != SPIFFS Flash !!!!

Current dev in dev_linearWeb!

A little Sketch to Handle all common things for you like Network (WiFi, DNS, MDNS, Captive Portals, Filehandling, config creation (as Json), Webserver, MQTT)

If you find any bugs please report it, i'll fix it as fast as possible... This is my private project in a very early state, i might change some functionalities but i try to add and change all features without changing the general function, you dont need any changes if i update my Baseprogram, only new functionalities or parameters will change. Only if there is no other way than a complete change e.g security fixes i will change complete interfaces but in this case i will write it in README and ChangeLog

priority work:

    -> Add @PJON to this project as NetworkBase

postponed work:

    -> switch from linear Webserver to Async Webserver (branch: dev)
    
Current work:

    -> Create Modules for ESPOS

bucket list:

    device: Encrypt all config files on ESP LittleFS Storage to prevent physical readout
    simple updater to update the sketch without removing hookup sketch
    mini "sketch store" for simple devices e.g mqtt thermometers/...
    launch blog website nodework.de
    Network: add SSDP
    Network: DNS Homeintegration
    Network: WPS Support
    Network: Dynamic LED Support
    Network: Dynamic WiFi Handling and auto disable to save energy
    Network: Custom per User OTA with SSL and Key verification
    Network: ESP NOW communication
    General: Power saving
    General: smart Deep Sleep

    
    General: Assistant compatibility
    General: Mod store for this system (e.g for LED control or thermometer)
    Button: detect push sequences (eg. long - short -long - short)

what's working:
    General:
        Modular Website Interface to set up Device and hook up functionality
        
    Filemanager
        create and read Json Strings or Objects
        create Config FIles and edit them
        Save stuff to LittleFS (char, String, Json, whatever)
        Save pretty Json from unformatted String or Json
        delete Files
        create Files
        Rename Files
        all a filemanager need to do

    LED
        Create LED Object(s)
        Control LED (blink, static, disable, enable)

    Button
        create Button Object(s)
        detect long and short Clicks and Amount (set max. val. in defines.h) (only amount not sequence!
        limit max push
        setup time for long and short click and max delay

    Network:
        MQTT Support (Client)
        wifi control (connect/disconnect/disable)
        config creation
        AP start
        Captive Portal (register and saving to LittleFS (using Filemanager))
        Webserver + custom service add
        ServiceSearch (NetworkIdent by j54j6) - later as JServiceManager with SSDP, UPnP and all common discovery protocols
        udp Communication
        Network: captive Portal for easy SignUp
        Network: MDNS Support
        Network: DNS Support
        Send Data to other devices (like MQTT Address)
        Administration Panel (OTA and Monitoring - Administration Server currently not posted on GitHub Pulbic)
    

