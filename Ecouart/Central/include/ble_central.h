/**
 * @file ble_central.h
 * @author Jeferson Fernando (jfss@ic.ufal.br)
 * @brief Interface de implementação de stack bluetooth com funcionalidade BLE
 * UART Central.
 * @version 0.1
 * @date 2022-11-02
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef BLE_CENTRAL_H_
#define BLE_CENTRAL_H_

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <sys/byteorder.h>
#include <sys/printk.h>
#include <zephyr.h>

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
 * @brief Escreve na característica BLE UART WRITE.
 *
 * @param buf [in] Ponteiro para buffer que contém dados a serem transmitidos.
 * @param buf_len Tamanho do buffer que contém dados a serem transmitidos.
 * @return int 0 para sucesso e um inteiro negativo em caso de falha.
 */
int ble_central_write_input(uint8_t *buf, uint16_t buf_len);

/**
 * @brief Inicializa a stack bluetooth com lógica BLE UART Central.
 *
 * @return int 0 para sucesso e um inteiro negativo em caso de falha.
 */
int ble_central_init(void);

#endif /* BLE_CENTRAL_H_ */