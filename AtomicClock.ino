#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include "NtpClient.h"
#include <ESPmDNS.h>
#include <cmath>
#include "RTClib.h"

RTC_DS3231 rtc;



LiquidCrystal_I2C lcd(lcdAddress, lcdColumns, lcdRows);

const char *ssid = "The Promised LAN";
const char *password = "BlueLizard6535";

NtpClient ntpClient;

const uint32_t gps_sync_threshhold_us = 500;
const int16_t gps_sync_calibration = -94;
const long five_hundred_ms = 0.5 * 1e6;
const long one_second = 1e6;
volatile uint64_t last_gps_pulse = 0;

void IRAM_ATTR isr() {
  last_gps_pulse = esp_timer_get_time();
}

timeval update_ntp() {
  uint64_t time = ntpClient.update();
  Serial.print("Received time: ");
  Serial.println(time);
  struct timeval tv;
  tv.tv_sec = time / 1000;
  tv.tv_usec = (time % 1000) * 1000;
  settimeofday(&tv, NULL);
  lcd.clear();
  return tv;
}


void setup() {
  // put your setup code here, to run once:
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Booting up...");

  Serial.begin(115200);

  //Connect to the WiFi network
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString());
  delay(1000);

  char* ip = "129.6.15.28";
  ntpClient.init(ip);

  rtc.begin();
  if (rtc.lostPower()) {
    Serial.println("RTC Lost Power");
    int64_t time = ntpClient.update();
    rtc.adjust(DateTime(time / 1000));
    Serial.print("Updated RTC with current time: ");
    Serial.println(time / 1000);
  }

  DateTime rtcTime = rtc.now();
  Serial.print("RTC Time: ");
  Serial.println(rtcTime.unixtime());
  struct timeval tv;
  tv.tv_sec = rtcTime.unixtime();
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);

  pinMode(5, INPUT);
  attachInterrupt(5, isr, FALLING);
}

int get_gps_offset_us(timeval *tv) {
  if (last_gps_pulse == 0)
    return 0;
  uint64_t us_since_pulse = esp_timer_get_time() - last_gps_pulse;
  if (us_since_pulse > one_second * 10) {
    Serial.println("Too long since last pulse");
    last_gps_pulse = 0;
    return 0;
  }
  int diff = (us_since_pulse % one_second) - tv->tv_usec;

  return diff;
}

void loop() {
  struct tm timeInfo;
  getLocalTime(&timeInfo);
  struct timeval tv;
  gettimeofday(&tv, NULL);

  // Check GPS time
  int gps_offset = get_gps_offset_us(&tv);
  if (gps_offset != 0) {
    // Serial.print("GPS offset: ");
    // Serial.print(gps_offset);
    // Serial.println("us");
    if (abs(gps_offset) > gps_sync_threshhold_us) {
      tv.tv_usec = tv.tv_usec + gps_offset + gps_sync_calibration;
      settimeofday(&tv, NULL);
      Serial.print("Updated time of day from GPS by ");
      Serial.print(gps_offset);
      Serial.println("us");
    }
  }
 


  lcd.setCursor(0, 0);
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

  // if (updated_from_gps) {
  //   Serial.println("Updated GPS time");
  //   updated_from_gps = false;
  // }

  delay(100);
}
