#include "driver/gpio.h"
#include "driver/twai.h"
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include "sdkconfig.h"
#include "esp_log.h"

#define ARR_SIZE(arr) (sizeof((arr)) / sizeof((arr[0])))
#define BUF_SIZE (1024 * 2)
#define RXD 3
#define TXD 1
#define UART_PORT UART_NUM_0

// Setup UART buffered IO with event queue
// const int uart_buffer_size = (1024 * 2);
QueueHandle_t uart_queue;

TaskHandle_t read_serial_handle = NULL;
TaskHandle_t transmit_message_handle = NULL;
TaskHandle_t receive_message_handle = NULL;

// CAN
twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);
twai_timing_config_t t_config = TWAI_TIMING_CONFIG_100KBITS();
twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

// UART
uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
};

int pids[6] = {0x100, 0x101, 0x102, 0x103, 0x104, 0x105};

void read_serial(void *arg)
{
  // Configure a temporary buffer for the incoming data
  uint8_t data[128];

  while (true)
  {
    // Read data from the UART
    int length = uart_read_bytes(UART_PORT, data, (BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);
    // Write data back to the UART
    uart_write_bytes(UART_PORT, (const char *)data, length);
  }
}

void transmit_message(void *arg)
{
  // Configure a temporary buffer for the incoming data
  uint8_t data[128];
  twai_message_t message;

  while (true)
  {
    // Read data from the UART
    int length = uart_read_bytes(UART_PORT, data, (BUF_SIZE - 1), pdMS_TO_TICKS(20));

    if (length)
    {
      // Write data back to the UART
      // uart_write_bytes(UART_PORT, (const char *)data, length);
      for (int v = 0; v < length; v++)
      {
        printf("%02X\n", data[v]);
      }

      message.identifier = data[0] * 0x100 + data[1];
      message.extd = 0;
      message.data_length_code = data[2];
      for (int i = 0; i < data[2]; i++)
      {
        message.data[i] = data[3 + i];
      }

      if (twai_transmit(&message, portMAX_DELAY) == ESP_OK)
      {
        printf("Message transmitted\n");
      }
      else
      {
        printf("num deu\n");
      }
    }
  }
}

void receive_message(void *arg)
{
  twai_message_t message;

  while (true)
  {
    if (twai_receive(&message, pdMS_TO_TICKS(10000)) == ESP_OK)
    {
      printf("%ld %ld ", xTaskGetTickCount(), message.identifier);
      if (!(message.rtr))
      {
        for (int i = 0; i < message.data_length_code; i++)
        {
          printf("%d ", message.data[i]);
        }
      }
      printf("\n");
    }
    else
    {
      printf("No message received\n");
    }
  }
}

void app_main(void)
{
  // Initializing CAN
  ESP_ERROR_CHECK(twai_driver_install(&g_config, &t_config, &f_config));
  ESP_ERROR_CHECK(twai_start());

  // Initializing UART
  ESP_ERROR_CHECK(uart_driver_install(UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0));
  ESP_ERROR_CHECK(uart_set_pin(UART_PORT, TXD, RXD, 18, 19));
  ESP_ERROR_CHECK(uart_param_config(UART_PORT, &uart_config));

  xTaskCreatePinnedToCore(receive_message, "receive_message", 4096, NULL, 10, &receive_message_handle, 1);
  // xTaskCreatePinnedToCore(read_serial, "read_serial", 4096, NULL, 10, &read_serial_handle, 1);
  xTaskCreatePinnedToCore(transmit_message, "transmit_message", 4096, NULL, 10, &transmit_message_handle, 1);
}