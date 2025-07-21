#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include "NtpClient.h"
#include <ESPmDNS.h>
#include <cmath>

const int lcdColumns = 16;
const int lcdRows = 2;
const int lcdAddress = 0x27;

LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);

const char *ssid = "The Promised LAN";
const char *password = "BlueLizard6535";

NtpClient client;

int gps_offset = 0;
const long five_hundred_ms = 0.5 * 1e6;
const long one_second = 1e6;
bool updated_from_gps = false;

void IRAM_ATTR isr() {
  struct timeval tv;
  gettimeofday(&tv, NULL);

  if (tv.tv_usec > five_hundred_ms) {
    // Assume slow
    gps_offset = tv.tv_usec - one_second;
  }
  else {
    // Assum fast
    gps_offset = tv.tv_usec;
  }

  if (abs(gps_offset) > 500) {
    updated_from_gps = true;
    if (gps_offset < 0) {
      tv.tv_usec = 0;
      tv.tv_sec++;
    }
    else {
      tv.tv_usec = 0;
    }
    settimeofday(&tv, NULL);
  }

}


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

  char* ip = "129.6.15.28";

  client.init(ip);

  uint64_t time = client.update();
  Serial.print("Received time: ");
  Serial.println(time);
  struct timeval tv;
  tv.tv_sec = time / 1000;
  tv.tv_usec = (time % 1000) * 1000;
  settimeofday(&tv, NULL);
  lcd.clear();

  pinMode(12, INPUT);
  attachInterrupt(12, isr, RISING);
}

void loop() {
  // put your main code here, to run repeatedly:
  lcd.setCursor(0, 0);
  struct tm timeInfo;
  getLocalTime(&timeInfo);
  struct timeval tv;
  gettimeofday(&tv, NULL);
  char timeStringBuff[64];
  strftime(timeStringBuff, sizeof(timeStringBuff), "%H:%M:%S", &timeInfo);
  // Serial.print("Current time: ");
  // Serial.println(timeStringBuff);
  lcd.print(timeStringBuff);
  lcd.setCursor(8, 0);
  lcd.print('.');
  lcd.setCursor(9, 0);
  uint16_t ms = tv.tv_usec / 1e3;
  lcd.print(ms);

  // Serial.print("GPS offset: ");
  // Serial.println(gps_offset);

  if (updated_from_gps) {
    Serial.println("Updated GPS time");
    updated_from_gps = false;
  }

  delay(100);
}
