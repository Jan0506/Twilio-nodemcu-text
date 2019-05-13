

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "twilio.hpp"

// Use software serial for debugging?
#define USE_SOFTWARE_SERIAL 1

//SETUP
const char* ssid = "Bethurem_WiFi";
const char* password = "deadbeef12345678";
const char* fingerprint = "4C:FE:00:19:F3:57:89:7B:02:9A:A4:AA:2E:7B:CB:6E:53:D1:7E:AE";
const char* account_sid = "AC289617dc2b7c931662add2d9fd18beda";
const char* auth_token = "54317ea33003f4cfa1d9aeed768dfb9c";
String to_number    = "+17204750620";
String from_number = "+17207073716";
String message_body    = "Your washer/dryer is done!";
String master_number    = "+17204750620";
String media_url = "";
//


// Global twilio objects
Twilio *twilio;
ESP8266WebServer twilio_server(8000);

//  Optional software serial debugging
#if USE_SOFTWARE_SERIAL == 1
#include <SoftwareSerial.h>
// You'll need to set pin numbers to match your setup if you
// do use Software Serial

//extern SoftwareSerial swSer(14, 4, false, 256);

#endif

/*
 * Callback function when we hit the /message route with a webhook.
 * Use the global 'twilio_server' object to respond.
 */
 void handle_message() {
        #if USE_SOFTWARE_SERIAL == 1
        Serial.println("Incoming connection!  Printing body:");
        #endif
        bool authorized = false;
        char command = '\0';

        // Parse Twilio's request to the ESP
        for (int i = 0; i < twilio_server.args(); ++i) {
                #if USE_SOFTWARE_SERIAL == 1
                Serial.print(twilio_server.argName(i));
                Serial.print(": ");
                Serial.println(twilio_server.arg(i));
                #endif

                if (twilio_server.argName(i) == "From" and 
                    twilio_server.arg(i) == master_number) {
                    authorized = true;
                } else if (twilio_server.argName(i) == "Body") {
                        if (twilio_server.arg(i) == "?" or
                            twilio_server.arg(i) == "0" or
                            twilio_server.arg(i) == "1") {
                                command = twilio_server.arg(i)[0];
                        }
                }
        } // end for loop parsing Twilio's request

        // Logic to handle the incoming SMS
        String response = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
        if (command != '\0') {
                if (authorized) {
                        switch (command) {
                        case '0':
                                digitalWrite(LED_BUILTIN, LOW);
                                response += "<Response><Message>"
                                "Turning light off!"
                                "</Message></Response>";
                                break;
                        case '1':
                                digitalWrite(LED_BUILTIN, HIGH);
                                response += "<Response><Message>"
                                "Turning light on!"
                                "</Message></Response>";
                                break;
                        case '?':
                        default:
                                response += "<Response><Message>"
                                "0 - Light off, 1 - Light On, "
                                "? - Help\n"
                                "The light is currently: ";
                                response += digitalRead(LED_BUILTIN);
                                response += "</Message></Response>";
                                break;               
                        }
                } else {
                        response += "<Response><Message>"
                        "Unauthorized!"
                        "</Message></Response>";
                }

        } else {
                response += "<Response><Message>"
                "Look: a SMS response from an ESP8266!"
                "</Message></Response>";
        }

        twilio_server.send(200, "application/xml", response);
}





/*
 * Setup function for ESP8266 Twilio Example.
 * 
 * Here we connect to a friendly wireless network, instantiate our twilio 
 * object, optionally set up software serial, then send a SMS or MMS message.
 */
void setup() {
        WiFi.begin(ssid, password);
        twilio = new Twilio(account_sid, auth_token, fingerprint);

        #if USE_SOFTWARE_SERIAL == 1
        Serial.begin(115200);
        while (WiFi.status() != WL_CONNECTED) {
                delay(1000);
                Serial.print(".");
        }
        Serial.println("");
        Serial.println("Connected to WiFi, IP address: ");
        Serial.println(WiFi.localIP());
        #else
        while (WiFi.status() != WL_CONNECTED) delay(1000);
        #endif

        // Response will be filled with connection info and Twilio API responses
        // from this initial SMS send.
        String response;
        bool success = twilio->send_message(
                to_number,
                from_number,
                message_body,
                response,
                media_url
                );

        // Set up a route to /message which will be the webhook url
        twilio_server.on("/message", handle_message);
        twilio_server.begin();

        // Use LED_BUILTIN to find the LED pin and set the GPIO to output
        pinMode(LED_BUILTIN, OUTPUT);

        #if USE_SOFTWARE_SERIAL == 1
        Serial.println(response);
        #endif
}


/* 
 *  In our main loop, we listen for connections from Twilio in handleClient().
 */
void loop() {
        twilio_server.handleClient();
}
