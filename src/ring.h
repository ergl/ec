#ifndef _RING_H_
#define _RING_H_

struct ring_t {
    // Backing array
    char* buffer;
    // Start and end pointers
    unsigned int start;
    unsigned int end;
    // Max capacity of the buffer
    unsigned int capacity;
    // 1 -> full
    unsigned int full;
};

// Initialize a ring buffer (byo ring and buffer)
void ring_init(struct ring_t* ring, char* buffer, unsigned int size);

// Mark ring as empty
void ring_reset(struct ring_t* ring);

// Push a value to the ring
// Old values will be overwritten
void ring_put(struct ring_t* ring, char data);

// Pop head of the ring
int ring_get(struct ring_t* ring, char* data);

// 1 -> empty, 0 -> not
int ring_empty(struct ring_t* ring);

// 1 -> full, 0 -> not
int ring_full(struct ring_t* ring);

// Return max number of elements in the ring
unsigned int ring_capacity(struct ring_t* ring);

// Return number of elements in the ring
unsigned int ring_size(struct ring_t* ring);

#endif
