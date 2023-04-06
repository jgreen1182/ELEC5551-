#include "common/mavlink.h"
#include "common/mavlink_msg_battery_status.h"

const int batteryPins[] = {A0, A1};
const int numOfBatteries = sizeof(batteryPins) / sizeof(batteryPins[0]);
const float referenceVoltage = 5.0;
const int analogResolution = 1023;

const float r1 = 10000.0;
const float r2 = 10000.0;
const float voltageDividerFactor = (r1 + r2) / r2;

void setup() {
  Serial.begin(57600); // Initialize the serial communication for the flight controller at 57600 baud rate
}

void loop() {
  for (int i = 0; i < numOfBatteries; i++) {
    int analogValue = analogRead(batteryPins[i]);
    float voltage = (analogValue / (float)analogResolution) * referenceVoltage * voltageDividerFactor;

    send_mavlink_battery_status(i, voltage); // Send the battery voltage data using MAVLink
    delay(200);
  }
  delay(2000);
}

void send_mavlink_battery_status(uint8_t battery_id, float voltage) {
  mavlink_message_t msg;
  uint16_t voltages[10] = {0};

  voltages[0] = voltage * 1000; // Convert voltage to millivolts

  mavlink_msg_battery_status_pack(1, 200, &msg, battery_id, MAV_BATTERY_FUNCTION_UNKNOWN, MAV_BATTERY_TYPE_UNKNOWN, INT16_MIN, voltages, -1, -1, -1, -1, 0, NULL, 0, 0, UINT32_MAX);
  uint8_t buf[MAVLINK_MAX_PACKET_LEN];
  uint16_t len = mavlink_msg_to_send_buffer(buf, &msg);
  Serial.write(buf, len);
}
