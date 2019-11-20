// Load Wire library for use with TMP102 sensor
#include <Wire.h>
#define TMP102_I2C_ADDRESS 72
// Load Wi-Fi library
#include <ESP8266WiFi.h>

// Replace with your network credentials
const char* ssid     = "MySpectrumWiFi9c-2G";
const char* password = "wideship903";

// Set web server port number to 80
WiFiServer server(80);

float temperature;

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output14State = "off";
String output12State = "off";

// Assign output variables to GPIO pins
const int output14 = 14;
const int output12 = 12;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  // Initialize input for A0, connected to tmp102 sensor
  //pinMode(A0, INPUT);
  // Initialize the output variables as outputs
  pinMode(output14, OUTPUT);
  pinMode(output12, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output14, LOW);
  digitalWrite(output12, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop(){
  
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /14/on") >= 0) {
              Serial.println("GPIO 14 on");
              output14State = "on";
              digitalWrite(output14, HIGH);
            } else if (header.indexOf("GET /14/off") >= 0) {
              Serial.println("GPIO 14 off");
              output14State = "off";
              digitalWrite(output14, LOW);
            } else if (header.indexOf("GET /12/on") >= 0) {
              Serial.println("GPIO 12 on");
              output12State = "on";
              digitalWrite(output12, HIGH);
            } else if (header.indexOf("GET /12/off") >= 0) {
              Serial.println("GPIO 12 off");
              output12State = "off";
              digitalWrite(output12, LOW);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Clinton's ESP8266 Web Server</h1>");
            
            // Display current state, and ON/OFF buttons for GPIO 14  
            client.println("<p>GPIO 14 - State " + output14State + "</p>");
            // If the output14State is off, it displays the ON button       
            if (output14State=="off") {
              client.println("<p><a href=\"/14/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/14/off\"><button class=\"button button2\">OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 12  
            client.println("<p>GPIO 12 - State " + output12State + "</p>");
            // If the output12State is off, it displays the ON button       
            if (output12State=="off") {
              client.println("<p><a href=\"/12/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/12/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            client.print("<p>Current Temperature: ");
            client.print(temperature);
            client.println(" degrees Fahrenheit</p>");
            client.println("</body></html>");
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
    }
  } 
}
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
// Show temperature in browser 
     getTemp102();
     delay(5000);
}

void getTemp102()
{
  byte firstbyte, secondbyte; //these are the bytes we read from the TMP102 temperature registers
  int val; /* an int is capable of storing two bytes, this is where we "chuck" the two bytes together. */ 
  float convertedtemp; /* We then need to multiply our two bytes by a scaling factor, mentioned in the datasheet. */ 
  float correctedtemp; 
  float ninefifths;
  float convertedfahrenheit;
  float correctedfahrenheit;
 
  /* Reset the register pointer (by default it is ready to read temperatures)
You can alter it to a writeable register and alter some of the configuration - 
the sensor is capable of alerting you if the temperature is above or below a specified threshold. */
 
  Wire.beginTransmission(TMP102_I2C_ADDRESS); //Say hi to the sensor. 
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(TMP102_I2C_ADDRESS, 2);
  Wire.endTransmission();
 
 
  firstbyte      = (Wire.read()); 
  /*read the TMP102 datasheet - here we read one byte from
   each of the temperature registers on the TMP102*/
  secondbyte     = (Wire.read()); 
  /*The first byte contains the most significant bits, and 
   the second the less significant */
    val = firstbyte;
    if ((firstbyte & 0x80) > 0) {
      val |= 0x0F00;
    } 
    val <<= 4; 
 /* MSB */
    val |= (secondbyte >> 4);    
/* LSB is ORed into the second 4 bits of our byte.
Bitwise maths is a bit funky, but there's a good tutorial on the playground*/
    convertedtemp = val*0.0625;
    correctedtemp = convertedtemp - 5.0;
    ninefifths = 9.0/5.0; 
    convertedfahrenheit = ((ninefifths)*convertedtemp) + 32.0;
    correctedfahrenheit = convertedfahrenheit - (ninefifths * 5.0);
    temperature = convertedfahrenheit;
    /* See the above note on overreading */
  
  Serial.print("firstbyte is ");
  Serial.print("\t");
  Serial.println(firstbyte, BIN);
  Serial.print("secondbyte is ");
  Serial.print("\t");
  Serial.println(secondbyte, BIN);
  Serial.print("Concatenated byte is ");
  Serial.print("\t");
  Serial.println(val, BIN);
  Serial.print("Converted temp is ");
  Serial.print("\t");
  Serial.print(convertedtemp);
  Serial.println(" degrees Celcius and ");
  Serial.print(convertedfahrenheit);
  Serial.println(" degrees Fahrenheit");
  Serial.print("Corrected temp is ");
  Serial.print("\t");
  Serial.print(correctedtemp);
  Serial.println(" degrees Celcius and ");
  Serial.print(correctedfahrenheit);
  Serial.println(" degrees Fahrenheit");
  Serial.println();
}
