#include "NativeUDP.h"

class NativeUDPAdapter {
    private:
        NativeUDP udp;
    public:
        bool begin(const char* ip, uint16_t port) {
            return udp.begin(ip, port);
        }

        void beginPacket() {
            // Do nothing
        }

        void write(const uint8_t* buf, size_t len) {
            udp.write(buf, len);
        }

        void endPacket() {
            // Do nothing
        }

        bool parsePacket() {
            return udp.hasData();
        }

        ssize_t read(uint8_t* buf, size_t len) {
            return udp.read(buf, len);
        }

        void usleep(int us) {
            ::usleep(us);
        }
};
     