/*  OpenPipe Breakout BLE MIDI
 *
 *  Send BLE MIDI commands to external synths.
 *  
 *  Connect the OpenPipe Breakout wires to a LightBlue Bean or Bean+
 *  
 *  0. Bean+
 *  
 *  RED -> A2 (VCC)
 *  BLACK -> A3 (GND)
 *  WHITE-> A4 (SDA)
 *  GREEN-> A5 (SCL)
 *
 *  If connecting via I2C connector (rhs), A2 and A3 are unused.
 *  However, NOTE that the first pin of the I2C is GND and the 
 *  second pin is VCC, whereas OpenPipe connector has red (VCC) 
 *  first and black (GND) second. Switch them in the connector.
 *  
 *  1. Bean
 *  
 *  WHITE-> A0 (SDA)
 *  GREEN-> A1 (SCL)
 *  RED -> D0 (VCC)
 *  BLACK -> D1 (GND)
 *  
 *  Switch white/green wires with red/black wires in connector.
 *  
 *  © OpenPipe Labs. 2017
 *  www.openpipe.cc
 */

#include <Wire.h> // Wire library for communicating with OpenPipe
#include <OpenPipe.h> // OpenPipe Library

// Select fingering here
#define FINGERING FINGERING_GAITA_GALEGA
//#define FINGERING FINGERING_GAITA_ASTURIANA
//#define FINGERING FINGERING_GREAT_HIGHLAND_BAGPIPE
//#define FINGERING FINGERING_UILLEANN_PIPE
//#define FINGERING FINGERING_SACKPIPA

#define CHANNEL CHANNEL0

// Global variables
unsigned long previous_fingers;
unsigned char previous_note;
boolean playing;
boolean connected;

void setup() {

  // OpenPipe setup
#if defined(IS_BEAN) && IS_BEAN == 1 // Bean
  OpenPipe.power(0, 1); // VCC PIN in D0 and GND PIN in D1
#else
  OpenPipe.power(A2, A3); // VCC PIN in A2 and GND PIN in A3
#endif
  OpenPipe.config();
  OpenPipe.setFingering(FINGERING);

  // Initialize variables
  previous_fingers = 0xFF;
  previous_note = 0;
  playing = false;
  connected = false;

  // LED is blue until we're connected
  Bean.setLedBlue(255);

  BeanMidi.enable();
}

void loop() {

  // Connection
  if (Bean.getConnectionState() != connected) {
    connected = !connected;
    if (connected) {
      Bean.setLedBlue(0);
    } else {
      Bean.setLedBlue(255);
    }
  }
  if (!connected) {
    return;
  }

  // Read OpenPipe fingers
  unsigned long fingers = OpenPipe.readFingers();

  // If fingers have changed...
  if (fingers != previous_fingers) {
    previous_fingers = fingers;

    // Check the low right thumb sensor
    if (OpenPipe.isON()) {
      playing = true;

      // If note changed...
      if (OpenPipe.note != previous_note && OpenPipe.note != 0xFF) {
        // Stop previous and start current
        BeanMidi.loadMessage(NOTEOFF + CHANNEL, previous_note, 127);
        BeanMidi.loadMessage(NOTEON + CHANNEL, OpenPipe.note, 127);
        BeanMidi.sendMessages();
        previous_note = OpenPipe.note;
      }
    } else {
      if (playing) {
        BeanMidi.noteOff(CHANNEL, previous_note, 0); // Stop the note
        playing = false;
      }
    }
  }
}

