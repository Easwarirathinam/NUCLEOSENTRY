#include <DFRobot_Geiger.h>
#include <WiFi.h>
#include <WebServer.h>
#include <U8g2lib.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h> // Include the HardwareSerial library

// Pin definitions
#define detect_pin 2

// WiFi credentials
const char* ssid = "Rizia";
const char* password = "123456789";

// Global objects
DFRobot_Geiger geiger(detect_pin);
WebServer server(80);
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=/U8X8_PIN_NONE, / clock=*/22, /* data=*/21);
HardwareSerial SIM800Serial(2); // Define hardware serial for SIM800C, using GPIO 16 (RX) and 17 (TX)

// Function prototypes
void connectToWiFi();
void handleRoot();
void handleDataRequest();
void updateDisplay(int cpm, float nSv_h);
void testSIM800();
void makeCall(String phoneNumber);
void sendSMS(String phoneNumber, String message);

void setup() {
  Serial.begin(115200);
  delay(100);

  connectToWiFi();

  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleDataRequest);

  server.begin();

  u8g2.begin();
  geiger.start();

  SIM800Serial.begin(9600);
  delay(1000);
}

void loop() {
  server.handleClient();

  int cpm = geiger.getCPM();
  float nSv_h = geiger.getnSvh();

  updateDisplay(cpm, nSv_h);

  // Check if either CPM or nSv/h exceeds the threshold
  if (cpm >= 2 || nSv_h >= 2) {
    makeCall("+918248397283"); // Make a call to the specified phone number
    delay(30000); // Delay for 30 seconds to avoid making multiple calls rapidly
   sendSMS("+918248397283", "High radiation level detected! CPM: " + String(cpm) + ", nSv/h: " + String(nSv_h));
    delay(300); // Delay for 30 seconds to avoid sending multiple SMS rapidly
  }
}

void connectToWiFi() {
  Serial.println();
  Serial.println("Connecting to WiFi...");
  
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void handleRoot() {
  int cpm = geiger.getCPM();
  float nSv_h = geiger.getnSvh();

  String page = "<!DOCTYPE html><html><head><title>Geiger Readings</title><style>table, th, td {border: 1px solid blue;}</style><script>function updateData() {var xhr = new XMLHttpRequest();xhr.onreadystatechange = function() {if (xhr.readyState == 4 && xhr.status == 200) {var data = JSON.parse(xhr.responseText);document.getElementById(\"cpm\").innerHTML = data.CPM;document.getElementById(\"nSv_h\").innerHTML = data.nSv_h;}};xhr.open(\"GET\", \"/data\", true);xhr.send();}setInterval(updateData, 5000);</script></head><body><h1 style=\"font-size:300%;\" ALIGN=CENTER>Geiger Readings</h1><p ALIGN=CENTER style=\"font-size:150%;\"><b>Geiger value</b></p><table ALIGN=CENTER style=\"width:50%\"><tr><th>CPM</th><td ALIGN=CENTER id=\"cpm\">";
  page += cpm;
  page += "</td></tr><tr><th>nSv/h</th><td ALIGN=CENTER id=\"nSv_h\">";
  page += nSv_h;
  page += "</td></tr></table></body></html>";

  server.send(200, "text/html", page);
}

void handleDataRequest() {
  StaticJsonDocument<100> jsonDocument;
  jsonDocument["CPM"] = geiger.getCPM();
  jsonDocument["nSv_h"] = geiger.getnSvh();

  String jsonResponse;
  serializeJson(jsonDocument, jsonResponse);

  server.send(200, "application/json", jsonResponse);
}

void updateDisplay(int cpm, float nSv_h) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont17_tr);
  u8g2.setCursor(50, 17);
  u8g2.print("GEIGER=");
  u8g2.setCursor(55, 40);
  u8g2.print(cpm);
  u8g2.print(" CPM");
  u8g2.setFont(u8g2_font_profont15_tr);
  u8g2.setCursor(10, 50);
  u8g2.print("nSv/h=");
  u8g2.print(nSv_h);
  u8g2.setFont(u8g2_font_open_iconic_app_4x_t);
  u8g2.drawGlyph(10, 35, 72);
  u8g2.sendBuffer();
}

void testSIM800() {
  SIM800Serial.println("AT");
  delay(1000);
  if(SIM800Serial.find("OK")) {
    Serial.println("SIM800C is ready!");
  } else {
    Serial.println("SIM800C not responding. Check connections and restart.");
  }
}

void makeCall(String phoneNumber) {
  SIM800Serial.println("ATD" + phoneNumber + ";"); // ATD command to dial the number
  delay(1000);
}

void sendSMS(String phoneNumber, String message) {
  SIM800Serial.println("AT+CMGF=1");
  delay(1000);
  SIM800Serial.print("AT+CMGS=\"");
  SIM800Serial.print(phoneNumber);
  SIM800Serial.println("\"");
  delay(1000);
  SIM800Serial.print(message);
  delay(1000);
  SIM800Serial.write((char)26);
  delay(1000); 
}
