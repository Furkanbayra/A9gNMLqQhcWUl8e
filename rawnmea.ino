#include <SoftwareSerial.h>
#include <avr/interrupt.h>

const int BUFFER_SIZE = 256;
char buffer[BUFFER_SIZE];
int bufferIndex = 0;
SoftwareSerial NMEA(2, 3);

volatile bool minuteElapsed = false;
volatile int seconds = 0;

String unprocessedData[20]; 
int unprocessedDataIndex = 0;

void setup() {
  while (!Serial)
    ;
  Serial.begin(9600);
  NMEA.begin(9600);
  cli();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 15624;
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS12) | (1 << CS10);
  TIMSK1 |= (1 << OCIE1A);
  sei();
}

void loop() {
  if (minuteElapsed) {
    minuteElapsed = false;
    for (int i = 0; i < unprocessedDataIndex; i++) {
      Serial.println(unprocessedData[i]);
    }
    unprocessedDataIndex = 0;
  }
  if (NMEA.available()) {
    char receivedChar = NMEA.read();

    if (receivedChar == '\n') {
      buffer[bufferIndex] = '\0';
      String data = String(buffer);
      bufferIndex = 0;
      unprocessedData[unprocessedDataIndex++] = data;
      if (unprocessedDataIndex >= 10) {
        unprocessedDataIndex = 0;
      }
    } else {
      buffer[bufferIndex] = receivedChar;
      bufferIndex++;
      if (bufferIndex >= BUFFER_SIZE) {
        bufferIndex = BUFFER_SIZE - 1;
      }
    }
  }
}

ISR(TIMER1_COMPA_vect) {
  seconds++;
  if (seconds >= 10) {  
    minuteElapsed = true;
    seconds = 0;
  }
}
