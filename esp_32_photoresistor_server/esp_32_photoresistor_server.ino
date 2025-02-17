#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>

const char* ssid = "xxxxxxxx";
const char* password = "xxxxxxxx";
const char* serverIP = "http://192.168.1.202:5000"; //IP and port of the server hosting DB and API. Change to fit your needs.
const char* timeGet = "http://192.168.1.202:5000/get_time"; //Implementation of API call.
const char* postData = "http://192.168.1.202:5000/add_value"; //Implementation of API call.
const int led = 13;
const int deviceID = 123456789; //Device ID conventions are to be established by the user.
int httpCode = -0;
int currentMinute = -0;
int reading = -0;
unsigned long lastFiveMinuteUpdate = 0;
bool errorSinceLastReset = false;
String currentTime = "";
String errorCodes = ""; //This is used as a basic logger, for display in the webUI.
IPAddress staticIP(192, 168, 1, 100); //IP for this device. Change as desired, depending on network.
IPAddress subnet(255, 255, 255, 0);
IPAddress gateway(192, 168, 1, 1);
WiFiClient client;

bool transistor = false;

	//The following dynamically generates an HTML document to display as a webUI
    String generateHtml() {
    return "<!DOCTYPE html>\n"
    "<html lang=\"en\">\n"
    "<head>\n"
    "  <meta charset=\"UTF-8\">\n"
    "  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
    "  <title>Light Level Server</title>\n"
    "  <link rel=\"stylesheet\" href=\"styles.css\">\n"
	"<script>\n"
	"  function refresh() {\n"
	"    fetch('/')"
	"    .then(response => response.text())"
	"    .then(data => {"
	"      document.body.innerHTML = data;"
	"    })"
	"    .catch(error => {"
	"      console.error('Error fetching page:', error);"
	"    });"
	"  }\n"
    "  function DBPost() {\n"
	"    document.getElementById('DBPostButton').disabled = true;" //Set a one-second delay, before reactivating, to avoid Primary Key crashes.
	"    fetch('/dbpost')"
	"    .then(response => response.text())"
	"    .then(data => {"
	" 	   setTimeout(() => {"
	" 	     document.getElementById('DBPostButton').disabled = false;"
	"      document.body.innerHTML = data;"
	"	   }, 1000);"
	"    })"
	"    .catch(error => {"
	"      console.error('Error fetching page:', error);"
	" 	   setTimeout(() => {"
	" 	     document.getElementById('DBPostButton').disabled = false;"
	"	   }, 1000);"
	"    });"
	"  }\n"
	"</script>"
    "</head>\n"
    "<body>\n"
    "  <div class=\"container\">\n"
    "    <h1>ESP32 Light Level Sensor/Server</h1>\n"
    "    <p>ManawatuCoder is hosting this on an esp32!</p>\n"
	"	 <button onclick=\"refresh()\">Refresh Values</button>\n"
    "    <button id=\"DBPostButton\" onclick=\"DBPost()\">Post To DB</button>\n"
    "    <p id=\"date_time\"> Current Time: " + currentTime + "<br>Current Reading: " + reading + "<br>Latest HTTP Code: " + httpCode + "<br>Error Since Last Reset? " + String(errorSinceLastReset ? "true: " : "false") + errorCodes + "</p>\n"
    "  </div>\n"
    "</body>\n"
    "</html>";
    }
    
WebServer server(80);

void handleRoot() {
  HTTPClient httpClient;
  reading = analogRead(34);
  
  //Get time from server API.
  //This can be instantiated upon loading, and tracked by the microcontroller clock
  //But this allows it to stay synchronised with the database system, even if the database system clock changes.
  //Same is true for time retrieval elsewhere in this codebase.
  httpClient.begin(timeGet);
  httpClient.setTimeout(5000);
  httpCode = httpClient.GET();
  if (httpCode == HTTP_CODE_OK) {
      currentTime = httpClient.getString();
  } else {
      currentTime = "Failed to retrieve time: " + String(httpCode);
	  errorSinceLastReset = true;
	  errorCodes += "timeGet: ";
	  errorCodes += String(httpCode);
	  errorCodes += "; ";
  }
  httpClient.end();
  String htmlDoc = generateHtml();
  server.send(200, "text/html", htmlDoc);
}

void DBPost() {
  HTTPClient httpClient;
  reading = analogRead(34); //Reading from the light sensor.
  //The above line will work with any analog sensor compatible with the esp32 MCU.

  httpClient.begin(timeGet);
  httpClient.setTimeout(5000);
  httpCode = httpClient.GET();
  if (httpCode == HTTP_CODE_OK) {
      currentTime = httpClient.getString();
  } else {
      currentTime = "Failed to fetch time: " + String(httpCode) + " " + httpClient.getString();
      errorSinceLastReset = true;
	  errorCodes += "timeGet: ";
	  errorCodes += String(httpCode);
	  errorCodes += "; ";
  }
  httpClient.end();

  httpClient.begin(postData);
  httpClient.setTimeout(5000);
  httpCode = httpClient.POST(currentTime + "," + reading + "," + deviceID);
   if (httpCode != HTTP_CODE_OK) {
    errorSinceLastReset = true;
	errorCodes += "DBPost: ";
	errorCodes += String(httpCode);
	errorCodes += "; ";
   }
  httpClient.end();
  String htmlDoc = generateHtml(); //We should really be updating this in the browser, not on the esp32.
  server.send(200, "text/html", htmlDoc); //This should simply return a value/values for the browser to update the page with.
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(34, INPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.config(staticIP, gateway, subnet);
  WiFi.begin(ssid, password);
  HTTPClient httpClient;
  Serial.println("");
  reading = analogRead(34); //Initial light level reading.

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  httpClient.begin(timeGet);
  httpClient.setTimeout(5000);
  httpCode = httpClient.GET();
  if (httpCode == HTTP_CODE_OK) {
      currentTime = httpClient.getString();
  } else {
      currentTime = "Failed to fetch time: " + String(httpCode);
	  errorSinceLastReset = true;
	  errorCodes += "Setup timeGet: ";
	  errorCodes += String(httpCode);
	  errorCodes += "; ";
  }
  httpClient.end();
  
  //POSTs to DB upon power-on.
  httpClient.begin(postData);
  httpClient.setTimeout(5000);
  httpCode = httpClient.POST(currentTime + "," + String(reading) + "," + deviceID);
   if (httpCode != HTTP_CODE_OK) {
      errorSinceLastReset = true;
	  errorCodes += "Setup DBPost: ";
	  errorCodes += String(httpCode);
	  errorCodes += "; ";
   }
  httpClient.end();

  server.on("/", handleRoot);
  server.on("/dbpost", DBPost);

  //Plain text return of the current time. Why not?
  server.on("/gettime", []() {
    server.send(200, "text/plain", currentTime);
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();


  if (millis() - lastFiveMinuteUpdate >= 300000) {  
        lastFiveMinuteUpdate = millis();  // Update timestamp
        DBPost();
    }

  delay(2);
}
