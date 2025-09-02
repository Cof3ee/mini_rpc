#pragma once

#include <string>
#include <functional>
#include <cstdint>

#include "transport.h"

using Rpc_Handler = std::function<std::vector<uint8_t>(const std::vector<uint8_t>&)>;

struct Function_Registry_Item
{
    static constexpr std::size_t NAME_MAX = 8; 
    char name[NAME_MAX];    
    Rpc_Handler handler;
};

class registry_function
{
public:
    void register_func(const std::string& name, Rpc_Handler h);
    bool has(const std::string& name) const;
    bool call(const std::string& name, const std::vector<uint8_t>& args,
              std::vector<uint8_t>& out_result) const;

private:
    static constexpr size_t MAX_FUNCS = 8;
    Function_Registry_Item table[MAX_FUNCS];
    size_t count = 0;
};

// Incoming transport message (REQ) handler.
// Returns a transport message of type RESP (success) or ERR (error).
transport_message handle_request(const transport_message& req, const registry_function& reg);