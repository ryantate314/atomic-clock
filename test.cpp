#include "NtpClient.h"
#include <iostream>

int main(int argc, char* argv[]) {
    NtpClient client;


    // Update with a specific NTP server
    if (argc > 1) {
        std::cout << "Using NTP server: " << argv[1] << std::endl;
        // Initialize the client
        client.init(argv[1]);
        uint64_t trueTime = client.update();
        std::cout << "True time (ms since epoch): " << trueTime << std::endl;
    } else {
        perror("No NTP server provided. Usage: ./NtpClient <NTP_SERVER_IP>");
        return 1;
    }

    return 0;
}