#ifndef NET_SLAVE_H
#define NET_SLAVE_H

#define SN_BUFFER_SIZE 260
#define SN_MAX_DATA_SIZE 256

#define SN_START_BYTE 0xFF

typedef void (*SN_SEND_FUNC_POINTER)(unsigned char);
typedef void (*SN_YIELD_FUNC_POINTER)(void);

typedef enum { SN_WAIT_STATE=0x00,
                   SN_READ_DEVICE_ID=0x01,
                   SN_READ_LENGTH_STATE=0x02,
                   SN_READ_DATA_STATE=0x03,
                   SN_READ_CRC_STATE=0x04,
                   SN_DATA_AVAILABLE=0x05,
                   SN_BUFFER_WRITE_STATE=0x06,
                   SN_SEND_START=0x07,
                   SN_SEND_LENGTH=0x08,
                   SN_SEND_DATA=0x09,
                   SN_SEND_CRC=0x0a
                  } sn_states_t;


void sn_set_send_byte_fp(SN_SEND_FUNC_POINTER fp);

void sn_set_yield_fp(SN_YIELD_FUNC_POINTER fp);

void sn_init();

int sn_connect(unsigned char device_id);

int sn_read(unsigned char* buffer, int length);

int sn_write(unsigned char* buffer, int length);

sn_states_t sn_process_bus_data(unsigned char data );

sn_states_t sn_current_state();

int sn_flush();

#endif
