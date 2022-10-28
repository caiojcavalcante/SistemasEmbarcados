#ifndef BLE_CENTRAL_H_
#define BLE_CENTRAL_H_
#if BLE_CENTRAL == 1

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gatt.h>
#include <zephyr.h>

#include "stdint.h"
#include "stdlib.h"
#include "string.h"

int ble_central_init(void);

#endif /* BLE_CENTRAL */
#endif /* BLE_CENTRAL_H_ */