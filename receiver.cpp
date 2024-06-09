#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#define BUFFER_SIZE 70
#define PACKET_SIZE 14
#define PAYLOAD_SIZE 10

typedef struct {
    uint16_t packet_number;
    uint16_t checksum;
    char payload[PAYLOAD_SIZE];
} Packet;

typedef struct {
    Packet packets[BUFFER_SIZE / PACKET_SIZE];
    int head;
    int tail;
    int count;
} Buffer;

// Function prototypes
uint16_t calculate_checksum(char *data, int length);
void receiver(Buffer *buffer, const char *output_filename);

uint16_t calculate_checksum(char *data, int length) {
    uint32_t sum = 0;
    for (int i = 0; i < length; i++) {
        sum += data[i];
    }
    return (uint16_t)(sum % 65536);
}

void receiver(Buffer *buffer, const char *output_filename) {
    static FILE *output_file = NULL;
    static uint16_t expected_packet_number = 0;
    if (output_file == NULL) {
        output_file = fopen(output_filename, "w");
        if (output_file == NULL) {
            perror("Failed to open output file");
            exit(EXIT_FAILURE);
        }
    }

    if (buffer->count == 0) {
        printf("Buffer is empty, nothing to receive\n");
        return;
    }

    Packet packet = buffer->packets[buffer->head];
    buffer->head = (buffer->head + 1) % (BUFFER_SIZE / PACKET_SIZE);
    buffer->count--;

    // Check packet checksum
    uint16_t computed_checksum = calculate_checksum(packet.payload, PAYLOAD_SIZE);
    if (computed_checksum != packet.checksum || packet.packet_number != expected_packet_number) {
        printf("Packet %d is corrupted or out of order, requesting retransmission\n", packet.packet_number);
        // Request retransmission logic (simplified)
        return;
    }

    fwrite(packet.payload, 1, PAYLOAD_SIZE, output_file);
    expected_packet_number++;
    printf("Packet %d received successfully\n", packet.packet_number);

    // Send ACK (simplified)
    printf("ACK for packet %d\n", expected_packet_number);
}

int main() {
    Buffer buffer = { .head = 0, .tail = 0, .count = 0 };
    const char *output_filename = "output1.txt";

    while (1) {
        receiver(&buffer, output_filename);
        sleep(1); // receiver rate
    }

    return 0;
}
