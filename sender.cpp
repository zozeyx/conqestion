#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

#define BUFFER_SIZE 70
#define PACKET_SIZE 14
#define PAYLOAD_SIZE 10
#define CHECKSUM_SIZE 2
#define PACKET_NUM_SIZE 2

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
void sender(Buffer *buffer, const char *filename);

uint16_t calculate_checksum(char *data, int length) {
    uint32_t sum = 0;
    for (int i = 0; i < length; i++) {
        sum += data[i];
    }
    return (uint16_t)(sum % 65536);
}

void sender(Buffer *buffer, const char *filename) {
    static FILE *file = NULL;
    static uint16_t packet_number = 0;
    if (file == NULL) {
        file = fopen(filename, "r");
        if (file == NULL) {
            perror("Failed to open file");
            exit(EXIT_FAILURE);
        }
    }

    if (buffer->count >= (BUFFER_SIZE / PACKET_SIZE)) {
        printf("Buffer is full, dropping packet\n");
        return;
    }

    Packet packet;
    packet.packet_number = packet_number++;
    if (fread(packet.payload, 1, PAYLOAD_SIZE, file) == 0) {
        printf("End of file reached\n");
        fclose(file);
        file = NULL;
        return;
    }

    packet.checksum = calculate_checksum(packet.payload, PAYLOAD_SIZE);

    // Add packet to buffer
    buffer->packets[buffer->tail] = packet;
    buffer->tail = (buffer->tail + 1) % (BUFFER_SIZE / PACKET_SIZE);
    buffer->count++;
}

int main() {
    Buffer buffer = { .head = 0, .tail = 0, .count = 0 };
    const char *input_filename = "input1.txt";
    
    while (1) {
        sender(&buffer, input_filename);
        sleep(0.5); // sender rate
    }
    
    return 0;
}