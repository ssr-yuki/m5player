#include <M5AtomS3.h>
#include <MIDI.h>
#include "solenoid.h"

#define LED_G (7)
#define LED_B (8)
#define POWER_SW (6)

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);
Solenoid solenoid;

const int minOnCount = 50;  // minimum duration to push keys [ms]
const int minWaitTime = 50;
const int numSolenoid = 40;

// incremental timers for notes
// 0: off
// positive: on, off signal is not yet received
// negative: on, will be turned off
int noteOnTimer[numSolenoid];

// incremental timers for notes
// 0: off, avaiable
// positive: off, on signal is not yet received
// negative: on, will be turned on
int noteWaitTimer[numSolenoid];

void PowerOn(){
  digitalWrite(POWER_SW, HIGH);
}
void PowerOff(){
  digitalWrite(POWER_SW, LOW);
}


void setup() {
  // initialize counters
  for (int i = 0; i < numSolenoid; ++i) {
    noteOnTimer[i] = 0;
    noteWaitTimer[i] = 0;
  }

  auto cfg = M5.config();
  AtomS3.begin(cfg);

  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);
  pinMode(POWER_SW, OUTPUT);
  PowerOff();

  solenoid.ResetSolenoid();

  Serial.begin(115200);
  Serial2.begin(31250, SERIAL_8N1, 1, 2);

  delay(1000);
  PowerOn();

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
  for(int i=0;i<60;i++){
  solenoid.SetSolenoid(i, 1);
  delay(50);
  solenoid.SetSolenoid(i, 0);
  delay(50);
  }
}

// return -1 if the value fo `noteNo` is out of the range
int noteNo2id(int noteNo) {
  if (noteNo >= 53 && noteNo <= 84) {
    // note 53 ~ 84 -> id 2 ~ 33
    return noteNo - 51;
  } else if (noteNo >= 90 && noteNo <= 92) {
    // note 90 ~ 92 -> id 36 ~ 38
    return noteNo - 54;
  } else {
    switch (noteNo) {
      case 48:
        return 0;
        break;
      case 51:
        return 1;
        break;
      case 87:
      case 88:
        return noteNo - 53;  // 87, 88 -> 34, 35
        break;
      case 94:
        return 39;
        break;
      default:
        return -1;
        break;
    }
  }
}

void MIDIReceptionTask(void *parameters) {
  while (1) {
    // update timers
    for (int i = 0; i < numSolenoid; ++i) {
      if (noteWaitTimer[i] > 0) {
        --noteWaitTimer[i];
      } else if (noteWaitTimer[i] < 0) {
        ++noteWaitTimer[i];
        if(noteWaitTimer[i] == 0) {
          solenoid.SetSolenoid(i, 1);
          noteOnTimer[i] = 1;
        }
      }

      if (noteOnTimer[i] != 0) {
        ++noteOnTimer[i];
        if (noteOnTimer[i] == 0) {
          solenoid.SetSolenoid(i, 0);
          noteWaitTimer[i] = minWaitTime;
        }
      }
    }

    while (MIDI.read()) {
      const int noteNO = MIDI.getData1();
      const int id = noteNo2id(noteNO);
      if (id < 0) {
        continue;
      }
  
      if (MIDI.getType() == midi::NoteOn) {
        if (noteWaitTimer[id] == 0) {
          solenoid.SetSolenoid(id, 1);
          noteOnTimer[id] = 1;
        } else if (noteWaitTimer[id] > 0) {
          noteWaitTimer[id] -= minWaitTime;
        }
        digitalWrite(LED_G, HIGH);
      } else if (MIDI.getType() == midi::NoteOff) {
        if (noteOnTimer[id] >= minOnCount) {
          solenoid.SetSolenoid(id, 0);
          noteOnTimer[id] = 0;
          noteWaitTimer[id] = minWaitTime;
        } else {
          noteOnTimer[id] -= minOnCount;
        }
        digitalWrite(LED_G, LOW);
      }
    }

    vTaskDelay(1); 
  }
}
