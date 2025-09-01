#pragma once

#include "transport.h" 
#include "channel.h"
#include <UART_TX.h>

#include <string>

struct Client_Response_Slot 
{
    uint8_t id;
    std::vector<std::vector<uint8_t>> responses; // multiple responses to one id
    bool used;
};

class Client_RPC
{
public:

	Client_RPC() : counter_msg(0) {}

    void send_msg(std::string name_function, uint8_t arg_1, uint8_t arg_2, uint8_t type_msg);
    void on_response(const transport_message& msg);
    bool pop_response(uint8_t id, std::vector<uint8_t>& out);

    static void register_global(Client_RPC* inst);
    static Client_RPC* global();

private:

	transport_message make_msg(std::string name_function, uint8_t arg_1, uint8_t arg_2, uint8_t type_msg);

private:
	uint8_t counter_msg;
        
        static constexpr size_t MAX_SLOTS = 8;
        Client_Response_Slot slots_[MAX_SLOTS];
};