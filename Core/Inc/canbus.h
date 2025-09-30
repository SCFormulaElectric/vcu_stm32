typedef struct {
    CAN_HandleTypeDef *hcan;
    CAN_TxHeaderTypeDef *tx_header;
    CAN_RxHeaderTypeDef *rx_header;
    uint8_t  rx_packet[8];
    uint8_t  tx_packet[8];
    uint32_t tx_mailbox;
	QueueHandle_t can_tx_queue;
	QueueHandle_t can_rx_queue;
} canbus_t;

typedef struct {
    uint32_t tx_id;
    uint8_t  tx_packet[8];
} can_message_t;