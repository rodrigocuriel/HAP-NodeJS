// MQTT Setup
var mqtt = require('mqtt');
console.log("Connecting to MQTT broker...");
var options = {
    port: 1883,
    host: 'curiel.me',
    clientId: 'homeKit'
};
var client = mqtt.connect(options);
console.log("moodLight Connected to MQTT broker");

var Accessory = require('../').Accessory;
var Service = require('../').Service;
var Characteristic = require('../').Characteristic;
var uuid = require('../').uuid;

// here's a fake hardware device that we'll expose to HomeKit
var MOOD_LIGHT = {
    powerOn: false,
    brightness: 100, // percentage
    hue: 0,
    saturation: 0,

    setPowerOn: function(on) {
        console.log("Turning moodLight %s!", on ? "on" : "off");

        if (on) {
            client.publish('moodLightPower', 'on');
            MOOD_LIGHT.powerOn = on;
        }
        else {
            client.publish('moodLightPower','off');
            MOOD_LIGHT.powerOn = false;
        }

    },
    setBrightness: function(brightness) {
        console.log("Setting light brightness to %s", brightness);
        client.publish('moodLightBrightness',String(brightness));
        MOOD_LIGHT.brightness = brightness;
    },
    setHue: function(hue){
        console.log("Setting light Hue to %s", hue);
        client.publish('moodLightHue',String(hue));
        MOOD_LIGHT.hue = hue;
    },
    setSaturation: function(saturation){
        console.log("Setting light Saturation to %s", saturation);
        client.publish('moodLightSaturation',String(saturation));
        MOOD_LIGHT.saturation = saturation;
    },
    identify: function() {
        console.log("Identify the light!");
    }
};

// Generate a consistent UUID for our light Accessory that will remain the same even when
// restarting our server. We use the `uuid.generate` helper function to create a deterministic
// UUID based on an arbitrary "namespace" and the word "moodLight".
var lightUUID = uuid.generate('hap-nodejs:accessories:moodLight');

// This is the Accessory that we'll return to HAP-NodeJS that represents our fake light.
var light = exports.accessory = new Accessory('light', lightUUID);

// Add properties for publishing (in case we're using Core.js and not BridgedCore.js)
light.username = "1A:2B:3C:5D:6E:F3";
light.pincode = "031-45-154";

// set some basic properties (these values are arbitrary and setting them is optional)
light
    .getService(Service.AccessoryInformation)
    .setCharacteristic(Characteristic.Manufacturer, "Curiel")
    .setCharacteristic(Characteristic.Model, "Rev-1")
    .setCharacteristic(Characteristic.SerialNumber, "201604");

// listen for the "identify" event for this Accessory
light.on('identify', function(paired, callback) {
    MOOD_LIGHT.identify();
    callback(); // success
});

// Add the actual Lightbulb Service and listen for change events from iOS.
// We can see the complete list of Services and Characteristics in `lib/gen/HomeKitTypes.js`
light
    .addService(Service.Lightbulb, "Plant Light") // services exposed to the user should have "names" like "Fake Light" for us
    .getCharacteristic(Characteristic.On)
    .on('set', function(value, callback) {
        MOOD_LIGHT.setPowerOn(value);
        callback(); // Our fake Light is synchronous - this value has been successfully set
    });

// We want to intercept requests for our current power state so we can query the hardware itself instead of
// allowing HAP-NodeJS to return the cached Characteristic.value.
light
    .getService(Service.Lightbulb)
    .getCharacteristic(Characteristic.On)
    .on('get', function(callback) {

        // this event is emitted when you ask Siri directly whether your light is on or not. you might query
        // the light hardware itself to find this out, then call the callback. But if you take longer than a
        // few seconds to respond, Siri will give up.

        var err = null; // in case there were any problems

        if (MOOD_LIGHT.powerOn) {
            console.log("Are we on? Yes.");
            callback(err, true);
        }
        else {
            console.log("Are we on? No.");
            callback(err, false);
        }
    });

// also add an "optional" Characteristic for Brightness
light
    .getService(Service.Lightbulb)
    .addCharacteristic(Characteristic.Brightness)
    .on('get', function(callback) {
        callback(null, MOOD_LIGHT.brightness);
    })
    .on('set', function(value, callback) {
        MOOD_LIGHT.setBrightness(value);
        callback();
    });

light
    .getService(Service.Lightbulb)
    .addCharacteristic(Characteristic.Hue)
    .on('get',function(callback){
        callback(null,MOOD_LIGHT.hue);
    })
    .on('set',function(value,callback){
        MOOD_LIGHT.setHue(value);
        callback();
    });

light
    .getService(Service.Lightbulb)
    .addCharacteristic(Characteristic.Saturation)
    .on('get',function(callback){
        callback(null,MOOD_LIGHT.saturation);
    })
    .on('set',function(value,callback){
        MOOD_LIGHT.setSaturation(value);
        callback();
    });
