#pragma once

#include <vector>
#include <cstdint>

static constexpr uint8_t FRAME_HDR  = 0xFA;
static constexpr uint8_t FRAME_DATA = 0xFB;
static constexpr uint8_t FRAME_END  = 0xFE;

std::vector<uint8_t> channel_encode(const std::vector<uint8_t>& payload);

class ChannelDecoder 
{
public:
    ChannelDecoder();
    // Feed one byte. If returns true, out_payload contains the full payload.
    bool feed(uint8_t byte, std::vector<uint8_t>& out_payload);
    void reset();

private:
    std::vector<uint8_t> channel_buf; // internal buffer
    std::size_t expected_payload_len = 0;
};
