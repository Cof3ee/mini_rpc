#pragma once

#include <cstdint>
#include <string>
#include <vector>

static constexpr uint8_t MSG_REQ    = 0x0B;
static constexpr uint8_t MSG_STREAM = 0x0C;
static constexpr uint8_t MSG_RESP   = 0x16;
static constexpr uint8_t MSG_ERR    = 0x21;

struct transport_message 
{
    uint8_t type = 0;
    uint8_t id   = 0;
    std::string name;
    std::vector<uint8_t> data;
};

std::vector<uint8_t> transport_encode(const transport_message& msg);

transport_message transport_decode(const std::vector<uint8_t>& buf);