## NFC Relay Control 

Just a small project to swith a relay to its second state while a certain card is held over the reader. The Arduino will reflect the change to the second state on pin 4 (0V for state 0, 5V for state 1).

The code checks for the card UID. The UID has to be defined in a header file named `tag-data.h`

```
#define TAG_DATA

const String KEY_UID = "00:00:00:00:00:00:00";

```