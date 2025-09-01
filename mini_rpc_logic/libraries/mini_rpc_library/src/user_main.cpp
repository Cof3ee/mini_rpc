#include "user_main.h"

#include "server_rpc.h"
#include "client_rpc.h"
#include "uart_rx.h"
#include "transport.h"
#include <cstdint>

static std::vector<uint8_t> rpc_sum(const std::vector<uint8_t>& args)
{
    if (args.size() < 2)
        return {0}; 

    uint8_t a = args[0];
    uint8_t b = args[1];
    uint8_t res = static_cast<uint8_t>(a + b);

    return { res }; 
}

void user_main_function(void) 
{
  // 1. Create and fill the registry
    registry_function reg;
    reg.register_func("sum", rpc_sum);

    // 2. Create a server
    static Server_RPC server(reg);

    // 3. Create a client
    static Client_RPC client;
    Client_RPC::register_global(&client);
    
    // 4. Initialize UART RX (the reception and link task will start)
    uart_rx_init(&server);

    // 5. Send a test request sum(3,4)
    client.send_msg("sum", 3, 4, MSG_REQ);
}