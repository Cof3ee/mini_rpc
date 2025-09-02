/*#include <registry.h>
#include "transport.h"

#include <stdexcept>


using namespace std;

void registry_function::register_func(const string &name, Rpc_Handler h) 
{
    // Let's check if it's already registered
    for (size_t i = 0; i < count; ++i)
    {
        if (table[i].name && name == table[i].name)
        {
            table[i].handler = move(h); // update
            return;
        }
    }

    // Add new if there is space
    if (count < MAX_FUNCS)
    {
        table[count].name = name.c_str();   
                                             
        table[count].handler = move(h);
        ++count;
    }
}

bool registry_function::has(const string &name) const
{
    for (size_t i = 0; i < count; ++i)
    {
        if (table[i].name && name == table[i].name)
        {
            return true;
        }
    }
    return false;
}

bool registry_function::call(const string &name,
                             const vector<uint8_t> &args,
                             vector<uint8_t> &out_result) const
{
    for (size_t i = 0; i < count; ++i)
    {
        if (table[i].name && name == table[i].name)
        {
            if (!table[i].handler)
                return false;

            out_result = table[i].handler(args);
            return true;
        }
    }
    return false;
}

transport_message handle_request(const transport_message& req, const registry_function& reg) 
{
    transport_message out;
    out.id = req.id;
    out.name = req.name;

    if (req.type != MSG_REQ) 
    {
        out.type = MSG_ERR;
        out.data = {'B','A','D'}; 
        return out;
    }

    if (!reg.has(req.name)) 
    {
        out.type = MSG_ERR;
        out.data = {'N','O','T','F'}; // not found
        return out;
    }

    vector<uint8_t> result;
    bool ok = reg.call(req.name, req.data, result);

    if (!ok) 
    {
        out.type = MSG_ERR;
        out.data = {'E','R','R'}; 
        return out;
    }

    out.type = MSG_RESP;
    out.data = move(result);
    return out;
}
*/

#include "registry.h"
#include "transport.h"

#include <cstring>
#include <utility>

using namespace std;

void registry_function::register_func(const string &name, Rpc_Handler h)
{
    for (size_t i = 0; i < count; ++i)
    {
        if (table[i].name[0] != '\0' && name == table[i].name)
        {
            table[i].handler = std::move(h);
            return;
        }
    }

    if (count < MAX_FUNCS)
    {
        size_t copy_len = name.size();
        if (copy_len >= Function_Registry_Item::NAME_MAX)
            copy_len = Function_Registry_Item::NAME_MAX - 1;

        memcpy(table[count].name, name.data(), copy_len);
        table[count].name[copy_len] = '\0';

        table[count].handler = std::move(h);
        ++count;
    }
}

bool registry_function::has(const string &name) const
{
    for (size_t i = 0; i < count; ++i)
    {
        if (name == table[i].name)
        {
            return true;
        }
    }
    return false;
}

bool registry_function::call(const string &name,
                             const vector<uint8_t> &args,
                             vector<uint8_t> &out_result) const
{
    for (size_t i = 0; i < count; ++i)
    {
        if (name == table[i].name)
        {
            if (!table[i].handler)
                return false;

            out_result = table[i].handler(args);
            return true;
        }
    }
    return false;
}

transport_message handle_request(const transport_message& req, const registry_function& reg)
{
    transport_message out;
    out.id = req.id;
    out.name = req.name;

    if (req.type != MSG_REQ)
    {
        out.type = MSG_ERR;
        out.data = {'B','A','D'};
        return out;
    }

    if (!reg.has(req.name))
    {
        out.type = MSG_ERR;
        out.data = {'N','O','T','F'}; // not found
        return out;
    }

    vector<uint8_t> result;
    bool ok = reg.call(req.name, req.data, result);

    if (!ok)
    {
        out.type = MSG_ERR;
        out.data = {'E','R','R'};
        return out;
    }

    out.type = MSG_RESP;
    out.data = move(result);
    return out;
}
