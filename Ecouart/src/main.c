#include <zephyr.h>

#include "ble_central.h"
#include "ble_peripheral.h"

void main(void) {
  int err = 0;

#if BLE_CENTRAL == 1
  err = ble_central_init();
#elif BLE_PERIPHERAL == 1
  err = ble_peripheral_init();
#else
#error "Define if your device is peripheral or central."
#endif
}
