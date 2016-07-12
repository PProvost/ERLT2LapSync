//------------------------------------------------------------------------------
// Include the IRremote library header
//
#include "IRremote.h"

//------------------------------------------------------------------------------
// Tell IRremote which Arduino pin is connected to the IR Receiver (TSOP4838)
//
int recvPin = 11;
IRrecv irrecv(recvPin);

#define ERLT_ZERO 300
#define ERLT_ONE  600
#define ERLT_TOLERANCE  150

#define MIN_TIMER_DELTA 5000       // milliseconds between valid lap recordings

#define CHAR_SOH    0x01
#define CHAR_TAB    0x09
#define CHAR_CR     0x0D
#define CHAR_LF     0x0A

#define MSG_TYPE_HEARTBEAT  0x23
#define MSG_TYPE_LAP        0x40

unsigned long timers[64];
unsigned long lastMillis = 0;
unsigned int lastSequence = 0;

//+=============================================================================
// Configure the Arduino
//
void  setup ( )
{
  Serial.begin(9600);   // Status message will be sent to PC at 115200 baud
  irrecv.enableIRIn();  // Start the receiver

  for (int i = 0; i < 64; i++ )
  {
    timers[i] = millis();
  }
}

bool matchERLT(int measured, int desired)
{
  int ticksMeasured = measured * USECPERTICK;
  return ((ticksMeasured >= desired - ERLT_TOLERANCE) && (ticksMeasured <= desired + ERLT_TOLERANCE));
}

bool decodeERLT(decode_results *results)
{
  unsigned long data = 0;
  int offset = 1; // skip the gap reading

  if (results->rawlen - 1 != 9) return false;

  // First two bits must be zero
  if (!matchERLT(results->rawbuf[offset++], ERLT_ZERO)) return false;
  if (!matchERLT(results->rawbuf[offset++], ERLT_ZERO)) return false;

  // Next 6 pulses are the value
  for (int i = 0; i < 6; i++)
  {
    if (matchERLT(results->rawbuf[offset], ERLT_ONE)) data = (data << 1) | 1;
    else if (matchERLT(results->rawbuf[offset], ERLT_ZERO)) data = (data << 1) | 0;
    else return false;
    offset++;
  }

  // Last one is a checksum - 1 for odd count of ONES, 0 for even count
  // TODO

  results->value = data;
  return true;
}

void  dumpERLT(decode_results *results)
{
  Serial.print("ERLT ID: ");
  Serial.println(results->value);

  Serial.print("Timestamp: ");
  Serial.println(millis());
}

void sendTrackmateMessage(int id, unsigned long timestamp)
{
  char buff[8];
  sprintf(buff, "100%04d", id);

  Serial.write(CHAR_SOH);           // Header
  Serial.write(MSG_TYPE_LAP);
  Serial.write(CHAR_TAB);           // Tab
  Serial.print("202");
  Serial.write(CHAR_TAB);
  Serial.print(++lastSequence, DEC);
  Serial.write(CHAR_TAB);
  Serial.print(buff); // Transponder ID
  Serial.write(CHAR_TAB);
  Serial.print(timestamp / 1000.0, 2);  // Timestamp
  Serial.write(CHAR_TAB);
  Serial.write("1");
  Serial.write(CHAR_TAB);
  Serial.write("111");
  Serial.write(CHAR_TAB);
  Serial.write("0");
  Serial.write(CHAR_TAB);
  Serial.print("x5724");
  Serial.write(CHAR_CR);
  Serial.write(CHAR_LF);
}

void sendHeartbeatMessage()
{
  // send ++lastSequence
  Serial.write(CHAR_SOH);
  Serial.write(MSG_TYPE_HEARTBEAT);
  Serial.write(CHAR_TAB);
  Serial.print("202");
  Serial.write(CHAR_TAB);
  Serial.print(++lastSequence, DEC);
  Serial.write(CHAR_TAB);
  Serial.print("0");
  Serial.write(CHAR_TAB);
  Serial.print("xC249");
  Serial.write(CHAR_CR);
  Serial.write(CHAR_LF);
}

//+=============================================================================
// The repeating section of the code
//
void  loop ( )
{
  decode_results  results;        // Somewhere to store the results

  unsigned long currentMillis = millis();
  if (currentMillis > lastMillis + 1000)
  {
    sendHeartbeatMessage();
    lastMillis = currentMillis;
  }

  if (irrecv.decode(&results))
  { // Grab an IR code
    if (decodeERLT(&results))
    {
      int id = results.value;
      if (id < 64)
      {
        if (currentMillis > timers[id] + MIN_TIMER_DELTA)
        {
          sendTrackmateMessage(id, currentMillis);
          timers[id] = currentMillis;
        }
      }
    }

    irrecv.resume();              // Prepare for the next value
  }
}
