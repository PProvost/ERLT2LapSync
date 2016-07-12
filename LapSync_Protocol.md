# TrackMate Serial Protocol for MultiGP LapSync

## Format specification notes

In the following specification, the following guidelines apply:
- Non printable ASCII characters will be shown in hex format, followed by the
  ASCII symbol name in parentheses, e.g. 0x01 (SOH).
- Single ASCII characters will be shown in hex format, followed by the ASCII symbol
  in single quotes (C-style), e.g. 0x23 ('#')
- ASCII strings will be shown in double quotes and should be transmitted as a series
  of bytes corresponding to the ASCII chars

## Initialization / Reset

LapSync will send the following bytes to Trackmate to tell it to start
sending timing messages or to reset the hit counter (which is ignored by
LapSync).

    0x01 0x3F 0x2C 0x32 0x30 0x32 0x2C 0x30 0x2C 0x31 0x31 0x2C 0x0D 0x0A

Note: It is not required that the timing system respond to this message, and it
is acceptable for it to always be sending messages if that is easier to
implement.

## Trackmate message format

Trackmate messages are a stream of ASCII containing 0x01 (SOH) and a message
type character, then a set of tab (0x09) delimited data fields depending on the
message type, and terminated by 0x0D 0x0A (ASCII CR-LF). 

There are two kinds of messages:
- Heartbeat messages are sent once per minute 
- Lap record messages are sent as they occur. 

Note: No de-duping (e.g. two triggers fired by one passing through the gate) is
required. That is handled by LapSync.

## Heartbeat messages

Heartbeat messages are composed of the following tab delimited fields:

1. Message type - byte - always 0x23 ('#') for heartbeat messages
2. Unknown - string - always "202"
3. Message number - string - the message number represented as a string, e.g.
   "1665". Each new message increments the sequence number, e.g. "1666" for the
   message after "1665". 
4. Unknown - byte - always 0x30 ('0')
5. Unknown - string - always "xC249"

Example complete heartbeat message:

    0x01                      SOH
    0x23                      '#'
    0x09                      TAB
    0x32 0x30 0x32            "202"
    0x09                      TAB 
    0x31 0x36 0x36 0x35       "1665"
    0x09                      TAB
    0x30                      '0' 
    0x09                      TAB
    0x78 0x43 0x32 0x34 0x39  "xC249"
    0x0D 0x0A                 CR+LF 

## Lap record messages

1. Message type - byte - always 0x40 ('@') for heartbeat messages
2. Unknown - string - always "202"
3. Message number - string - the message number represented as a string, e.g.
   "1665". Each new message increments the sequence number, e.g. "1666" for the
   message after "1665". 
4. Transponder ID - string - ASCII string containing the transponder ID, e.g. "1001259" or "12"
5. Timestamp - string - timestamp when the lap was recorded in seconds with two decimal places
6. Hits - string - number of times since reset that this transponder was seen, e.g. "16"
7. Unknown - srting - always "111"
8. Unknown - byte - always 0x30 ('0')
9. Unknown - string - always "x5724"

    0x01                                  SOH
    0x40                                  '@'
    0x09                                  TAB
    0x32 0x30 0x32                        "202"
    0x09                                  TAB
    0x31 0x36 0x37 0x32                   "1672"
    0x09                                  TAB
    0x31 0x30 0x30 0x31 0x32 0x35 0x39    "1001259"
    0x09                                  TAB
    0x33 0x32 0x38 0x2E 0x37 0x39         "328.79"
    0x09                                  TAB
    0x31 0x36                             "16"
    0x09                                  TAB
    0x31 0x31 0x31                        "111"
    0x09                                  TAB
    0x30                                  '0'
    0x09                                  TAB
    0x78 0x35 0x37 0x32 0x34              "x5724"
    0x0D 0x0A                             CR+LF 

