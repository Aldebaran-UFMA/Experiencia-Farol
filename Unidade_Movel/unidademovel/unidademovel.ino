/*
 * LoRa E32-TTL-100
 * Receive fixed transmission message on channel.
 * https://www.mischianti.org/2019/12/03/lora-e32-device-for-arduino-esp32-or-esp8266-power-saving-and-sending-structured-data-part-5/
 *
 * E32-TTL-100----- Arduino UNO or esp8266
 * M0         ----- 3.3v (To config) GND (To send) 7 (To dinamically manage)
 * M1         ----- 3.3v (To config) GND (To send) 6 (To dinamically manage)
 * TX         ----- RX PIN 2 (PullUP)
 * RX         ----- TX PIN 3 (PullUP & Voltage divider)
 * AUX        ----- Not connected (5 if you connect)
 * VCC        ----- 3.3v/5v
 * GND        ----- GND
 *
 */
//#define E32_TTL_1W 
#include "Arduino.h"
#include "LoRa_E32.h"
void printHex(uint8_t num);

// ---------- esp8266 pins --------------

//LoRa_E32 e32ttl(3, 4, 2); // portas: RX TX AUX M1 M0

//SoftwareSerial ss(4,3);
//LoRa_E32 e32ttl(&ss, 10,5,6); // portas: RX TX AUX M1 M0


SoftwareSerial ss(3,4);
LoRa_E32 e32ttl(&ss, 2,5,6); // portas: RX TX AUX M1 M0



// -------------------------------------
void printParameters(struct Configuration configuration);
void printModuleInformation(struct ModuleInformation moduleInformation);
//The setup function is called once at startup of the sketch
void setup()
{
	Serial.begin(9600);
	while (!Serial) {
	    ; // wait for serial port to connect. Needed for native USB
    }
	delay(100);

	e32ttl.begin();

//	e32ttl.resetModule();
	// After set configuration comment set M0 and M1 to low
	// and reboot if you directly set HIGH M0 and M1 to program
  Serial.println("Configuração anterior:");
	ResponseStructContainer c;
	c = e32ttl.getConfiguration();
	Configuration configuration = *(Configuration*) c.data;
	printParameters(configuration);
	c.close();
  
	// ---------------------------
	Serial.println();
	
  //e32ttl.setMode(MODE_2_POWER_SAVING);
  setLoraConfig();
  Serial.println("Start listening!");

}
//Structs
typedef struct {
  float lats,longs;
  uint8_t dia, mes;
  long unsigned int ano;
  uint8_t hrs,mins,seg;  
}Gps_t;
struct Upload {
  uint8_t id_barco;
  Gps_t gps1;
  float tensao;
};
struct beacon{
  unsigned long cont, times;
  float temp;
  float pressao;
}beacon_t;

// The loop function is called in an endless loop
void loop()
{
	if (e32ttl.available()  > 1){

  uint8_t type[1];  
 // first part of structure
  ResponseContainer rs = e32ttl.receiveInitialMessage(sizeof(type));
  memcpy ( type, rs.data.c_str(), sizeof(type) );
  if(type[0]==1){
    ResponseStructContainer rsc = e32ttl.receiveMessage(sizeof(beacon));
   struct beacon farol = *(beacon*) rsc.data;

    Serial.println(String(type[0]) + "," + String(farol.cont) + "," + String(farol.times) + "," + String(farol.temp) + "," + String(farol.pressao));

    rsc.close();
    
  }else if(type[0]==0 || type[0]==2){
    ResponseStructContainer rsc = e32ttl.receiveMessage(sizeof(Upload));
    struct Upload upload = *(Upload*) rsc.data;
 
  
      Serial.println(String(type[0])+ "," + String(upload.id_barco) +  "," + String(upload.gps1.lats)+ "," + String(upload.gps1.longs) + "," + (int)upload.gps1.dia + "," + (int)upload.gps1.mes +","+ (int)upload.gps1.ano + "," + (int)upload.gps1.hrs + "," + (int)upload.gps1.mins + "," + (int) upload.gps1.seg+"," +  upload.tensao);  
    
   rsc.close();
  }else{
    Serial.println("Estrutura de dados não reconhecida!");
  }
	
}
}

void printHex(uint8_t num) {
  char hexCar[2];
  sprintf(hexCar, "%02X", num);
  Serial.print(hexCar);
}

void setLoraConfig(){
  ResponseStructContainer c;
  c = e32ttl.getConfiguration();
  // It's important get configuration pointer before all other operation
  Configuration configuration = *(Configuration*) c.data;
  Serial.println(c.status.getResponseDescription());
  Serial.println(c.status.code);
  c = e32ttl.getConfiguration();
   configuration.ADDL = 0xFF;
  configuration.ADDH = 0xFF;
  configuration.CHAN = 0x17;
  configuration.SPED.airDataRate  = AIR_DATA_RATE_000_03;
  configuration.SPED.uartBaudRate = UART_BPS_9600;
  configuration.OPTION.fixedTransmission = FT_FIXED_TRANSMISSION; //FT_TRANSPARENT_TRANSMISSION;
  configuration.OPTION.fec = FEC_0_OFF;
  configuration.OPTION.ioDriveMode = IO_D_MODE_PUSH_PULLS_PULL_UPS;
  e32ttl.setConfiguration(configuration, WRITE_CFG_PWR_DWN_SAVE);
  printParameters(configuration);
  c.close();
}


void printParameters(struct Configuration configuration) {
	Serial.println("----------------------------------------");

	Serial.print(F("HEAD : "));  Serial.print(configuration.HEAD, BIN);Serial.print(" ");Serial.print(configuration.HEAD, DEC);Serial.print(" ");Serial.println(configuration.HEAD, HEX);
	Serial.println(F(" "));
	Serial.print(F("AddH : "));  Serial.println(configuration.ADDH, DEC);
	Serial.print(F("AddL : "));  Serial.println(configuration.ADDL, DEC);
	Serial.print(F("Chan : "));  Serial.print(configuration.CHAN, DEC); Serial.print(" -> "); Serial.println(configuration.getChannelDescription());
	Serial.println(F(" "));
	Serial.print(F("SpeedParityBit     : "));  Serial.print(configuration.SPED.uartParity, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTParityDescription());
	Serial.print(F("SpeedUARTDatte  : "));  Serial.print(configuration.SPED.uartBaudRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getUARTBaudRate());
	Serial.print(F("SpeedAirDataRate   : "));  Serial.print(configuration.SPED.airDataRate, BIN);Serial.print(" -> "); Serial.println(configuration.SPED.getAirDataRate());

	Serial.print(F("OptionTrans        : "));  Serial.print(configuration.OPTION.fixedTransmission, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFixedTransmissionDescription());
	Serial.print(F("OptionPullup       : "));  Serial.print(configuration.OPTION.ioDriveMode, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getIODroveModeDescription());
	Serial.print(F("OptionWakeup       : "));  Serial.print(configuration.OPTION.wirelessWakeupTime, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getWirelessWakeUPTimeDescription());
	Serial.print(F("OptionFEC          : "));  Serial.print(configuration.OPTION.fec, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getFECDescription());
	Serial.print(F("OptionPower        : "));  Serial.print(configuration.OPTION.transmissionPower, BIN);Serial.print(" -> "); Serial.println(configuration.OPTION.getTransmissionPowerDescription());

	Serial.println("----------------------------------------");

}
void printModuleInformation(struct ModuleInformation moduleInformation) {
	Serial.println("----------------------------------------");
	Serial.print(F("HEAD BIN: "));  Serial.print(moduleInformation.HEAD, BIN);Serial.print(" ");Serial.print(moduleInformation.HEAD, DEC);Serial.print(" ");Serial.println(moduleInformation.HEAD, HEX);

	Serial.print(F("Freq.: "));  Serial.println(moduleInformation.frequency, HEX);
	Serial.print(F("Version  : "));  Serial.println(moduleInformation.version, HEX);
	Serial.print(F("Features : "));  Serial.println(moduleInformation.features, HEX);
	Serial.println("----------------------------------------");

}
