/**
 * @file main.c
 * @author Jeferson Fernando (jfss@ic.ufal.br)
 * @brief Main do projeto BLE Peripheral Uart
 * @version 0.1
 * @date 2022-11-01
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <sys/printk.h>
#include <zephyr.h>

#include "ble_peripheral.h"

void main(void) {
  int err = 1;

  /* Inicializa lógica do Peripheral. */
  err = ble_peripheral_init();

  if (err) {
    printk("|BLE PERIPHERAL| Error initializing Peripheral.\n");
  }
}
