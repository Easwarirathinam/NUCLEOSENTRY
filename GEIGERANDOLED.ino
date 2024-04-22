#include <DFRobot_Geiger.h>
#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFi.h>       // Include the Wi-Fi library

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, 22, 21);   // ESP32 Thing, HW I2C with pin remapping

char ssid[] = "test";     // Your Wi-Fi network SSID
char pass[] = "12345678"; // Your Wi-Fi network password

#define detect_pin 2 // Assuming you're using ESP32, change it according to your board

/*!
   @brief Constructor
   @param pin   External interrupt pin
*/
DFRobot_Geiger geiger(detect_pin);

WiFiClient client;

void setup() {
  Serial.begin(115200);

  // Initialize Wi-Fi connection
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to WiFi");

  u8g2.begin();

  // Start counting, enable external interrupt
  geiger.start();
}

void loop() {
  delay(15000); // 15-second delay

  int cpm = geiger.getCPM();

  Serial.print("CPM: ");
  Serial.println(cpm);

  // Display on OLED
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_profont17_tr);
  u8g2.setCursor(50,17);
  u8g2.println("GEIGER="); // Display a message
  u8g2.setCursor(55,40);
  u8g2.println(cpm);
  u8g2.println("CPM");
  u8g2.setFont(u8g2_font_profont15_tr);
  u8g2.setCursor(10,50);
  u8g2.println("NSVH="); // Display a message
  u8g2.println(geiger.getnSvh());

  u8g2.setFont(u8g2_font_open_iconic_app_4x_t);
  u8g2.drawGlyph(10, 35, 72); 
  u8g2.sendBuffer();
}