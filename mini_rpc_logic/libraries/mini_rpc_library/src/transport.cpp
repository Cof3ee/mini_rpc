#include <transport.h>

std::vector<uint8_t> transport_encode(const transport_message& msg)
{
    std::vector<uint8_t> out;
    
    out.reserve(2 + msg.name.size() + 1 + msg.data.size());
    out.push_back(msg.type);
    out.push_back(msg.id);
    out.insert(out.end(), msg.name.begin(), msg.name.end());
    out.push_back(0x00); // терминатор имени
    out.insert(out.end(), msg.data.begin(), msg.data.end());
    
    return out;
}

transport_message transport_decode(const std::vector<uint8_t>& buf) 
{
     transport_message msg;

    if (buf.size() < 3) 
    {
        msg.type = MSG_ERR;
        msg.data = {'B','A','D'};
        return msg;
    }

    msg.type = buf[0];
    msg.id   = buf[1];

    std::size_t pos = 2;
    while (pos < buf.size() && buf[pos] != 0x00) 
    {
        msg.name.push_back(static_cast<char>(buf[pos]));
        ++pos;
    }

    if (pos == buf.size()) 
    {
        msg.type = MSG_ERR;
        msg.name.clear();
        msg.data = {'B','A','D'};
        return msg;
    }
    ++pos;

    if (pos < buf.size()) 
    {
        msg.data.insert(msg.data.end(), buf.begin() + pos, buf.end());
    }

    return msg;
}
