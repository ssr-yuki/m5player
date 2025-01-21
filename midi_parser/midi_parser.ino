#include <M5AtomS3.h>
#include <MIDI.h>

MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);

void setup() {
  auto cfg = M5.config();
  AtomS3.begin(cfg);

  // for serial printing
  Serial.begin(115200);

  // for MIDI communication
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

  Serial.println("Finished initialisation");
}


void loop() {
  delay(1000);
}


void MIDIReceptionTask(void *parameters) {
  while (1) {
    if (MIDI.read()) {
      const int noteNO = MIDI.getData1();
      if (MIDI.getType() == midi::NoteOn) {
        Serial.print("Receive NoteON: ");
        Serial.println(noteNO);
      } else if (MIDI.getType() == midi::NoteOff) {
        Serial.print("Receive NoteOFF: ");
        Serial.println(noteNO);
      }
    }

    vTaskDelay(1); 
  }
}
