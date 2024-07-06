#include "driver/gpio.h"
#include "driver/twai.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t receive_task_handle = NULL;

void receive_message(void *arg)
{
  while (true)
  {
    // Wait for message to be received
    twai_message_t message;
    if (twai_receive(&message, pdMS_TO_TICKS(10000)) == ESP_OK)
    {
      // Process received message
      // if (message.extd)
      // {
      //   printf("Message is in Extended Format\n");
      // }
      // else
      // {
      //   printf("Message is in Standard Format\n");
      // }
      printf("%ld  ", message.identifier);
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
      printf("No message received.\n");
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
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

  xTaskCreatePinnedToCore(receive_message, "receive_message", 4096, NULL, 10, &receive_task_handle, 1);
}