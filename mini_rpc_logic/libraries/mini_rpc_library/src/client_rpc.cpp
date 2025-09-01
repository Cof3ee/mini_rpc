#include <client_rpc.h>

using namespace std;

void on_response(const transport_message& msg)
{
    Client_RPC* g = Client_RPC::global();
    if (g != nullptr) g->on_response(msg);
}

void Client_RPC::send_msg(string name_function, uint8_t arg_1, uint8_t arg_2, uint8_t type_msg)
{
    transport_message msg = make_msg(name_function, arg_1, arg_2, type_msg);

    auto payload =  transport_encode(msg);
    auto frame = channel_encode(payload);
    send_packet(frame);

    counter_msg++;
}

transport_message Client_RPC::make_msg(std::string name_function, uint8_t arg_1, uint8_t arg_2, uint8_t type_msg)
{
    transport_message msg;
    msg.name = name_function;
    msg.id = counter_msg;
    msg.data.push_back(arg_1);
    msg.data.push_back(arg_2);
    msg.type = type_msg;

    return msg;
}

void Client_RPC::on_response(const transport_message& msg)
{

    for (auto& slot : slots_) 
    {
        if (slot.used && slot.id == msg.id) 
        {
            slot.responses.push_back(msg.data);
            return;
        }
    }


    for (auto& slot : slots_) 
    {
        if (!slot.used) 
        {
            slot.used = true;
            slot.id = msg.id;
            slot.responses.clear();
            slot.responses.push_back(msg.data);
            return;
        }
    }
}

bool Client_RPC::pop_response(uint8_t id, std::vector<uint8_t>& out)
{
    for (auto& slot : slots_) 
    {
        if (slot.used && slot.id == id && !slot.responses.empty()) 
        {
            out = std::move(slot.responses.front());
            slot.responses.erase(slot.responses.begin());
            
            if (slot.responses.empty()) 
            {
                slot.used = false;
            }
            return true;
        }
    }
    return false;
}

// static-global instance
static Client_RPC* g_client_instance = nullptr;

void Client_RPC::register_global(Client_RPC* inst) 
{
    g_client_instance = inst;
}

Client_RPC* Client_RPC::global() 
{
    return g_client_instance;
}