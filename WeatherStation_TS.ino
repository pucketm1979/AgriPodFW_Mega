#include <DHT.h>
#include <stdlib.h>

// F/W Version
const String fwversion = "1.02.01";

/* Version Details
  1.02.01 - Added Version Derails
            Added more detailed comments
            Changed Delay to 60 second samples to try to address wifi dropping off
            Added fwversion string
            Added Boot/Welcome Banner
*/

// LED 
int ledPin = 13;

// DHT11 analog output to Arduino analog A8
int lm35Pin = A8;

//Photocell sensor output pin to Arduino analog A9 pin
const int photoPin = A9;

// Cycle Counter
int currentCycleCount;

// Configure the thingspeak API key for later
String apiKey = "63MS08JB7M6YMF1S";

//Configure the DHT for reading later
#define DHTPIN A8
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// this runs once
void setup() {                
  
  // initialize the digital pin as an output.
  pinMode(ledPin, OUTPUT);    

  // enable debug Serial0
  Serial.begin(115200); 
  // enable hardware Serial2
  Serial2.begin(115200);
  // enable DHT Sensor (Temp & Humidity)
  dht.begin();
  currentCycleCount = 0;
  welcomeBanner();
    
}


// the loop 
void loop() {
  //Set CycleCount
  currentCycleCount = currentCycleCount+1;
  Serial.print("Current_Cycle_Count: ");
  Serial.println(currentCycleCount);
  if (currentCycleCount >= 100){softReset();currentCycleCount = 0;}

  // blink LED on board
  digitalWrite(ledPin, HIGH);   
  delay(200);               
  digitalWrite(ledPin, LOW);

  // Grab the Sensor Readings
  float temp = dht.readTemperature(true);
  float humidity = dht.readHumidity();
  //int strGas = analogRead(gasPin);
    
  // convert to string
  char buf[16];
  String strTemp = dtostrf(temp, 4, 1, buf);
  String strHumidity = dtostrf(humidity, 4, 1, buf);

  // print the converted output
  Serial.println("Temp_Sense: "+strTemp);
  Serial.println("Hum_Sense: "+strHumidity);
  
  // Create and Start TCP connection
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += "144.212.80.11"; // api.thingspeak.com
  cmd += "\",80";
  Serial2.println(cmd);

  //Check for errors 
  if(Serial2.find("Error")){
    Serial.println("AT+CIPSTART error");
    return;
}

  // prepare GET string
  String getStr = "GET /update?api_key=";
  getStr += apiKey;
  getStr +="&field1=";
  getStr += String(strTemp);
  getStr +="&field2=";
  getStr += String(strHumidity);
  //getStr +="&field3=";
  //getStr += String(strGas);
  getStr += "\r\n\r\n";

  // send data length
  cmd = "AT+CIPSEND=";
  cmd += String(getStr.length());
  Serial2.println(cmd);
  delay(800);

  // send the Sensor Data
  if(Serial2.find(">")){
    Serial2.print(getStr);
  }
  else{
    Serial2.println("AT+CIPCLOSE");
    // alert user
    Serial.println("AT+CIPCLOSE");
  }
  
  cycleDelay(); // Call Cycle Delay Function
}

void welcomeBanner(void){
  Serial.println("AgriPODZ Megapod Sensor Module");
  Serial.print("F/W Version: ");
  Serial.println(fwversion);
  Serial.println();
  return;
}

void cycleDelay(void){
  Serial.println("Sleeping...");
  delay(60000); // 60 Second Delay in ms
}

void softReset(void){
  Serial2.println("AT+RST"); //esp8266 Reset  
  Serial.println("Soft_Reset"); 
  cycleDelay();
}

