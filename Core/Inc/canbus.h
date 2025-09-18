
typedef struct {
    CAN_HandleTypeDef *hcan;
    CAN_TxHeaderTypeDef *tx_header;
    CAN_TxHeaderTypeDef *rx_header;
    uint8_t  rx_packet;
    uint8_t  tx_packet;
    uint32_t tx_mailbox;
} canbus_t;