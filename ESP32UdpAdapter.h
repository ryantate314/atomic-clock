#include <WiFiUdp.h>

class ESP32UdpAdapter {
    private:
        WiFiUDP udp;
        const char* ip;
        uint16_t port;
    public:
        bool begin(const char* ip, uint16_t port) {
            this->ip = ip;
            this->port = port;
            return udp.begin(port);
        }

        void beginPacket() {
            udp.beginPacket(this->ip, this->port);
        }

        void write(const uint8_t* buf, size_t len) {
            udp.write(buf, len);
        }

        void endPacket() {
            udp.endPacket();
        }

        bool parsePacket() {
            return udp.parsePacket() > 0;
        }

        ssize_t read(uint8_t* buf, size_t len) {
            return udp.read(buf, len);
        }

        void usleep(int us) {
            ::usleep(us);
        }
};
     