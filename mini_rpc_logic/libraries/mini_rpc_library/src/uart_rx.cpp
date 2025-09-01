#include "uart_rx.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

extern "C" 
{
#include "stm32f0xx_hal.h"
}

#include <vector>
#include <string>
#include <cstdint>

static constexpr size_t RX_BYTE_QUEUE_LEN = 32;
static constexpr uint16_t RPC_TASK_STACK   = 64;
static constexpr UBaseType_t RPC_TASK_PRIO = (tskIDLE_PRIORITY + 2);

extern UART_HandleTypeDef huart1;


extern bool send_packet(const std::vector<uint8_t>& packet);

extern void on_response(const transport_message& msg);


static QueueHandle_t rx_byte_queue = nullptr;
static TaskHandle_t rpc_link_task_handle = nullptr;
static uint8_t rx_byte = 0; // buffer for HAL_UART_Receive_IT
static ChannelDecoder decoder;

// Pointer to Server_RPC (injection via uart_rx_init)
static Server_RPC* g_server_ptr = nullptr;

// ---------- HAL callback (ISR) ----------
extern "C" void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart != &huart1) return;

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if (rx_byte_queue != nullptr) {
        xQueueSendFromISR(rx_byte_queue, &rx_byte, &xHigherPriorityTaskWoken);
    }

    // Rearm the reception of the next byte
    HAL_UART_Receive_IT(huart, &rx_byte, 1);

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

// Input Stream Processing Task 
static void rpc_link_task(void* pv)
{
    (void)pv;
    uint8_t b;
    std::vector<uint8_t> payload;

    for (;;) 
    {
        if (xQueueReceive(rx_byte_queue, &b, portMAX_DELAY) == pdTRUE) 
        {
            // We feed a byte into the channel layer decoder
            if (decoder.feed(b, payload)) 
            {
                // Received a valid payload (transport message)
                // Transport Layer Deserialization
                transport_message msg = transport_decode(payload);

                // Processing depending on message type
                if (msg.type == MSG_REQ) 
                {
                    // Calling a server handler using registry_ inside Server_RPC
                    if (g_server_ptr != nullptr) 
                    {
                        transport_message resp = g_server_ptr->process_transport_message(msg);

                        // Serialization and frame generation of the data link layer
                        std::vector<uint8_t> payload_out = transport_encode(resp);
                        std::vector<uint8_t> frame = channel_encode(payload_out);

                        send_packet(frame);
                    } 
                }
                else if (msg.type == MSG_RESP || msg.type == MSG_ERR) 
                {
                    // We pass the received response to the client side
                    on_response(msg);
                }
                else if (msg.type == MSG_STREAM) 
                {
                  
                  if (g_server_ptr != nullptr) 
                  {
                  // Receive one response from the server handler
                  transport_message resp = g_server_ptr->process_transport_message(msg);

                  
                  resp.type = MSG_RESP;
                  resp.id   = msg.id;

                  // Serialize and encode the link layer frame once
                  std::vector<uint8_t> payload_out = transport_encode(resp);
                  std::vector<uint8_t> frame = channel_encode(payload_out);

                  // Send three times (short pause between sendings)
                  for (int i = 0; i < 3; ++i) 
                  {
                      send_packet(frame); 
                      vTaskDelay(pdMS_TO_TICKS(10));
                  }
                }
            }
        }
    }
  }
}

void uart_rx_init(Server_RPC* server)
{
    // Save a pointer to Server_RPC
    g_server_ptr = server;

    
    if (rx_byte_queue == nullptr) 
    {
        rx_byte_queue = xQueueCreate(RX_BYTE_QUEUE_LEN, sizeof(uint8_t));
        configASSERT(rx_byte_queue != nullptr);
    }

    if (rpc_link_task_handle == nullptr) 
    {
        BaseType_t rc = xTaskCreate(rpc_link_task, "RPC_LINK", RPC_TASK_STACK, nullptr, RPC_TASK_PRIO, &rpc_link_task_handle);
        configASSERT(rc == pdPASS);
    }

    // Start receiving 1 byte in IT mode (rearming is performed in ISR)
    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);
}

void uart_rx_deinit()
{
    // Stop receiving
    HAL_UART_AbortReceive(&huart1);

    // Delete task
    if (rpc_link_task_handle != nullptr) 
    {
        vTaskDelete(rpc_link_task_handle);
        rpc_link_task_handle = nullptr;
    }

    // Delete queue
    if (rx_byte_queue != nullptr) 
    {
        vQueueDelete(rx_byte_queue);
        rx_byte_queue = nullptr;
    }

    g_server_ptr = nullptr;
}