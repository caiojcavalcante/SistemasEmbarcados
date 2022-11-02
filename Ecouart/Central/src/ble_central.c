/**
 * @file ble_central.c
 * @author Jeferson Fernando (jfss@ic.ufal.br)
 * @brief Interface de implementação de stack bluetooth com funcionalidade BLE
 * UART Central.
 * @version 0.1
 * @date 2022-11-02
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "ble_central.h"

/**
 * @brief Callback que trata a stack bluetooth atualizando tamanho da MTU.
 *
 * @param conn [in] Ponteiro para estrutura de handle de conexão.
 * @param tx  Tamanho da MTU TX.
 * @param rx  Tamanho da MTU RX.
 */
void ble_central_mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx);

/**
 * @brief Callback que trata a verificação do Peripheral.
 *
 * @param data [in] Ponteiro para estrutura que contém os dados dos serviços do
 * periférico.
 * @param user_data [in] Dado passado no cadastro do callback, contém o endereço
 * do Peripheral.
 * @return true Em caso de sucesso.
 * @return false EM caso de erro.
 */
static bool ble_central_eir_found(struct bt_data *data, void *user_data);

/**
 * @brief Callback que trata a identificação de um dispositivo em adversiting.
 *
 * @param addr Ponteiro para estrutura que identifica o dispostivo.
 * @param rssi RSSI do dispositivo identificado.
 * @param type Tipo de conexão disponível para ser estabelecida com o
 * dispositivo.
 * @param ad Ponteiro para estrutura de representação de conexão.
 */
static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *ad);

/**
 * @brief Callback que trata notify.
 *
 * @param conn [in] Ponteiro para estrutura de handle de conexão.
 * @param params [in] Ponteiro para estrutura dos parâmetros de inscrição.
 * @param buf [in] Ponteiro para buffer dos dados do notify.
 * @param length Tamanho do buffer dos dados do notify.
 * @return uint8_t BT_GATT_ITER_CONTINUE para prosseguir com iteração e
 * BT_GATT_ITER_STOP, caso contrário.
 */
static uint8_t ble_central_notify(struct bt_conn *conn,
                                  struct bt_gatt_subscribe_params *params,
                                  const void *buf, uint16_t length);

/**
 * @brief Callback que trata a identificação das características do dispositivo
 * bluetooth pareado.
 *
 * @param conn [in] Ponteiro para estrutura de handle de conexão.
 * @param attr [in] Ponteiro para estrutura do atributo atualizado.
 * @param params [in] Ponteiro para estrutura dos parâmetros de identificação
 * das características.
 * @return uint8_t BT_GATT_ITER_CONTINUE para prosseguir com iteração e
 * BT_GATT_ITER_STOP, caso contrário.
 */
static uint8_t
ble_central_discover_func(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                          struct bt_gatt_discover_params *params);

/**
 * @brief Callback que trata a stack bluetooth após uma conexão.
 *
 * @param conn [in] Ponteiro para estrutura de handle de conexão.
 * @param err Indica se houve erro durante pareamento.
 */
static void ble_central_connected(struct bt_conn *conn, uint8_t err);

/**
 * @brief Callback que trata a stack bluetooth após uma desconexão.
 *
 * @param conn [in] Ponteiro para estrutura de handle de conexão.
 * @param reason Indica causa da desconexão.
 */
static void ble_central_disconnected(struct bt_conn *conn, uint8_t reason);

/**
 * @brief Inicializa escaneamento de dispositivo em advertising.
 *
 */
static void ble_central_start_scan(void);

/**
 * @brief Callback que trata a stack bluetooth após o mesmo estar pronto,
 * procurando por periféricos.
 *
 * @param err Indica se houver erro na inicialização.
 */
static void ble_central_search_for_peripherals(int err);

/**
 * @brief Estrutura interna de variáveis.
 *
 */
static struct {
  struct bt_conn_cb conn_callbacks; /* Estrutura de callbacks de conexão. */
  struct bt_gatt_cb gatt_callbacks; /* Estrutura de callbacks de GATT. */
  struct bt_gatt_discover_params
      discover_params; /* Estrutra de parâmetros para descobertas de atributos
                          GATT. */
  struct bt_gatt_subscribe_params
      subscribe_params;         /* Estrutura de parâmetros para subcribe.  */
  struct bt_conn *default_conn; /* Ponteiro para handle de conexões ativas. */
  uint16_t write_handle;        /* Handle da característica de write. */
  struct bt_uuid_16 uuid;       /* Estrutura que define UUIDs. */
} self = {
    .conn_callbacks =
        {
            .connected = ble_central_connected,
            .disconnected = ble_central_disconnected,
        },
    .gatt_callbacks =
        {
            .att_mtu_updated = ble_central_mtu_updated,
        },
    .discover_params = {0},
    .subscribe_params = {0},
    .default_conn = NULL,
    .write_handle = 0,
    .uuid = BT_UUID_INIT_16(0),
};

void ble_central_mtu_updated(struct bt_conn *conn, uint16_t tx, uint16_t rx) {
  printk("|BLE CENTRAL| Updated MTU. TX:%d RX:%d bytes.\n", tx, rx);
}

static bool ble_central_eir_found(struct bt_data *data, void *user_data) {
  bt_addr_le_t *addr = user_data;
  int i;

  printk("|BLE CENTRAL| [AD]: %u data_len %u.\n", data->type, data->data_len);

  switch (data->type) {
  case BT_DATA_UUID16_SOME:
  case BT_DATA_UUID16_ALL:
    if (data->data_len % sizeof(uint16_t) != 0U) {
      printk("|BLE CENTRAL| AD malformed.\n");
      return true;
    }

    for (i = 0; i < data->data_len; i += sizeof(uint16_t)) {
      struct bt_le_conn_param *param;
      struct bt_uuid *uuid;
      uint16_t u16;
      int err;

      memcpy(&u16, &data->data[i], sizeof(u16));
      uuid = BT_UUID_DECLARE_16(sys_le16_to_cpu(u16));
      if (bt_uuid_cmp(uuid, BLE_UART_SVC_UUID)) {
        continue;
      }

      err = bt_le_scan_stop();
      if (err) {
        printk("|BLE CENTRAL| Stop LE scan failed (err %d).\n", err);
        continue;
      }

      param = BT_LE_CONN_PARAM_DEFAULT;
      err = bt_conn_le_create(addr, BT_CONN_LE_CREATE_CONN, param,
                              &self.default_conn);
      if (err) {
        printk("|BLE CENTRAL| Create conn failed (err %d).\n", err);
        ble_central_start_scan();
      }

      return false;
    }
  }

  return true;
}

static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type,
                         struct net_buf_simple *ad) {
  char dev[BT_ADDR_LE_STR_LEN];

  bt_addr_le_to_str(addr, dev, sizeof(dev));
  printk("|BLE CENTRAL| Device:%s, AD evt type %u, AD data len %u, RSSI %i.\n",
         dev, type, ad->len, rssi);

  /* Caso o dispostivo possa ser lido e conectado, inicia verificação de seus
   * serviços. */
  if (type == BT_GAP_ADV_TYPE_ADV_IND ||
      type == BT_GAP_ADV_TYPE_ADV_DIRECT_IND) {
    bt_data_parse(ad, ble_central_eir_found, (void *)addr);
  }
}

static uint8_t ble_central_notify(struct bt_conn *conn,
                                  struct bt_gatt_subscribe_params *params,
                                  const void *buf, uint16_t length) {

  if (!buf) {
    printk("|BLE CENTRAL| Unsubscribed.\n");
    params->value_handle = 0U;
    return BT_GATT_ITER_CONTINUE;
  }

  char data[length + 1];

  memcpy(data, buf, length);
  data[length] = '\0';

  printk("|BLE CENTRAL| Notification Received data: %s. Length %u.\n", data,
         length);

  return BT_GATT_ITER_CONTINUE;
}

static uint8_t
ble_central_discover_func(struct bt_conn *conn, const struct bt_gatt_attr *attr,
                          struct bt_gatt_discover_params *params) {
  int err;

  if (!attr) {
    printk("|BLE CENTRAL| Discover complete.\n");
    (void)memset(params, 0, sizeof(*params));
    return BT_GATT_ITER_STOP;
  }

  printk("|BLE CENTRAL| Discover attribute handle: %u.\n", attr->handle);

  /* Identifica o serviço BLE UART. */
  if (!bt_uuid_cmp(self.discover_params.uuid, BLE_UART_SVC_UUID)) {
    memcpy(&self.uuid, BLE_UART_NOTIFY_CHAR_UUID, sizeof(self.uuid));
    self.discover_params.uuid = &self.uuid.uuid;
    self.discover_params.start_handle = attr->handle + 1;
    self.discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;

    /* Continua descoberta para a característica de notify. */
    err = bt_gatt_discover(conn, &self.discover_params);
    if (err) {
      printk("|BLE CENTRAL| Discover failed (err %d).\n", err);
    }
  } else if (!bt_uuid_cmp(self.discover_params.uuid,
                          BLE_UART_NOTIFY_CHAR_UUID)) {
    memcpy(&self.uuid, BLE_UART_WRITE_CHAR_UUID, sizeof(self.uuid));
    self.discover_params.uuid = &self.uuid.uuid;
    self.discover_params.start_handle = attr->handle + 1;
    self.discover_params.type = BT_GATT_DISCOVER_CHARACTERISTIC;
    self.subscribe_params.value_handle = bt_gatt_attr_value_handle(attr);

    /* Continua descoberta para a caracterísitca de escrita. */
    err = bt_gatt_discover(conn, &self.discover_params);
    if (err) {
      printk("|BLE CENTRAL| Discover failed (err %d).\n", err);
    }
  } else if (!bt_uuid_cmp(self.discover_params.uuid,
                          BLE_UART_WRITE_CHAR_UUID)) {
    memcpy(&self.uuid, BT_UUID_GATT_CCC, sizeof(self.uuid));
    self.discover_params.uuid = &self.uuid.uuid;
    self.discover_params.start_handle = attr->handle + 1;
    self.discover_params.type = BT_GATT_DISCOVER_DESCRIPTOR;
    self.write_handle = bt_gatt_attr_value_handle(attr);

    /* Continua descoberta para descritor do serviço. */
    err = bt_gatt_discover(conn, &self.discover_params);
    if (err) {
      printk("|BLE CENTRAL| Discover failed (err %d).\n", err);
    }
  } else {
    self.subscribe_params.notify = ble_central_notify;
    self.subscribe_params.value = BT_GATT_CCC_NOTIFY;
    self.subscribe_params.ccc_handle = attr->handle;

    err = bt_gatt_subscribe(conn, &self.subscribe_params);
    if (err && err != -EALREADY) {
      printk("|BLE CENTRAL| Subscribe failed (err %d).\n", err);
    } else {
      printk("|BLE CENTRAL| Subscribed!\n");
    }

    return BT_GATT_ITER_STOP;
  }

  return BT_GATT_ITER_STOP;
}

static void ble_central_connected(struct bt_conn *conn, uint8_t conn_err) {
  char addr[BT_ADDR_LE_STR_LEN];
  int err;

  bt_addr_le_to_str(bt_conn_get_dst(conn), addr, sizeof(addr));

  /* Caso ocorra erro, volta a escanear dispositivos. */
  if (conn_err) {
    printk("|BLE CENTRAL| Failed to connect to %s (%u).\n", addr, conn_err);

    bt_conn_unref(self.default_conn);
    self.default_conn = NULL;

    ble_central_start_scan();
    return;
  }

  printk("|BLE CENTRAL| Connected: %s.\n", addr);

  /* Inicializa identificação das características do dispositivos pareado. */
  if (conn == self.default_conn) {
    memcpy(&self.uuid, BLE_UART_SVC_UUID, sizeof(self.uuid));
    self.discover_params.uuid = &self.uuid.uuid;
    self.discover_params.func = ble_central_discover_func;
    self.discover_params.start_handle = 0x0001;
    self.discover_params.end_handle = 0xffff;
    self.discover_params.type = BT_GATT_DISCOVER_PRIMARY;

    err = bt_gatt_discover(self.default_conn, &self.discover_params);
    if (err) {
      printk("|BLE CENTRAL| Discover failed(err %d).\n", err);
      return;
    }
  }
}

static void ble_central_disconnected(struct bt_conn *conn, uint8_t reason) {
  printk("|BLE CENTRAL| Disconnected, (reason %u).\n", reason);

  /* Decrementa conexão anterior do contador. */
  if (self.default_conn) {
    bt_conn_unref(self.default_conn);
    self.default_conn = NULL;
  }

  /* Volta a realizar o escaneamento. */
  ble_central_start_scan();
}

static void ble_central_start_scan(void) {
  int err = 0;

  struct bt_le_scan_param scan_param = {
      .type = BT_LE_SCAN_TYPE_ACTIVE,
      .options = BT_LE_SCAN_OPT_NONE,
      .interval = BT_GAP_SCAN_FAST_INTERVAL,
      .window = BT_GAP_SCAN_FAST_WINDOW,
  };

  err = bt_le_scan_start(&scan_param, device_found);
  if (err) {
    printk("|BLE CENTRAL| Scanning failed to start (err %d).\n", err);
    return;
  }

  printk("|BLE CENTRAL| Scanning successfully started.\n");
}

static void ble_central_search_for_peripherals(int err) {
  ble_central_start_scan();
}

int ble_central_write_input(uint8_t *buf, uint16_t buf_len) {
  int err = 0;

  if (self.default_conn == NULL) {
    printk("Not connected!");
    return -1;
  }

  err = bt_gatt_write_without_response(self.default_conn, self.write_handle,
                                       buf, buf_len, false);
  if (err) {
    printk("%s: Write cmd failed (%d).\n", __func__, err);
  }

  return 0;
}

int ble_central_init() {
  int err = 0;

  /* Configura os callbacks necessários para o BLE. */
  bt_conn_cb_register(&self.conn_callbacks);
  bt_gatt_cb_register(&self.gatt_callbacks);

  /* Incializa Bluetooth. */
  err = bt_enable(ble_central_search_for_peripherals);
  if (err) {
    printk("|BLE CENTRAL| Bluetooth init failed (err %d).\n", err);
    return err;
  }

  printk("|BLE CENTRAL| Bluetooth initialized.\n");

  return 0;
}