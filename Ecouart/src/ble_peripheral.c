#include "ble_peripheral.h"
#if BLE_PERIPHERAL == 1

#define BLE_PERIPHERAL_NAME "BLE PERIPHERAL"
#define BLE_PERIPHERAL_NAME_LEN (sizeof(BLE_PERIPHERAL_NAME) - 1)

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx);

static struct bt_gatt_cb gatt_callbacks = {
    .att_mtu_updated = mtu_updated,
};

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
};

struct bt_conn *default_conn;

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, BLE_PERIPHERAL_NAME,
            BLE_PERIPHERAL_NAME_LEN),
};

static void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx) {
  printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

static void connected(struct bt_conn *conn, uint8_t err) {
  if (err) {
    printk("Connection failed (err %u)\n", err);
  } else {
    default_conn = bt_conn_ref(conn);
  }
}

static void disconnected(struct bt_conn *conn, uint8_t reason) {
  printk("Disconnected (reason %u)\n", reason);

  if (default_conn) {
    bt_conn_unref(default_conn);
    default_conn = NULL;
  }
}

static void ble_central_ready(int err) {
  if (err) {
    printk("Bluetooth init failed (err %d)\n", err);
    return;
  }

  err = bt_le_adv_start(
      BT_LE_ADV_PARAM(BT_LE_PER_ADV_OPT_NONE, BT_GAP_ADV_FAST_INT_MIN_2,
                      BT_GAP_ADV_FAST_INT_MAX_2, BT_GAP_PER_ADV_FAST_INT_MIN_1),
      ad, ARRAY_SIZE(ad), NULL, 0);
  if (err) {
    printk("Advertising failed to start (err %d)\n", err);
    return;
  }

  printk("BLE Central started advertising.\n");
}

int ble_peripheral_init() {
  int err = 0;

  /* Incializa Bluetooth. */
  err = bt_enable(ble_central_ready);
  if (err) {
    printk("Bluetooth init failed (err %d)\n", err);
    return 0U;
  }

  printk("Bluetooth initialized\n");

  /* Localiza dispositivos periféricos. */
  bt_conn_cb_register(&conn_callbacks);
  bt_gatt_cb_register(&gatt_callbacks);

  return 0;
}
#endif /* BLE_PERIPHERAL */