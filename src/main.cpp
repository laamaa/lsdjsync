#include "Arduino.h"

bool sequencerStarted = true; //Sequencer has started?
uint8_t midiClockMessageCount = 0;

const uint8_t pinGBClock = 0;     // Analog In 0 - clock out to gameboy
const uint8_t pinGBSerialOut = 3; // Analog In 1 - serial data to gameboy
const uint8_t pinGBSerialIn = 2;  // Analog In 2 - serial data from gameboy

void SendDataToLSDJ(unsigned int bitClock, unsigned int bitOut, unsigned int bitIn)
{
  digitalWriteFast(pinGBClock, bitClock);
  digitalWriteFast(pinGBSerialOut, bitOut);
  digitalWriteFast(pinGBSerialIn, bitIn);
}

/* Send a 8bit clock pulse to LSDJ
 * from Arduinoboy */
void SendClockTickToLSDJ()
{
  for (uint8_t ticks = 0; ticks < 8; ticks++)
  {
    SendDataToLSDJ(0, 0, 0);
    SendDataToLSDJ(1, 0, 0);
  }
}

inline void SequencerStart()
{
  sequencerStarted = true;
  midiClockMessageCount = 0;
}

inline void SequencerStop()
{
  sequencerStarted = false;
  digitalWriteFast(LED_BUILTIN, HIGH);
}

void OnRealTimeSystem(byte realtimebyte)
{
  switch (realtimebyte)
  {
  case 0xF8: //Clock message
    if (sequencerStarted)
    {
      SendClockTickToLSDJ();
      midiClockMessageCount++;
      if (midiClockMessageCount == 24)
      {
        digitalWriteFast(LED_BUILTIN, HIGH);
        midiClockMessageCount = 0;
      }
      else
      {
        digitalWriteFast(LED_BUILTIN, LOW);
      }
    }

    break;
  case 0xFA:          // Case: Transport Start Message
  case 0xFB:          // and Case: Transport Continue Message
    SequencerStart(); // Start the sequencer
    break;
  case 0xFC:         // Case: Transport Stop Message
    SequencerStop(); // Stop the sequencer
    break;
  }
}

void setup()
{
  /* Setup pins */
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(pinGBClock, OUTPUT);
  pinMode(pinGBSerialIn, INPUT);
  pinMode(pinGBSerialOut, OUTPUT);
  digitalWriteFast(pinGBClock, HIGH);    // gameboy wants a HIGH line
  digitalWriteFast(pinGBSerialOut, LOW); // no data to send
  digitalWriteFast(LED_BUILTIN, HIGH);
  usbMIDI.setHandleRealTimeSystem(OnRealTimeSystem);
}

void loop()
{
  usbMIDI.read();
}