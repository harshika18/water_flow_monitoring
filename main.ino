#include "WiFi.h"
#include "HTTPClient.h"
#include "ArduinoJson.h"
#include <ThingSpeak.h>

unsigned long Channel_ID =   904502;
char API_KEY[] = "AT9U6SPCOR7S61K8";

char* wifi_ssid = "JioFi3_127A1C";             //UserName of the wifi to be connected
char* wifi_pwd = "0rn36u30sm";                // Password of the wifi to be connected
String cse_ip = "onem2m.iiit.ac.in";             // IP of the server where data is posted
unsigned long int flow_frequency;               // Measures flow sensor pulses
int l_hour;                        // Used for  Calculated litres/hour
unsigned char flowsensor = 27;              // Sensor Input
unsigned long int currentTime;
unsigned long int cloopTime;
WiFiClient  client;
  
void flow ()   // Interrupt function ,increases flow_frequency value for every tick/interrupt  
{
   flow_frequency++;
}

//------------------------------------//

String cse_port = "443";          //port number
String server = "https://"+cse_ip+":"+cse_port+"/~/in-cse/in-name/";    //URI where the data is posted 

String createCI(String server, String ae, String cnt, String val)      //function to post data in the server per every minute
{
  // some needed declerations to post data on the server
  
  HTTPClient http;
  http.begin(server + ae + "/" + cnt + "/");
  http.addHeader("X-M2M-Origin", "admin:admin");
  http.addHeader("Content-Type", "application/json;ty=4");
  http.addHeader("Content-Length", "100");
  http.addHeader("Connection", "close");
  int code = http.POST("{\"m2m:cin\": {\"cnf\": \"text/plain:0\",\"con\": "+ String(val) +"}}");
  http.end();
  delay(300);
}

void setup()
{
  
   Serial.begin(115200);                     //baud rate decleration 
   pinMode(flowsensor, INPUT);               //declaring pin mode
   digitalWrite(flowsensor, HIGH);            //declaring flowsensor pin as high
   
   attachInterrupt(flowsensor, flow, RISING);          //attaching interrupt with the flow sensor 
   sei();                                       // Enable interrupts

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  ThingSpeak.begin(client);
  delay(100);

  WiFi.begin(wifi_ssid, wifi_pwd);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
   // Serial.println("Connecting to WiFi..");
  }
}

void loop()
{
  //Gets the current time of timer in milliseconds.
  
  currentTime = millis();
  
  //checks if wifi connection is lost , it enters into while loop and returns after wifi connection is re-establised
  
  if(WiFi.status() != WL_CONNECTED){
    WiFi.begin(wifi_ssid, wifi_pwd);
    while(WiFi.status() != WL_CONNECTED){
    //Serial.print("Wifi Connection lost ..");  
    }  
  }
  
  else if(currentTime >=cloopTime+1000*60){  
    
    //upload data to server every minute.
    
  cloopTime = currentTime;  // updating cloop time to current time
  
  l_hour = ((flow_frequency  ) / (8));   //converting the sensor output to L/min by using sensor-specific caliberating value
  Serial.print(l_hour);
  ThingSpeak.setField(1, l_hour);
  flow_frequency = 0;     //updating flow_frequency to zero again

  //when single sensor gives single value
  String sensor1 = String(l_hour);
  String units = "L/min";   //units of flowfrequency

  // Make it a single string
  String sensor_string = sensor1 + " " + units;

  // Make it OneM2M complaint
  sensor_string = "\"" + sensor_string + "\""; // DO NOT CHANGE THIS LINE
  

  // Send data to OneM2M server
  createCI(server, "Team27_Water_flow_monitoring", "node_1", sensor_string);
  int response;
    if((response = ThingSpeak.writeFields(Channel_ID, API_KEY)) == 200) {
      Serial.println("Data Updated to ThingSpeak successfully!");
    }
    else {
      Serial.println("Problem Updating data for ThingSpeak!\nError code: " + String(response)); 
    }
  }
}
