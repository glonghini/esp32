#include <stdio.h>
#include "esp_log.h"
#include "esp_err.h"
#include "esp_console.h"

void app_main(void)
{
  esp_console_repl_t *repl = NULL;
  esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
  repl_config.prompt = ">";

  // init console REPL environment
  esp_console_dev_uart_config_t uart_config = ESP_CONSOLE_DEV_UART_CONFIG_DEFAULT();
  ESP_ERROR_CHECK(esp_console_new_repl_uart(&uart_config, &repl_config, &repl));

  printf("\n ==================================================\n");
  printf(" |       Steps to test WiFi throughput            |\n");
  printf(" |                                                |\n");
  printf(" |  1. Print 'help' to gain overview of commands  |\n");
  printf(" |  2. Configure device to station or soft-AP     |\n");
  printf(" |  3. Setup WiFi connection                      |\n");
  printf(" |  4. Run iperf to test UDP/TCP RX/TX throughput |\n");
  printf(" |                                                |\n");
  printf(" =================================================\n\n");

  // start console REPL
  ESP_ERROR_CHECK(esp_console_start_repl(repl));
}