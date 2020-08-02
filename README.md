# ESPBase
Current dev in dev_linearWeb!

A little Sketch to Handle all common things for you like Network (WiFi, DNS, MDNS, Captive Portals, Filehandling, config creation (as Json), Webserver, MQTT)

Note: This is my first "complete Project" - I'm a newbie ^^ (2020) - im not finished until now but i work every day on this project

If you find any bugs please report it, i'll fix it as fast as possible... This is my private project in a very early state, i might change some functionalities but i try to add and change all features without changing the general function, you dont need any changes if i update my Baseprogram, only new functionalities or parameters will change. Only if there is no other way than a complete change e.g security fixes i will change complete interfaces but in this case i will write it in README and CahngeLog

postponed work:

    -> switch from linear Webserver to Async Webserver (branch: dev)
    
Current work:

    -> Finish WiFi Setup and DNS/MDNS Handling
    -> Add MQTT Handling
    -> Wrap current global sketch content into single Class


bucket list:

    device: Encrypt all config files on ESP LittleFS Storage to prevent physical readout
    simple updater to update the sketch without removing hookup sketch
    mini "sketch store" for simple devices e.g mqtt thermometers/you tell me...
    communitystore integration
    launch blog website nodework.de
    Network: captive Portal for easy SignUp
    Network: MDNS Support
    Network: DNS Support
    Network: DNS Homeintegration
    Network: WPS Support
    Network: Dynamic LED Support
    Network: PROGMEM Registration Website
    Network: Dynamic WiFi Handling and auto disable to save energy
    Network: MQTT Support (Broker and Client)
    Network: Custom per User OTA with SSL and Key verification
    Network: ESP NOW communication
    General: Encapsulating ESPBase from hook up functionality
    General: Power saving
    General: smart Deep Sleep
    General: Modular Website Interface to set up Device and hok up functionality
    General: Get this project working ^^
    General: Assistant compatibility
    General: Mod store for this system (e.g for LED control or thermometer)
    Button: detect push sequences (eg. long - short -long - short)
    General: Create a wrapper class for hook up functionality

what's working:

    Filemanager
        create and read Json Strings or Objects
        Save stuff to SPIFFS (char, String, Json)
        Save pretty Json from unformatted String or Json
        delete Files
        create Files
        Rename Files

    LED
        Create LED Object(s)
        Control LED (blink, static, disable, enable)

    Button
        create Button Object(s)
        detect long and short Clicks and Amount (set max. val. in defines.h) (only amount not sequence!
        limit max push
        setup time for long and short click and max delay

    Network:
        wifi control (connect/disconnect/disable)
        config creation
        AP start
        Captive Portal (register and saving to LittleFS (using Filemanager))

