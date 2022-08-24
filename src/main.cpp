#include <Arduino.h>
#include <FlexCAN_T4.h>


/*Odrive configuration:
odrv0.axis0.config.can.node_id = 0
odrv0.axis1.config.can.node_id = 1
odrv0.can.config.baud_rate = 250000
*/
FlexCAN_T4<CAN3, RX_SIZE_256, TX_SIZE_16> can1;
CAN_message_t msg;

void setup() {
  // put your setup code here, to run once:
  can1.begin();
  can1.setBaudRate(250000);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (!Serial){;;}
    if ( can1.read(msg) ) {
    Serial.print("CAN1 "); 
    Serial.print("MB: "); Serial.print(msg.mb);
    Serial.print("  ID: 0x"); Serial.print(msg.id, HEX );
    Serial.print("  EXT: "); Serial.print(msg.flags.extended );
    Serial.print("  LEN: "); Serial.print(msg.len);
    Serial.print(" DATA: ");
    for ( uint8_t i = 0; i < 8; i++ ) {
      Serial.print(msg.buf[i]); Serial.print(" ");
    }
    Serial.print("  TS: "); Serial.println(msg.timestamp);
  }
}