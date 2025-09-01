#include "transport.h"
#include "channel.h"
#include "registry.h"

#include <iostream>

// Пример демонстрации работы аппаратно-независимого ядра.
//  - Регистрируем функцию sum,
//  - Клиент формирует REQ (sum, args),
//  - Сервер принимает кадр, распаковывает, вызывает функцию и формирует RESP,
//  - Клиент распаковывает ответ и выводит результат.
//
// Это эмуляция loopback: все байты передаются внутри процесса.
int main() 
{
    registry_function reg;

    // Регистрируем функцию 'sum' — соглашение: два байта, результат в одном байте
    reg.register_func("sum", [](const std::vector<uint8_t>& args)->std::vector<uint8_t> 
        {
        if (args.size() < 2) return { 0xFF }; // ошибка формата аргументов
        uint8_t a = args[0];
        uint8_t b = args[1];
        uint8_t s = static_cast<uint8_t>(a + b);
        return { s };
        });

    // --- Клиент формирует запрос ---
    transport_message req;
    req.type = MSG_REQ;
    req.id = 7;
    req.name = "sum";
    req.data = { 5, 7 }; // 5 + 7

    // Сериализация транспортного сообщения (payload) и упаковка в кадр (frame)
    auto payload = transport_encode(req);
    auto frame = channel_encode(payload);

    // --- Серверная сторона: по-байтово подаём frame в ChannelDecoder ---
    ChannelDecoder decoder;
    std::vector<uint8_t> got_payload;
    for (uint8_t b : frame) 
    {
        if (decoder.feed(b, got_payload)) {
            // получили payload — десериализуем транспорт и обрабатываем
            auto parsed = transport_decode(got_payload);
            auto resp = handle_request(parsed, reg);
            auto resp_payload = transport_encode(resp);
            auto resp_frame = channel_encode(resp_payload);

            // --- Клиент принимает ответ (эмуляция) ---
            ChannelDecoder dec2;
            std::vector<uint8_t> got2;
            for (uint8_t rb : resp_frame) 
            {
                if (dec2.feed(rb, got2)) 
                {
                    auto parsed_resp = transport_decode(got2);
                    std::cout << "Response for id=" << int(parsed_resp.id) << ", name=" << parsed_resp.name << " : ";
                    for (auto v : parsed_resp.data) std::cout << int(v) << " ";
                    std::cout << std::endl;
                }
            }
        }
    }

    return 0;
}
