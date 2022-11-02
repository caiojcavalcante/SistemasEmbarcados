/**
 * @file ble_peripheral.c
 * @author Jeferson Fernando (jfss@ic.ufal.br)
 * @brief Implementação de stack bluetooth com funcionalidade BLE UART
 * Peripheral.
 * @version 0.1
 * @date 2022-11-02
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "ble_peripheral.h"

/**
 * @brief Callaback que trata alteração nas configurações do servico.
 *
 * @param attr  [in] Ponteiro para estrutura do atributo atualizado.
 * @param value Bitfield das configurações atualizadas.
 */
static void ble_peripheral_cfg_changed(const struct bt_gatt_attr *attr,
                                       uint16_t value);

/**
 * @brief Callback que trata a escrita em característica.
 *
 * @param conn [in] Ponteiro para estrutura de handle de conexão.
 * @param attr [in] Ponteiro para estrutura do atributo atualizado.
 * @param buf [in]  Ponteiro para buffer dos dados escritos.
 * @param len Tamanho do buffer dos dados escritos.
 * @param offset Offset de escrita.
 * @param flags Flags que indicam o modo de escrita.
 * @return int 0 para sucesso e um inteiro negativo em caso de falha.
 */
static int ble_peripheral_write_uart(struct bt_conn *conn,
                                     const struct bt_gatt_attr *attr,
                                     const void *buf, uint16_t len,
                                     uint16_t offset, uint8_t flags);

/**
 * @brief Callback que trata a stack bluetooth atualizando tamanho da MTU.
 *
 * @param conn [in] Ponteiro para estrutura de handle de conexão.
 * @param tx  Tamanho da MTU TX.
 * @param rx  Tamanho da MTU RX.
 */
static void ble_peripheral_mtu_updated(struct bt_conn *conn, uint16_t tx,
                                       uint16_t rx);

/**
 * @brief Callback que trata a stack bluetooth após uma conexão.
 *
 * @param conn [in] Ponteiro para estrutura de handle de conexão.
 * @param err Indica se houve erro durante pareamento.
 */
static void ble_peripheral_connected(struct bt_conn *conn, uint8_t err);

/**
 * @brief Callback que trata a stack bluetooth após uma desconexão.
 *
 * @param conn [in] Ponteiro para estrutura de handle de conexão.
 * @param reason Indica causa da desconexão.
 */
static void ble_peripheral_disconnected(struct bt_conn *conn, uint8_t reason);

/**
 * @brief Callback que trata a stack bluetooth após o mesmo estar pronto.
 *
 * @param init_err Indica se houve erro na inicialização.
 */
static void ble_peripheral_ready(int init_err);

/**
 * @brief Estrutura interna de variáveis.
 *
 */
static struct {
  struct bt_gatt_cb gatt_callbacks; /* Estrutura de callbacks de GATT. */
  struct bt_conn_cb conn_callbacks; /* Estrutura de callbacks de conexão. */
  struct bt_conn *default_conn; /* Ponteiro para handle de conexões ativas. */
} self = {
    .gatt_callbacks =
        {
            .att_mtu_updated = ble_peripheral_mtu_updated,
        },
    .conn_callbacks =
        {
            .connected = ble_peripheral_connected,
            .disconnected = ble_peripheral_disconnected,
        },
    .default_conn = NULL,
};

/**
 * @brief Define os dados condificados no adversiting.
 *
 */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA_BYTES(BT_DATA_UUID16_ALL,
                  BT_UUID_16_ENCODE(BLE_UART_UUID_SVC_VAL), ),
};

/**
 * @brief Define o serviço de BLE_UART.
 *
 */
BT_GATT_SERVICE_DEFINE(
    ble_uart_svc, BT_GATT_PRIMARY_SERVICE(BLE_UART_SVC_UUID),
    BT_GATT_CHARACTERISTIC(BLE_UART_NOTIFY_CHAR_UUID, BT_GATT_CHRC_NOTIFY,
                           BT_GATT_PERM_NONE, NULL, NULL, NULL),
    BT_GATT_CHARACTERISTIC(BLE_UART_WRITE_CHAR_UUID, BT_GATT_CHRC_WRITE,
                           BT_GATT_PERM_WRITE, NULL, ble_peripheral_write_uart,
                           NULL),
    BT_GATT_CCC(ble_peripheral_cfg_changed,
                (BT_GATT_PERM_READ | BT_GATT_PERM_WRITE)), );

static void ble_peripheral_cfg_changed(const struct bt_gatt_attr *attr,
                                       uint16_t value) {
  ARG_UNUSED(attr);

  bool notify_enabled = (value == BT_GATT_CCC_NOTIFY);

  printk("|BLE PERIPHERAL| Notify %s.\n",
         (notify_enabled ? "enabled" : "disabled"));
}

static int ble_peripheral_write_uart(struct bt_conn *conn,
                                     const struct bt_gatt_attr *attr,
                                     const void *buf, uint16_t len,
                                     uint16_t offset, uint8_t flags) {
  int err = 0;
  char data[len + 1];

  /* Copia dados recebidos. */
  memcpy(data, buf, len);
  data[len] = '\0';

  printk("|BLE PERIPHERAL| Received data %s.\n", data);

  /* Converte letras minúsculas para maiúsculas. */
  for (int i = 0; i < len; i++) {
    if ((data[i] >= 'a') && ((data[i] <= 'z'))) {
      data[i] = 'A' + (data[i] - 'a');
    }
  }

  printk("|BLE PERIPHERAL| Sending data %s.\n", data);

  /* Notifica Central com o dados convertidos. */
  err = bt_gatt_notify(NULL, &ble_uart_svc.attrs[1], data, len);
  if (err) {
    printk("|BLE PERIPHERAL| Error notifying.\n");
  }

  return 0;
}

static void ble_peripheral_mtu_updated(struct bt_conn *conn, uint16_t tx,
                                       uint16_t rx) {
  printk("|BLE PERIPHERAL| Updated MTU. TX:%d RX:%d bytes.\n", tx, rx);
}

static void ble_peripheral_connected(struct bt_conn *conn, uint8_t err) {
  if (err) {
    printk("|BLE PERIPHERAL| Peripheral Connection failed (err %u).\n", err);
  } else {
    self.default_conn = bt_conn_ref(conn);
    printk("|BLE PERIPHERAL| Connected.\n");
  }
}

static void ble_peripheral_disconnected(struct bt_conn *conn, uint8_t reason) {
  int err = 0;
  printk("|BLE PERIPHERAL| Disconnected, reason %u.\n", reason);

  /* Decrementa conexão anterior do contador. */
  if (self.default_conn) {
    bt_conn_unref(self.default_conn);
    self.default_conn = NULL;
  }

  /* Volta a realizar o adversiting. */
  err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
  if (err) {
    printk("|BLE PERIPHERAL| Advertising failed to start (err %d).\n", err);
  }
}

static void ble_peripheral_ready(int init_err) {
  int err = 0;
  if (init_err) {
    printk("|BLE PERIPHERAL| Bluetooth init failed (err %d).\n", err);
    return;
  }

  /* Inicializa Aversiting. */
  err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
  if (err) {
    printk("|BLE PERIPHERAL| Advertising failed to start (err %d).\n", err);
    return;
  }

  printk("|BLE PERIPHERAL| Started advertising.\n");
}

int ble_peripheral_init() {
  int err = 0;

  /* Configura os callbacks necessários para o BLE. */
  bt_conn_cb_register(&self.conn_callbacks);
  bt_gatt_cb_register(&self.gatt_callbacks);

  /* Incializa Bluetooth. */
  err = bt_enable(ble_peripheral_ready);
  if (err) {
    printk("|BLE PERIPHERAL| Bluetooth init failed (err %d).\n", err);
    return err;
  }

  printk("|BLE PERIPHERAL| Bluetooth initialized.\n");

  return 0;
}