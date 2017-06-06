#include <stdio.h>
#include "net_slave.h"


unsigned char _sn_generate_crc_buffer(unsigned char * buffer, int len);

int testCRC() {
  unsigned char buffer[] ={0xab,0x30,0x54,0xcd};
  unsigned char crc =  _sn_generate_crc_buffer( buffer, 4);
  if(crc != 0x47) {
    printf("CRC calculation is %x when should be 0x47\n",crc);
    return -1;
  }
  return 0;
}

/*
 *   Test that packet for different device is ignored
 */
int testPacketForOtherDevice() {
  unsigned char wire_data[] = {0xff,0x01,0x03,0x01,0x02,0x03,0x00};
  unsigned char buffer[256];
  sn_init();
  sn_connect(0x02);
  for(int i =0;i<7;i++) {
    sn_process_bus_data(wire_data[i] );
  }
  if (sn_current_state() != SN_WAIT_STATE) {
    printf("Packet for other device not in SN_WAIT_STATE %x\n",sn_current_state());
    return -1;
  }
  return 0;
}


/*
 *   Test that packet for this device is accepted and
 *  zero packet is returned
 */
int testPacketForDevice() {
  unsigned char wire_data[] = {0xff,0x02,0x03,0x01,0x02,0x03,0x00};
  unsigned char buffer[256];
  sn_init();
  sn_connect(0x02);
  for(int i =0;i<7;i++) {
    sn_process_bus_data(wire_data[i] );
  }
  if (sn_current_state() != SN_DATA_AVAILABLE) {
    printf("Packet for device not in SN_DATA_AVAILABLE %x\n",sn_current_state());
    return -1;
  }
  int l = sn_read(buffer,256);
  if(l!= 3) {
    printf("Data len is not 3 %x\n",l);
    return -1;
  }
  if(buffer[0] != 0x01 ||
     buffer[1] != 0x02 ||
     buffer[2] != 0x03 ) {
    printf("Data wrong\n");
    return -1;
  }
  return 0;
}

void rec_byte(unsigned char c) {

}


/*
 *   Test that packet for this device is accepted and data is
 *  returned correctly
 */
int testWritePacketFromDevice() {
  unsigned char wire_data[] = {0xff,0x02,0x00,0x00};
  unsigned char write_data[] = {'T','E','S','T'};
  unsigned char buffer[256];
  sn_init();
  sn_set_send_byte_fp(*rec_byte);
  sn_connect(0x02);
  for(int i =0;i<4;i++) {
    sn_process_bus_data(wire_data[i] );
  }
  if (sn_current_state() != SN_DATA_AVAILABLE) {
    printf("Packet for device not in SN_DATA_AVAILABLE %x\n",sn_current_state());
    return -1;
  }
  int l = sn_read(buffer,256);
  if(l!= 0) {
    printf("Data len is not 0 %x\n",l);
    return -1;
  }
   l = sn_write(write_data, 4);
  for(int i =0;i<8;i++) {
    sn_process_bus_data(0x0 );
  }

  return 0;
}


/*
 *  Test pack with start byte contained is handled correctly
 */

int testMixedPacket() {
  return 0;
}


int main(int argc, char const *argv[]) {
  return testCRC() | testPacketForOtherDevice() | testPacketForDevice() |testWritePacketFromDevice();
}
