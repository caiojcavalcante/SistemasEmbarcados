#ifndef BLE_PERIPHERAL_H_
#define BLE_PERIPHERAL_H_
#if BLE_PERIPHERAL == 1

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/gap.h>
#include <bluetooth/gatt.h>
#include <bluetooth/hci.h>
#include <bluetooth/uuid.h>
#include <sys/util.h>
#include <zephyr.h>
#include <zephyr/types.h>

#include "stdint.h"
#include "stdlib.h"
#include "string.h"

int ble_peripheral_init(void);

#endif /* BLE_PERIPHERAL */
#endif /* BLE_PERIPHERAL_H_ */