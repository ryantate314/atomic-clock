#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

class NativeUDP {
    int sockfd;
    struct sockaddr_in local_addr, remote_addr;
public:
    NativeUDP() : sockfd(-1) {}

    bool begin(const char* local_ip, uint16_t port) {
        sockfd = socket(AF_INET, SOCK_DGRAM, 0);
        if (sockfd < 0) return false;
        memset(&local_addr, 0, sizeof(local_addr));
        local_addr.sin_family = AF_INET;
        local_addr.sin_addr.s_addr = inet_addr(local_ip);
        local_addr.sin_port = htons(port);
        return connect(sockfd, (struct sockaddr*)&local_addr, sizeof(local_addr)) == 0;
    }

    ssize_t write(const uint8_t* buf, size_t len) {
        return ::write(sockfd, buf, len);
    }

    ssize_t read(uint8_t* buf, size_t len) {
        return ::read(sockfd, buf, len);
    }

    bool hasData() {
        uint8_t buffer[48];
        return recv(sockfd, buffer, 48, MSG_PEEK | MSG_DONTWAIT) > 0;
    }

    ~NativeUDP() {
        if (sockfd >= 0) close(sockfd);
    }
};