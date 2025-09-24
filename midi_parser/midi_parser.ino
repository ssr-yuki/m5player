#include <M5AtomS3.h>
#include <MIDI.h>
#include "solenoid.h"

#define LED_G (7)
#define LED_B (8)

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);
Solenoid solenoid;

const int minOnCount = 50;  // minimum duration to push keys [ms]
const int minWaitTime = 50;
const int numSolenoid = 40;

bool useYamaha = 0;

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

void setup() {
  // initialize counters
  for (int i = 0; i < numSolenoid; ++i) {
    noteOnTimer[i] = 0;
    noteWaitTimer[i] = 0;
  }

  auto cfg = M5.config();
  AtomS3.begin(cfg);
  delay(100);

  // display MIDI style
  M5.update();
  M5.Display.setTextSize(3);
  M5.Display.startWrite();

  useYamaha = M5.BtnA.isPressed();

  if (useYamaha) {
    M5.Display.println("YAMAHA");
  } else {
    M5.Display.println("Int.");
  }

  solenoid.ResetSolenoid();
  pinMode(LED_G, OUTPUT);
  pinMode(LED_B, OUTPUT);

  Serial.begin(115200);
  Serial2.begin(31250, SERIAL_8N1, 1, 2);

  delay(50);

  // for (int note = 30; note < 100; ++note) {
  //   const int id = noteNo2id(note);
  //   solenoid.SetSolenoid(id, 1);
  //   delay(250);
  //   solenoid.SetSolenoid(id, 0);
  //   delay(250);
  // }

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
  int ret = -1;

  if (useYamaha) {
    noteNo -= 12;
  }

  if (noteNo >= 36 && noteNo < 53) {
    ret = noteNo - 36;
  } else if (noteNo >= 60 && noteNo <= 72) {
    ret = noteNo - 60 + 17 + 4;
  } else if (noteNo >= 78 && noteNo <= 80) {
    // note 90 ~ 92 -> id 36 ~ 38
    ret = noteNo - 42;
  } else {
    switch (noteNo) {
      case 75:
      case 76:
        ret = noteNo - 41;  // 87, 88 -> 34, 35
        break;
      case 82:
        ret = 39;
        break;
    }
  }

  return ret;
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
