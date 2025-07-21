#include "ESP32UdpAdapter.h"
#include <chrono>

#ifndef NTPCLIENT_H
#define NTPCLIENT_H

const uint8_t LEAP_INDICATOR_UNKNOWN = 0b11;
const uint8_t VERSION_NUMBER = 0b0100; // NTP version 4
const uint8_t MODE_CLIENT = 0b11; // Client mode

// 2208988800
const unsigned long SEVENTY_YEARS = (70*365 + 17) * 24L * 60 * 60; // 70 years plus 17 leap days

struct NtpPacket {
  // 2-bit leap indicator, 3-bit version number, 3-bit mode
  uint8_t mode = (LEAP_INDICATOR_UNKNOWN << 6) | (VERSION_NUMBER << 3) | MODE_CLIENT;
  uint8_t stratum = 0xff;
  uint8_t poll;
  int8_t precision;
  uint32_t rootDelay;
  uint32_t rootDispersion;
  // 4-byte ascii code
  uint8_t referenceId[4];
  // Time the the system clock was last set or corrected
  uint64_t referenceTimestamp;
  // uint32_t referenceTimestamp_h;
  // uint32_t referenceTimestamp_l;
  // Time the Client thinks it is
  uint64_t originTimestamp;
  // uint32_t originTimestamp_h;
  // uint32_t originTimestamp_l;
  // Time the server received the request
  uint64_t receiveTimestamp;
  // uint32_t receiveTimestamp_h;
  // uint32_t receiveTimestamp_l;
  // Time the server sends the response
  uint64_t transmitTimestamp;
  // uint32_t transmitTimestamp_h;
  // uint32_t transmitTimestamp_l;
};

const int port = 123;


class NtpClient {

  private:
    ESP32UdpAdapter udp;
    NtpPacket transmitPacket;
    NtpPacket receivePacket;

    void serialize_packet(uint8_t* buff, NtpPacket &packet) {
      // Clear output buffer
      memset(buff, 0, sizeof(buff));
      // Serialize the NTP packet into the buffer
      buff[0] = packet.mode;
      buff[1] = packet.stratum;
      buff[2] = packet.poll;
      buff[3] = packet.precision;
      memcpy(&buff[4], &packet.rootDelay, sizeof(packet.rootDelay));
      memcpy(&buff[8], &packet.rootDispersion, sizeof(packet.rootDispersion));
      memcpy(&buff[12], &packet.referenceId, sizeof(packet.referenceId));
      memcpy(&buff[16], &packet.referenceTimestamp, sizeof(packet.referenceTimestamp));
      memcpy(&buff[24], &packet.originTimestamp, sizeof(packet.originTimestamp));
      memcpy(&buff[32], &packet.receiveTimestamp, sizeof(packet.receiveTimestamp));
      memcpy(&buff[40], &packet.transmitTimestamp, sizeof(packet.transmitTimestamp));
    }

    void parse_packet(uint8_t* buff, NtpPacket &packet) {
      // Deserialize the NTP packet from the buffer
      // std::memcpy(&receivePacket, buff, sizeof(NtpPacket));
      packet.mode = buff[0];
      packet.stratum = buff[1];
      packet.poll = buff[2];
      packet.precision = buff[3];

      netToHost(&buff[4], &packet.rootDelay);
      netToHost(&buff[8], &packet.rootDispersion);

      // ASCII Text
      memcpy(&packet.referenceId, &buff[12], sizeof(packet.referenceId));
      uint32_t high, low;

      netToHost(&buff[16], &high);
      netToHost(&buff[20], &low);
      packet.referenceTimestamp = static_cast<uint64_t>(high) << 32 | low;

      netToHost(&buff[24], &high);
      netToHost(&buff[28], &low);
      packet.originTimestamp = static_cast<uint64_t>(high) << 32 | low;

      netToHost(&buff[32], &high);
      netToHost(&buff[36], &low);
      packet.receiveTimestamp = static_cast<uint64_t>(high) << 32 | low;

      netToHost(&buff[40], &high);
      netToHost(&buff[44], &low);
      packet.transmitTimestamp = static_cast<uint64_t>(high) << 32 | low;
    }

  public:
    void init(const char* ip) {
      // udp.begin(WiFi.localIP(), port);
      udp.begin(ip, 123);
    }

    // Returns the unix timestamp in milliseconds
    uint64_t update() {
      uint8_t transmitBuffer[48] = {0};

      transmitPacket.originTimestamp = nowNtp();
      serialize_packet(transmitBuffer, transmitPacket);

      // std::cout << "Sending NTP request at " << nowUnix() << " ms" << std::endl;

      udp.beginPacket();
      udp.write(transmitBuffer, 48);
      udp.endPacket();

      int attempts = 0;
      const int delay = 100; //microseconds
      int maxAttempts = 10 / (delay / 1.0e6);
      while (!udp.parsePacket() && attempts < maxAttempts) {
        udp.usleep(delay);
        attempts++;
      }

      uint64_t receivedAt = nowNtp();

      if (attempts >= maxAttempts) {
        // Serial.println("Failed to receive NTP response");
        // std::cout << "Failed to receive NTP response" << std::endl;
        return 0;
      }

      udp.read(transmitBuffer, 48);
      // std::cout << "Buffer: ";
      // printHex(transmitBuffer, 48);
      // std::cout << std::endl;

      parse_packet(transmitBuffer, receivePacket);

      // uint64_t unixTime = ntpToUnix(receivePacket.transmitTimestamp);
      // std::cout << "Transmit time: " << unixTime << " ms" << std::endl;

      return ntpToUnix(getTrueTime(receivedAt));
    }

    int getNetworkDelay(uint64_t receivedAt) {
      return (
        receivedAt - receivePacket.originTimestamp -
        (
          receivePacket.transmitTimestamp - receivePacket.receiveTimestamp
        )
      ) / 2;
    }

    uint64_t getTrueTime(uint64_t receivedAt) {
      return receivePacket.transmitTimestamp + getNetworkDelay(receivedAt);
    }

    uint64_t ntpToUnix(uint32_t high, uint32_t low) {
      // Convert NTP timestamp to Unix timestamp
      return ntpToUnix((static_cast<uint64_t>(high) << 32) | low);
    }

    uint64_t ntpToUnix(uint64_t ntpTime) {
      // Convert NTP timestamp to Unix timestamp
      uint64_t unixTime = (ntpTime >> 32) - SEVENTY_YEARS;
      uint64_t fractionalPart = (ntpTime & 0xFFFFFFFF) * 1000.0 / UINT32_MAX;
      return unixTime * 1000 + fractionalPart;
    }

    uint64_t nowUnix() {
      return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    }

    uint64_t unixToNtp(uint64_t unixTime) {
      uint32_t high = unixTime / 1000 + SEVENTY_YEARS;
      uint32_t low = (unixTime % 1000) / 1000.0 * UINT32_MAX;
      return (static_cast<uint64_t>(high) << 32) | low;
    }

    uint64_t nowNtp() {
      return unixToNtp(nowUnix());
    }

    // void printHex(const uint8_t* data, size_t len) {
    //   for (size_t i = 0; i < len; i++) {
    //     std::cout << std::hex << std::setw(2) << std::setfill('0')
    //       << static_cast<uint32_t>(data[i]) << " ";
    //   }
    //   std::cout << std::dec << std::endl;
    // }

    // Parses a 4-byte network byte order buffer into a 32-bit unsigned integer
    void netToHost(const uint8_t* buffer, uint32_t* target) {
      *target = (static_cast<uint32_t>(buffer[0]) << 24) |
                (static_cast<uint32_t>(buffer[1]) << 16) |
                (static_cast<uint32_t>(buffer[2]) << 8) |
                static_cast<uint32_t>(buffer[3]);
    }
};


#endif
