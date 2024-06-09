#include <iostream>
#include <fstream>
#include <cstdlib>
#include <unistd.h>
#include <cstdint>

#define BUFFER_SIZE 70
#define PACKET_SIZE 14
#define PAYLOAD_SIZE 10

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

void receiver(const char *output_filename) {
    ofstream output_file(output_filename, ios::binary);
    if (!output_file.is_open()) {
        cerr << "Failed to open output file" << endl;
        exit(EXIT_FAILURE);
    }

    uint16_t expected_packet_number = 0;
    while (true) {
        ifstream buffer_file("buffer.bin", ios::binary);
        if (!buffer_file.is_open()) {
            cerr << "Failed to open buffer file" << endl;
            output_file.close();
            exit(EXIT_FAILURE);
        }

        buffer_file.seekg(0, ios::end);
        size_t file_size = buffer_file.tellg();
        buffer_file.seekg(0, ios::beg);

        if (file_size == 0) {
            cout << "Buffer is empty, nothing to receive" << endl;
            buffer_file.close();
            sleep(1); // receiver 주기
            continue;
        }

        while (true) {
            Packet packet;
            buffer_file.read(reinterpret_cast<char*>(&packet), sizeof(Packet));
            if (buffer_file.eof()) {
                break;
            }

            // Check packet checksum
            uint16_t computed_checksum = calculate_checksum(packet.payload, PAYLOAD_SIZE);
            if (computed_checksum != packet.checksum || packet.packet_number != expected_packet_number) {
                cout << "Packet " << packet.packet_number << " is corrupted or out of order, requesting retransmission" << endl;
                // Request retransmission logic (simplified)
                buffer_file.close();
                sleep(1); // receiver 주기
                continue;
            }

            output_file.write(packet.payload, PAYLOAD_SIZE);
            expected_packet_number++;
            cout << "Packet " << packet.packet_number << " received successfully" << endl;
        }

        buffer_file.close();
        sleep(1); // receiver 주기
    }

    output_file.close();
}

int main() {
    const char *output_filename = "output1.txt";
    receiver(output_filename);
    return 0;
}
