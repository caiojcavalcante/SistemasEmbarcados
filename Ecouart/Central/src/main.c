/**
 * @file main.c
 * @author Jeferson Fernando (jfss@ic.ufal.br)
 * @brief Main do projeto BLE Central Uart.
 * @version 0.1
 * @date 2022-11-02
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <sys/printk.h>
#include <zephyr.h>

#include "ble_central.h"
#include "message_receptor.h"

void main(void) {
  int err = 1;

  /* Inicializa lógica do Central. */
  err = ble_central_init();
  if (err) {
    printk("|BLE CENTRAL| Error initializing Central.\n");
  }
}