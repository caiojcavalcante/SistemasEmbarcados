/**
 * @file ble_peripheral.h
 * @author Jeferson Fernando (jfss@ic.ufal.br)
 * @brief Interface de implementação de stack bluetooth com funcionalidade BLE
 * UART Peripheral.
 * @version 0.1
 * @date 2022-11-02
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef BLE_PERIPHERAL_H_
#define BLE_PERIPHERAL_H_

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gap.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/services/hrs.h>
#include <bluetooth/uuid.h>
#include <sys/printk.h>
#include <sys/util.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "stdint.h"
#include "stdlib.h"
#include "string.h"

/**
 * @brief Valor do UUID do serviço BLE UART.
 *
 */
#define BLE_UART_UUID_SVC_VAL 0x2BC4

/**
 * @brief UUID do serviço BLE UART.
 *
 */
#define BLE_UART_SVC_UUID BT_UUID_DECLARE_16(BLE_UART_UUID_SVC_VAL)

/**
 * @brief Valor do UUID da característica de Notify do BLE UART.
 *
 */
#define BLE_UART_NOTIFY_CHAR_UUID_VAL 0x2BC5

/**
 * @brief UUID da caracterísitica de Notify do BLE UART.
 *
 */
#define BLE_UART_NOTIFY_CHAR_UUID                                              \
  BT_UUID_DECLARE_16(BLE_UART_NOTIFY_CHAR_UUID_VAL)

/**
 * @brief Valor do UUID da característica de escrita do BLE UART.
 *
 */
#define BLE_UART_WRITE_CHAR_UUID_VAL 0x2BC6

/**
 * @brief UUID da característica de escrita do BLE UART.
 *
 */
#define BLE_UART_WRITE_CHAR_UUID                                               \
  BT_UUID_DECLARE_16(BLE_UART_WRITE_CHAR_UUID_VAL)

/**
 * @brief Inicializa a stack bluetooth com lógica BLE UART Peripheral.
 *
 * @return int 0 para sucesso e um inteiro negativo em caso de falha.
 */
int ble_peripheral_init(void);

#endif /* BLE_PERIPHERAL_H_ */