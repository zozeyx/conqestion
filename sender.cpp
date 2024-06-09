#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <cstdint>

#define BUFFER_SIZE 70
#define PACKET_SIZE 14
#define PAYLOAD_SIZE 10
#define CHECKSUM_SIZE 2

using namespace std;

struct Packet {
    uint16_t packet_number;
    uint16_t checksum;
    char payload[PAYLOAD_SIZE];
};

uint16_t calculate_checksum(char *data, int length) {
    uint32_t sum = 0;
    for (int i = 0; i < length; i++) {
        sum += data[i];
    }
    return (uint16_t)(sum % 65536);
}

void sender(const char *filename) {
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Failed to open input file" << endl;
        exit(EXIT_FAILURE);
    }

    ofstream buffer_file("buffer.bin", ios::binary);
    if (!buffer_file.is_open()) {
        cerr << "Failed to open buffer file" << endl;
        file.close();
        exit(EXIT_FAILURE);
    }

    uint16_t packet_number = 0;
    while (!file.eof()) {
        Packet packet;
        packet.packet_number = packet_number++;
        file.read(packet.payload, PAYLOAD_SIZE);
        if (file.eof()) {
            cout << "End of file reached" << endl;
            break;
        }

        packet.checksum = calculate_checksum(packet.payload, PAYLOAD_SIZE);

        buffer_file.write(reinterpret_cast<char*>(&packet), sizeof(Packet));
        buffer_file.flush(); // Ensure data is written to disk

        cout << "Packet " << packet.packet_number << " sent successfully" << endl;
        sleep(1); // sender 주기
    }

    file.close();
    buffer_file.close();
}

int main() {
    const char *input_filename = "input1.txt";
    sender(input_filename);
    return 0;
}
