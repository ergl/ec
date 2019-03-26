#include "ring.h"

static void forward(struct ring_t* ring) {
    if (ring->full) {
        ring->end = (ring->end + 1) % ring->capacity;
    }

    ring->start = (ring->start + 1) % ring->capacity;
    ring->full = (ring->start == ring->end);
}

static void backward(struct ring_t* ring) {
    ring->full = 0;
    ring->end = (ring->end + 1) % ring->capacity;
}

void ring_init(struct ring_t* ring, char* buffer, unsigned int size) {
    ring->buffer = buffer;
    ring->capacity = size;
    ring_reset(ring);
}

void ring_reset(struct ring_t* ring) {
    ring->start = 0;
    ring->end = 0;
    ring->full = 0;
}

void ring_put(struct ring_t* ring, char data) {
    ring->buffer[ring->start] = data;
    forward(ring);
}

int ring_get(struct ring_t* ring, char* data) {
    if (ring_empty(ring)) {
        return -1;
    }

    *data = ring->buffer[ring->end];
    backward(ring);
    return 0;
}

int ring_empty(struct ring_t* ring) {
    return (!ring->full && (ring->start == ring->end));
}

int ring_full(struct ring_t* ring) {
    return ring->full;
}

unsigned int ring_capacity(struct ring_t* ring) {
    return ring->capacity;
}

unsigned int ring_size(struct ring_t* ring) {
    unsigned int capacity = ring->capacity;
    unsigned int start = ring->start;
    unsigned int end = ring->end;
    unsigned int full = ring->full;

    if (full) {
        return capacity;
    }

    // Start is pass end, just substract
    if (start >= end) {
        return (start - end);
    }

    // End is past start, diff the offset
    return ((start + capacity) - end);
}
