#include <M5AtomS3.h>
#include <MIDI.h>
#include "solenoid.h"

#define LED_G (7)
#define LED_B (8)

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);
Solenoid solenoid;

const int minOnCount = 5;  // minimum duration to push keys [ms]
const int numSolenoid = 40;

// incremental timers for notes
// 0: off
// positive: on, off signal is not yet received
// negative: on, will be turned off
int noteOnTimer[numSolenoid];

void setup() {
  // initialize counters
  for (int i = 0; i < numSolenoid; ++i) {
    noteOnTimer[i] = 0;
  }

  auto cfg = M5.config();
  AtomS3.begin(cfg);

  solenoid.ResetSolenoid();
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  Serial.begin(115200);
  Serial2.begin(31250, SERIAL_8N1, 1, 2);

  delay(50);

  // activate all MIDI channels
  MIDI.begin(MIDI_CHANNEL_OMNI);

  // turn off MIDI Thru (output of received signals)
  MIDI.turnThruOff();

  // declare MIDI reception task
  xTaskCreatePinnedToCore(
    MIDIReceptionTask,
    "MIDIReceptionTask",
    8192,
    NULL,
    1,
    NULL,
    1
  );
}

void loop() {
  digitalWrite(LED_B, HIGH);
  delay(500);
  digitalWrite(LED_B, LOW);

  delay(100);
}

// return -1 if the value fo `noteNo` is out of the range
int noteNo2id(int noteNo) {
  if (noteNo >= 53 && noteNo <= 84) {
    // note 53 ~ 84 -> id 3 ~ 34
    return noteNo - 50;
  } else if (noteNo >= 90 && noteNo <= 92) {
    // note 90 ~ 92 -> id 37 ~ 39
    return noteNo - 53;
  } else {
    switch (noteNo) {
      case 48:
        return 1;
        break;
      case 51:
        return 2;
        break;
      case 87:
      case 88:
        return noteNo - 52;  // 87, 88 -> 35, 36
        break;
      case 92:
        return 40;
        break;
      default:
        return -1;
        break;
    }
  }
}

void MIDIReceptionTask(void *parameters) {
  while (1) {
    if (MIDI.read()) {
      // update timers
      for (int i = 0; i < numSolenoid; ++i) {
        if (noteOnTimer[i] != 0) {
          ++noteOnTimer[i];
          if (noteOnTimer[i] == 0) {
            solenoid.SetSolenoid(i, 0);          
          }
        }
      }

      const int noteNO = MIDI.getData1();
      if (MIDI.getType() == midi::NoteOn) {
        const int id = noteNo2id(noteNO);
        solenoid.SetSolenoid(id, 1);
        noteOnTimer[id] = 1;
        digitalWrite(LED_G, HIGH);
      } else if (MIDI.getType() == midi::NoteOff) {
        const int id = noteNo2id(noteNO);
        if (noteOnTimer[id] >= minOnCount) {
          solenoid.SetSolenoid(id, 0);
          noteOnTimer[id] = 0;
        } else {
          noteOnTimer[id] -= minOnCount; 
        }
        digitalWrite(LED_G, LOW);
      }
    }

    vTaskDelay(1); 
  }
}
