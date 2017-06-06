#include "net_slave.h"
#include <stdio.h>
#include <stdint.h>

char _sn_buffer[SN_BUFFER_SIZE];
char _sn_write_buffer[SN_BUFFER_SIZE];

int _sn_buffer_position;

int _sn_write_buffer_position;
int _sn_write_buffer_sn_length;

unsigned char _sn_device_id;

sn_states_t _sn_state;
unsigned char _sn_length;

SN_SEND_FUNC_POINTER _sn_send_fp;
SN_YIELD_FUNC_POINTER _sn_yield_fp;

// CRC poly 0x5d
uint32_t _crc_div = (0x15d00 >> 1);


// Check CRC
unsigned char _sn_generate_crc_buffer(unsigned char * buffer, int len) {
  unsigned char crc = 0x00;
  uint16_t top = 0x0000;

  for(int i=0; i<len; i++) {
    top = top << 8;
    top = top | buffer[i];
    for(int b =0; b<8;b++) {
      if((0x8000 >> b) & top)
        top = top ^ (_crc_div >> b );
    }
    printf(" %x \n",top);
  }

  top = top << 8;
  for(int b =0; b<8;b++) {
    if((0x8000 >> b) & top)
      top = top ^ (_crc_div >> b);
    if((top & 0xff00) == 0)
      return (unsigned char)top ;
  }
  printf(" %x \n",top);
  return (unsigned char)top;
}

void sn_init() {
  _sn_send_fp=0;
  _sn_yield_fp=0;
}
int sn_connect(unsigned char device_id) {
  // device_id 32 is th maximum - maybe we should use the top 3 bits......
  if(device_id > 32) {
    return -1;
  }
  _sn_device_id = device_id;
  _sn_state = SN_WAIT_STATE;
  return 0;
}

int sn_read(unsigned char* buffer, int length) {
  if(_sn_state==SN_DATA_AVAILABLE) {
    int bytes_read=0;
    while (bytes_read < length && _sn_buffer_position<_sn_length) {
      buffer[bytes_read++] = _sn_buffer[_sn_buffer_position++];
    }
    if(_sn_buffer_position==_sn_length) {
      _sn_state = SN_BUFFER_WRITE_STATE;
      sn_flush();
    }
    _sn_buffer_position=0;
    return bytes_read;
  } else {
    return -1;
  }
}

int sn_write(unsigned char* buffer, int length) {
  // TODO block here and loop
  if(length > SN_BUFFER_SIZE ) {
    return -1;
  }
  int i;
  // If no bufferspace flush
  if(length > SN_BUFFER_SIZE-_sn_write_buffer_position) {
     sn_flush();
  }

  if(length < SN_BUFFER_SIZE-_sn_write_buffer_position) {
    for(i = 0;i<length;i++) {
      _sn_write_buffer[_sn_write_buffer_position++] = buffer[i];
    }
  }
  return i;
}

int sn_flush() {
  while(_sn_state!=SN_BUFFER_WRITE_STATE) {
    if(_sn_yield_fp)
      (_sn_yield_fp)();
    // delay(100);
    // TODO Calc timeout and timeout here.....
  }
  int pos = 0;
  _sn_write_buffer_sn_length = _sn_write_buffer_position;
  _sn_write_buffer_position=0;
  _sn_buffer_position=0;
  _sn_length =pos;
  _sn_state = SN_SEND_START;
  // Process the bus
  for(int i =0;i<_sn_write_buffer_sn_length+3;i++) {
    sn_process_bus_data(0x0 );
  }
  _sn_write_buffer_position = 0;

  return _sn_length;
}

void sn_set_send_byte_fp(SN_SEND_FUNC_POINTER fp) {
  _sn_send_fp=fp;
}

void sn_set_yield_fp(SN_YIELD_FUNC_POINTER fp) {
  _sn_yield_fp=fp;
}

sn_states_t sn_current_sn_state() {
  return _sn_state;
}



// Check CRC
char _sn_generate_crc() {
  return 0;
}

// reset the buffer
void   _sn_move_buffer() {
  // This is effectivly going to scroll the buffer
  // This is a quick and dirty way to sort this, a circular
  // buffer would be better
  // TODO Use a circular buffer to avoid moving memory arround....


}

void _sn_send_byte(unsigned char data) {
    if(_sn_send_fp)
      (_sn_send_fp)(data);
}

sn_states_t sn_current_state() {
  return _sn_state;
}
sn_states_t sn_process_bus_data(unsigned char data ) {
  switch(_sn_state)
  {
    case SN_WAIT_STATE:
      if(data==SN_START_BYTE) {
        _sn_buffer_position = 0;
        _sn_state = SN_READ_DEVICE_ID;
      }
      break;
    case SN_READ_DEVICE_ID:
      if(data==_sn_device_id) {
        _sn_state = SN_READ_LENGTH_STATE;
      } else {
        _sn_state = SN_WAIT_STATE;
      }
      break;
    case SN_READ_LENGTH_STATE:
      _sn_length = data;
      if(_sn_length==0)
        _sn_state = SN_READ_CRC_STATE;
      else
        _sn_state = SN_READ_DATA_STATE;
      break;
    case SN_READ_DATA_STATE:
      _sn_buffer[_sn_buffer_position++]=data;
      if(_sn_buffer_position==_sn_length) {
        _sn_state = SN_READ_CRC_STATE;
      }
      break;
    case SN_READ_CRC_STATE:
      if(_sn_generate_crc() ==data) {
        _sn_state = SN_DATA_AVAILABLE;
        _sn_buffer_position = 0;
      } else {
        _sn_move_buffer();
      }
      break;
    case SN_SEND_START:
      _sn_send_byte(SN_START_BYTE);
      _sn_state = SN_SEND_LENGTH;
      break;
    case SN_SEND_LENGTH:
      _sn_send_byte(_sn_length);
      _sn_state = SN_SEND_DATA;
      break;
    case SN_SEND_DATA:
      _sn_send_byte(_sn_write_buffer[_sn_write_buffer_position++]);
      if(_sn_write_buffer_position==_sn_write_buffer_sn_length) {
        _sn_state = SN_SEND_CRC;
      }
      break;
      case SN_SEND_CRC:
        _sn_send_byte(_sn_generate_crc());
        _sn_state = SN_WAIT_STATE;
        break;
    default:
      break;
  }
  return _sn_state;
}
