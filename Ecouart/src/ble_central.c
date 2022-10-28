#if BLE_CENTRAL == 1
#include "ble_central.h"

#define BLE_CENTRAL_NAME "BLE CENTRAL"
#define BLE_CENTRAL_NAME_LEN (sizeof(BLE_CENTRAL_NAME) - 1)

static void connected(struct bt_conn *conn, uint8_t err);
static void disconnected(struct bt_conn *conn, uint8_t reason);
static void search_for_peripherals(int err);

void mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx) {
  printk("Updated MTU: TX: %d RX: %d bytes\n", tx, rx);
}

static struct bt_gatt_cb gatt_callbacks = {
    .att_mtu_updated = mtu_updated,
};

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
};

struct bt_conn *default_conn;

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *ad) {
  char dev[BT_ADDR_LE_STR_LEN];
  struct bt_conn *conn;
  int err;

  bt_addr_le_to_str(addr, dev, sizeof(dev));
  printk("[DEVICE]: %s, AD evt type %u, AD data len %u, RSSI %i\n", dev, type,
         ad->len, rssi);

  /* We're only interested in connectable events */
  if (type != BT_GAP_ADV_TYPE_ADV_IND &&
      type != BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
    return;
  }

  err = bt_le_scan_stop();
  if (err) {
    printk("%s: Stop LE scan failed (err %d)\n", __func__, err);
    return;
  }

  err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN,
                          BT_LE_CONN_PARAM_DEFAULT, &conn);
  if (err) {
    err = bt_le_scan_start(BT_LE_SCAN_ACTIVE, device_found);
    if (err) {
      printk("%s: Scanning failed to start (err %d)\n", __func__, err);
      return;
    }
    printk("%s: Scanning successfully started\n", __func__);
  } else {
    bt_conn_unref(conn);
  }
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

static void search_for_peripherals(int err) {

  err = bt_le_scan_start(BT_LE_SCAN_ACTIVE, device_found);
  if (err) {
    printk("%s: Scanning failed to start (err %d)\n", __func__, err);
    return;
  }

  printk("%s: Scanning successfully started\n", __func__);
}

int ble_central_init() {
  int err = 0;

  /* Incializa Bluetooth. */
  err = bt_enable(search_for_peripherals);
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

#endif /* BLE_CENTRAL */