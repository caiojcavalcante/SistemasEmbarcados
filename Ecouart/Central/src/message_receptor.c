/**
 * @file message_receptor.c
 * @author Jeferson Fernando (jfss@ic.ufal.br)
 * @brief Implementação de tarefa para entrada de dados do usuário.
 * @version 0.1
 * @date 2022-11-02
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "message_receptor.h"

/**
 * @brief Tarefa que executa recepção da entrada e envio via Bluetooth.
 *
 */
static void input_task(void);

/**
 * @brief Define a tarefa de entrada.
 *
 */
K_THREAD_DEFINE(input, 1024, input_task, NULL, NULL, NULL, 1, 0, 1000);

static void input_task(void) {
  int err = 0;
  char *recvd_line = NULL;

  console_getline_init();

  while (true) {

    k_sleep(K_MSEC(100));

    printk("|BLE CENTRAL| Enter a line:");
    recvd_line = console_getline();

    if (recvd_line == NULL) {
      printk("|BLE CENTRAL| Error receiving line!\n");
      continue;
    }

    printk("|BLE CENTRAL| Sending line:%s\n", recvd_line);

    err = ble_central_write_input(recvd_line, strlen(recvd_line));
  }
}