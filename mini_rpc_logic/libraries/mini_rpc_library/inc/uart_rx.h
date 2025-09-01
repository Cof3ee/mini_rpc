#pragma once

#include "channel.h"    
#include "transport.h"  
#include "registry.h"
#include "server_rpc.h"
#include <client_rpc.h>

#include "UART_TX.h"

#ifdef __cplusplus
extern "C" 
{
#endif

// Initialize UART reception/processing
void uart_rx_init(Server_RPC* server);

// Deinitialize (stop receiving and delete queue/task)
void uart_rx_deinit();

#ifdef __cplusplus
}
#endif