#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include "NtpClient.h"
#include <ESPmDNS.h>

const int lcdColumns = 16;
const int lcdRows = 2;
const int lcdAddress = 0x27;

LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);

const char *ssid = "The Promised LAN";
const char *password = "BlueLizard6535";

NtpClient client;


void setup() {
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Booting up...");

  Serial.begin(9600);

  //Connect to the WiFi network
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString());
  delay(1000);

  char* ip = WiFi.gethostbyname("pool.ntp.org");

  client.init(ip);

  uint64_t time = client.update();
}

void loop() {
  // put your main code here, to run repeatedly:
  

  delay(10000);
}
