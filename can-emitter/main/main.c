#include "driver/gpio.h"
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t transmit_task_handle = NULL;

void transmit_message(void *arg)
{
  while (true)
  {
    twai_message_t message;
    message.identifier = rand() & 0xFF;
    message.extd = 0;
    message.data_length_code = 4;
    for (int i = 0; i < 4; i++)
    {
      message.data[i] = 0;
    }

    // Queue message for transmission
    if (twai_transmit(&message, portMAX_DELAY) == ESP_OK)
    {
      printf("Message queued for transmission\n");
    }
    else
    {
      printf("Failed to queue message for transmission\n");
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void app_main(void)
{
  // Initialize configuration structures using macro initializers
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_21, GPIO_NUM_22, TWAI_MODE_NORMAL);
  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  // Install TWAI driver
  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK)
  {
    printf("Driver installed\n");
  }
  else
  {
    printf("Failed to install driver\n");
    return;
  }

  // Start TWAI driver
  if (twai_start() == ESP_OK)
  {
    printf("Driver started\n");
  }
  else
  {
    printf("Failed to start driver\n");
    return;
  }

  xTaskCreatePinnedToCore(transmit_message, "transmit_message", 8192, NULL, 10, &transmit_task_handle, 1);
}