#include <M5AtomS3.h>
#include <MIDI.h>
#include "solenoid.h"

#define LED_G (7)
#define LED_B (8)

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);
Solenoid solenoid;

void setup() {
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

int noteNo2id(int noteNo){
  return noteNo-30;
}

void MIDIReceptionTask(void *parameters) {
  while (1) {
    if (MIDI.read()) {
      const int noteNO = MIDI.getData1();
      if (MIDI.getType() == midi::NoteOn) {
        Serial.print("Receive NoteON: ");
        Serial.println(noteNO);
        solenoid.SetSolenoid(noteNo2id(noteNO),1);
        digitalWrite(LED_G, HIGH);
      } else if (MIDI.getType() == midi::NoteOff) {
        Serial.print("Receive NoteOFF: ");
        Serial.println(noteNO);
        solenoid.SetSolenoid(noteNo2id(noteNO),0);
        digitalWrite(LED_G, LOW);
      }
    }

    vTaskDelay(1); 
  }
}
