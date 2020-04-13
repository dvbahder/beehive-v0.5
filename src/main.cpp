/*******************************************************************************
 * Ver. 0.5
 * Date 08-03-2020
 * Application ID beehive-001-dvb
 * Device ID beehive01
 * - die LEDs dienen nur der Anzeige im Test, sie sollten entweder ausgeschaltet oder garnicht verbaut werden
 * 
 * ToDo
 * - verwendete Wägezelle eichen, damit beim Einschalten des Nodes sofort das Gewicht (ohne Tarieren) angezeigt wird
 * - DeepSleep einbauen
 * 
 *******************************************************************************/
#include <Arduino.h>
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include <Wire.h>
#include <math.h>

//--- define ---
#define WEMOS_LORA_GW

//---NeoPixels---
#include <NeoPixelBus.h>
#include <NeoPixelAnimator.h>
#include <RGBLed.cpp>
#include <RGBLed.h>
// WS2812 an D3 GPIO 0

//---Testdaten---
int test = 0; 
int32_t batt = 5;
float V1 = 5.0;
unsigned int raw = 5;

//---Konfiguration HX711----
#include "HX711.h"
const int LOADCELL_DOUT_PIN = 4;     // Waage DOUT D2 GPIO4
const int LOADCELL_SCK_PIN = 5;      // Waage SCK D1 GPIO5
HX711 scale;
boolean HX711_OK;
int i = 0;
int32_t weight = 1;
float W1 = 1.0;

//---Konfiguration DS18B20------------
#include <OneWire.h>
#include <DallasTemperature.h> 
#define ONE_WIRE_BUS 2 //PIN D4 GPIO 2
#define Sensor_Aufloesung 12
DeviceAddress Sensor_Adressen;
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
byte Anzahl_Sensoren_DS18B20 = 1; 
int32_t temp1 = 1;
int32_t temp2 = 1;
float T1 = 23.5;
float T2 = 24.5;
float Temperatur[2] = {999.99,999.99};

//---Konfiguration TTN------------
static const PROGMEM u1_t NWKSKEY[16] = { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F } ;
static const u1_t PROGMEM APPSKEY[16] = { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F } ;
static const u4_t DEVADDR = 0x261FFFFF;
// 
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static uint16_t packetNumber = 0;
static osjob_t sendjob;
const unsigned TX_INTERVAL = 30; //Standard 60sek
int ln; // Paketlänge

// --- Batterie Voltage ---
void SensorBatt(){
    raw = analogRead(A0);
    V1 = (raw/1023.0)*3.3;
}

// --- Temperatur DS18B20 ---
void SensorDS18B20() {
  if ((Anzahl_Sensoren_DS18B20 > 0) and (Anzahl_Sensoren_DS18B20 < 3)) {
    sensors.requestTemperatures();
    Serial.print("Messung...Anzahl Devices...");
    Serial.println(sensors.getDeviceCount(),DEC);
    for (byte i = 0 ; i < Anzahl_Sensoren_DS18B20; i++) {
      if (i < sensors.getDeviceCount()) {
        for (byte j = 0 ; j < 3; j++) {
          Temperatur[i] = sensors.getTempCByIndex(i);  
          delay(50);
          if ((Temperatur[i] < 60) and (Temperatur[i] > -40)) j = 10; // Werte fur Fehlererkennung
          else {
            Temperatur[i] = 999.99;
          } // else end
        } // for end
      }
    }
    T1 = Temperatur[0]; // Hier kann die Zuordnung der Sensoren geandert werden, siehe DHT
    if (Anzahl_Sensoren_DS18B20 == 2) T2 = Temperatur[1];  // Hier kann die Zuordnung der Sensoren geandert werden, if Schleife dafur bauen
  }
}

// --- LoadCell auslesen ---
void ScaleWeight(){
    W1 = (scale.get_units(5));
}

// --- Pin mapping RF95
const lmic_pinmap lmic_pins = {
    .nss = 16,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = LMIC_UNUSED_PIN,
    .dio = {15, 15, LMIC_UNUSED_PIN},
};

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    } else {
        // --- Testdaten ---
        test = 123456;
        ScaleWeight();
        weight = W1*100;
        SensorBatt();
        batt = V1*100;
        SensorDS18B20();
        temp1 = T1*100;
        //temp2 = T2*100;
        
        byte payload[10];
        payload[0] = test;
        payload[1] = test >> 8;        
        payload[2] = test >> 16;  // todo Stelle kürzen
        payload[3] = weight;
        payload[4] = weight >> 8;        
        payload[5] = weight >> 16;  // todo Stelle kürzen
        payload[6] = batt;
        payload[7] = batt >> 8;
        payload[8] = temp1;
        payload[9] = temp1 >> 8;                  
        
        //// Prepare upstream data transmission at the next possible time.
        // Daten senden
        LMIC_setTxData2(1, (uint8_t*)payload, sizeof(payload), 0);
        packetNumber++;
        Serial.println(F("Packet queued"));
        LedRGBON (50, 2, true);
        delay (50);
    }
    
    // Next TX is scheduled after TX_COMPLETE event.
}

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));   
            if (LMIC.dataLen) {
              Serial.println(F("Received "));
              Serial.println(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            // Schedule next transmission
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void setup() {
    // init packet counter
    // sprintf(mydata, "Packet = %5u", packetNumber);

    Serial.begin(115200);
    pinMode(A0, INPUT);
    
    Serial.println(" ");
    Serial.println("Starting.");
    Serial.println("LoRa-Node Platine-Test");
    rgb_led.Begin();
    LedRGBOFF();

    Serial.println("HX711 Demo - Initializing the scale");
    scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    scale.set_gain();
    //scale.set_scale(1.0); 
    scale.set_scale(22.561181);          // this value is obtained by calibrating the scale with known weights; see the README for details
    delay(50);
    scale.tare();                  // reset the scale to 0
    delay(50);

    // --- Setup DS18B20 - Temperatur ---
    Serial.println("Initialisiere DS18B20...");
    if ((Anzahl_Sensoren_DS18B20 > 0) and (Anzahl_Sensoren_DS18B20 < 3)) {
        sensors.begin();
        for (byte i = 0 ; i < sensors.getDeviceCount(); i++) {
            if (sensors.getAddress(Sensor_Adressen, i)) {
                sensors.setResolution(Sensor_Adressen, Sensor_Aufloesung);
            }
        }
    } 
    delay(10);

    // --- LMIC init ---
    os_init();
    LMIC_reset();

    #ifdef PROGMEM
        uint8_t appskey[sizeof(APPSKEY)];
        uint8_t nwkskey[sizeof(NWKSKEY)];
        memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
        memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
        LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    #else
        // If not running an AVR with PROGMEM, just use the arrays directly
        LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
    #endif

    #if defined(CFG_eu868)
    // Set up the channels used by the Things Network
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
    #elif defined(CFG_us915)
    LMIC_selectSubBand(1);
    #endif
    // Disable link check validation
    LMIC_setLinkCheckMode(0);
    // TTN uses SF9 for its RX2 window.
    LMIC.dn2Dr = DR_SF9;
    // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    // Start job
    do_send(&sendjob);
}

void loop() {       
    
    if (LMIC.dataLen) {
        ln = LMIC.dataLen;

        Serial.print("len: ");
        Serial.println(ln);
        LedRGBON (20, 1, true);
    }

    Serial.print("Testdaten: ");
    Serial.println(test);
    Serial.print("Gewicht: ");
    Serial.print(W1);
    Serial.println(" g");
    Serial.print("Temp1: ");
    Serial.println(T1);
    Serial.print("Batterie: ");
    Serial.println(V1);

    delay(50);
    LedRGBOFF();
    delay(10000);
    //delay(100000);
    
    os_runloop_once();
}