#include "Screen.h"
#include <WiFi.h>
#include "NtpClient.h"
#include <ESPmDNS.h>
#include <cmath>
#include "RTClib.h"
#include "time_helpers.h"

const int BUTTON_PIN = 4;
const int RTC_SQW_PIN = 19;
const int GPS_PPS_PIN = 5;

RTC_DS3231 rtc;

Screen screen;

const char *ssid = "The Promised LAN";
const char *password = "BlueLizard6535";

NtpClient ntpClient;

const uint32_t gps_sync_threshhold_us = 500;
const int16_t gps_sync_calibration = -94;
const long five_hundred_ms = 0.5 * 1e6;
const long one_second = 1e6;
volatile uint64_t last_gps_pulse = 0;
volatile uint64_t last_rtc_pulse = 0;

void IRAM_ATTR gps_pps_isr() {
  last_gps_pulse = esp_timer_get_time();
}

void IRAM_ATTR rtc_sqw_isr() {
  last_rtc_pulse = esp_timer_get_time();
}

uint64_t update_ntp() {
  uint64_t time = ntpClient.update();
  Serial.print("Received time: ");
  Serial.println(time);
  return time;
}

void setup() {
  // put your setup code here, to run once:
  screen.init();
  screen.showMessage("Booting up", 0);

  Serial.begin(115200);

  //Connect to the WiFi network
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  screen.showMessage("WiFi Connected", 500);

  char* ip = "129.6.15.28";
  ntpClient.init(ip);

  rtc.begin();
  if (rtc.lostPower()) {
    Serial.println("RTC Lost Power");
    int64_t time = update_ntp();
    rtc.adjust(DateTime(time / 1000));
    Serial.print("Updated RTC with current time");
    screen.showMessage("Time frm NTP");
  }
  else {
    screen.showMessage("Time frm RTC");
  }

  DateTime rtcTime = rtc.now();
  Serial.print("RTC Time: ");
  Serial.println(rtcTime.unixtime());
  struct timeval tv;
  tv.tv_sec = rtcTime.unixtime();
  tv.tv_usec = 0;
  settimeofday(&tv, NULL);

  pinMode(GPS_PPS_PIN, INPUT);
  attachInterrupt(GPS_PPS_PIN, gps_pps_isr, FALLING);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  
  pinMode(RTC_SQW_PIN, INPUT_PULLUP);
  attachInterrupt(RTC_SQW_PIN, rtc_sqw_isr, RISING);
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
  if (diff > 5e5)
    diff = diff - one_second;

  return diff;
}

void show_ntp_offset() {
  if (WiFi.status() == WL_CONNECTED) {
    screen.showMessage("Getting NTP");
    uint64_t ntp = update_ntp();
    uint64_t now = now_unix_ms();
    char buffer[16];
    snprint_diff(buffer, sizeof(buffer), ntp, now);
    screen.showMessage(buffer);
  }
  else {
    screen.showMessage("WiFi Not Connected");
  }
}

void loop() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  screen.setTime(tv);

  if (digitalRead(BUTTON_PIN) == LOW) {
    Serial.println("Button pressed!");
    // show_ntp_offset();
  }

  // Check GPS time
  int gps_offset = get_gps_offset_us(&tv);
  if (gps_offset != 0) {
    if (abs(gps_offset) > gps_sync_threshhold_us) {
      tv.tv_usec = tv.tv_usec + gps_offset + gps_sync_calibration;
      settimeofday(&tv, NULL);
      Serial.print("Updated time of day from GPS by ");
      Serial.print(gps_offset);
      Serial.println("us");
      screen.showMessage("Updt From GPS", 2000);
    }
  }
 
  screen.tick();

  delay(100);
}
