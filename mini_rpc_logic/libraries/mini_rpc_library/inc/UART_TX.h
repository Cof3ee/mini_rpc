#pragma once

#include <vector>
#include <cstdint>

// Initialize the transmission module
void tx_comm_init();

// Non-blocking send: places a copy of packet in the transmit queue.
// Returns true if the frame is queued, false if the queue is full or no memory is allocated.
bool send_packet(const std::vector<uint8_t>& packet);