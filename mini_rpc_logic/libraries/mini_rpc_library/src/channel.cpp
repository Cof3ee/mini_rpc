#include "channel.h"

#include "crc8.h"

#include <cstring>

using namespace std;

ChannelDecoder::ChannelDecoder() 
{ 
    reset(); 
}

void ChannelDecoder::reset() 
{
    channel_buf.clear();
    expected_payload_len = 0;
}

// Frame encoding: collect fields and calculate CRC
vector<uint8_t> channel_encode(const vector<uint8_t>& payload) 
{
    vector<uint8_t> frame;
    frame.reserve(6 + payload.size());
    frame.push_back(FRAME_HDR);

    uint16_t len = static_cast<uint16_t>(payload.size());
    frame.push_back(static_cast<uint8_t>(len & 0xFF));        // len_l
    frame.push_back(static_cast<uint8_t>((len >> 8) & 0xFF)); // len_h

    // CRC on two bytes of length
    uint8_t crc_hdr = crc8(&frame[1], 2);
    frame.push_back(crc_hdr);

    frame.push_back(FRAME_DATA); // start data
    frame.insert(frame.end(), payload.begin(), payload.end()); // payload

    // Packet CRC: all bytes from FRAME_HDR to the last byte of payload inclusive
    uint8_t crc_pkt = crc8(frame.data(), frame.size());
    frame.push_back(crc_pkt);

    frame.push_back(FRAME_END);

    return frame;
}

bool ChannelDecoder::feed(uint8_t byte, vector<uint8_t>& out_payload) 
{
    channel_buf.push_back(byte);

    // If the first byte is not a start byte - reset
    if (channel_buf.size() == 1 && channel_buf[0] != FRAME_HDR) 
    { 
        channel_buf.clear(); return false;
    }

    // When 5 bytes have been accumulated, the header can be parsed
    if (channel_buf.size() == 5 && channel_buf[0] == FRAME_HDR) 
    {
        uint16_t len = static_cast<uint16_t>(channel_buf[1]) | (static_cast<uint16_t>(channel_buf[2]) << 8);
        uint8_t crc_hdr_recv = channel_buf[3];
        uint8_t crc_hdr_calc = crc8(&channel_buf[1], 2);
        
        if (crc_hdr_recv != crc_hdr_calc) 
        { 
            channel_buf.clear(); return false;
        }
        expected_payload_len = len;
    }

    if (expected_payload_len != 0) 
    {
        size_t full_len = 1 + 2 + 1 + 1 + expected_payload_len + 1 + 1;
        if (channel_buf.size() >= full_len) 
        {
            // Check stop byte
            if (channel_buf[full_len - 1] != FRAME_END) 
            { 
                channel_buf.clear(); expected_payload_len = 0; return false;
            }
           
            // Extract payload
            vector<uint8_t> payload;
            payload.insert(payload.end(), channel_buf.begin() + 5, channel_buf.begin() + 5 + expected_payload_len);
            uint8_t crc_recv = channel_buf[5 + expected_payload_len];
            // CRC is calculated for all bytes from the beginning of the frame to the end of the payload inclusively
            uint8_t crc_calc = crc8(channel_buf.data(), 5 + expected_payload_len);
           
            if (crc_recv != crc_calc) 
            { 
                channel_buf.clear(); expected_payload_len = 0; return false; 
            }
            
            out_payload = move(payload);
            channel_buf.clear();
            expected_payload_len = 0;
            return true;
        }
    }
    return false;
}
