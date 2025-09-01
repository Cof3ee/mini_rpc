#include <UART_TX.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"
#include "stm32f0xx_hal.h"

#include <new>      
#include <cstdint>

using namespace std;

static constexpr int TX_QUEUE_LEN = 8;            
static constexpr int TX_TASK_STACK = 64;         
static constexpr UBaseType_t TX_TASK_PRIO = tskIDLE_PRIORITY + 2;
static constexpr TickType_t TX_WAIT_TICKS = pdMS_TO_TICKS(5000); 


static QueueHandle_t tx_queue = nullptr;          
static TaskHandle_t tx_task_handle = nullptr;     
static TaskHandle_t tx_task_handle_for_isr_notify = nullptr; 


extern UART_HandleTypeDef huart1;

static void tx_task(void* pv)
{
    for (;;) 
    {
        std::vector<uint8_t>* pframe = nullptr;

        
        if (xQueueReceive(tx_queue, &pframe, portMAX_DELAY) == pdTRUE) 
        {
            if (pframe == nullptr) continue;

            uint16_t len = static_cast<uint16_t>(pframe->size());
            uint8_t* data = const_cast<uint8_t*>(pframe->data());

            
            if (HAL_UART_Transmit_DMA(&huart1, data, len) == HAL_OK) 
            {
                
                (void) ulTaskNotifyTake(pdTRUE, TX_WAIT_TICKS);
            } 
            else 
            {

                HAL_StatusTypeDef st = HAL_UART_Transmit(&huart1, data, len, 1000);
                (void)st; 
            }
            delete pframe;
        }
    }
}


extern "C" void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart == &huart1) 
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        vTaskNotifyGiveFromISR(tx_task_handle_for_isr_notify, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
}


void tx_comm_init()
{
    tx_queue = xQueueCreate(TX_QUEUE_LEN, sizeof(vector<uint8_t>*));
    configASSERT(tx_queue != nullptr);

    BaseType_t rc = xTaskCreate(tx_task, "TX_TASK", TX_TASK_STACK, nullptr, TX_TASK_PRIO, &tx_task_handle);
    configASSERT(rc == pdPASS);

    tx_task_handle_for_isr_notify = tx_task_handle;
}


bool send_packet(const vector<uint8_t>& packet)
{
    if (packet.empty()) return true; 

    if (packet.size() > static_cast<size_t>(numeric_limits<uint16_t>::max())) 
    {
        return false; 
    }


    vector<uint8_t>* p = new (nothrow) vector<uint8_t>(packet);
    if (!p) 
    {
        return false; 
    }
    
    BaseType_t ok = xQueueSend(tx_queue, &p, 0); 
    if (ok != pdTRUE) 
    {

        delete p;
        return false;
    }

    return true;
}