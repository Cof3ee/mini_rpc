#pragma once

#include "registry.h"
#include "transport.h"

class Server_RPC
{
public:
	explicit Server_RPC(const registry_function& reg);

        // getter for accessing the registry (used when processing requests)
        const registry_function& registry() const { return registry_; }
        registry_function& registry() { return registry_; }
        
        // Process the transport message (REQ) and return the generated response (RESP/ERR).
        transport_message process_transport_message(const transport_message& req) const;
	
        void on_rx();
	void off_rx();

private:

	registry_function registry_;
	bool rx_;
};