#include "server_rpc.h"

void Server_RPC::on_rx()
{
	rx_ = true;
}

void Server_RPC::off_rx()
{
	rx_ = false;
}

Server_RPC::Server_RPC(const registry_function& reg) : registry_(reg), rx_(0)
{}

transport_message Server_RPC::process_transport_message(const transport_message& req) const
{
    // Delegate the formation of the response to the existing handle_request function
    return ::handle_request(req, registry_);
}