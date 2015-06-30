#define INPUT_FROM_WELDER_CONTROLLER 2
#define INPUT_FROM_SYNCHRONIZATION 3
#define GEN60HZ 5
#define THYRISTOR_1 7
#define THYRISTOR_2 8

const float halfPeriodOfSineWave50Hz = 1.0 / 50.0;
const float halfPeriodOfHalfSineWave50Hz = halfPeriodOfSineWave50Hz / 2.0;
const float halfPeriodOfSineWave60Hz = 1.0 / 60.0;
const float halfPeriodOfHalfSineWave60Hz = halfPeriodOfSineWave60Hz / 2.0; // півперіод половини синусоїди
const float halfPeriodOfSineWave50HzInMicroseconds = 10000.0;

volatile unsigned long timeOfStartPinHighLevelState;
volatile unsigned long periodOutSignal;
volatile unsigned long periodOfSignalDisplacement;
volatile boolean isEnabledOutputSignalGeneration;
volatile boolean stateOfInputPinFromWelderMCU;

void setup() {
  pinMode(INPUT_FROM_WELDER_CONTROLLER, INPUT);
  pinMode(INPUT_FROM_SYNCHRONIZATION, INPUT);
  pinMode(THYRISTOR_1, OUTPUT);
  pinMode(THYRISTOR_2, OUTPUT);
  pinMode(GEN60HZ, OUTPUT);
  attachInterrupt(0, periodFromConntroller, CHANGE);
  attachInterrupt(1, synchronization, RISING);
  setPwmFrequency(GEN60HZ, 1024);
  analogWrite(GEN60HZ, 127);
}

void loop() {
}

void synchronization() {
  if (isEnabledOutputSignalGeneration) {
    unsigned long highStatePeriod = periodOutSignal;
    unsigned long displacementDelay = periodOfSignalDisplacement;
    delayMicroseconds(displacementDelay);
    digitalWrite(THYRISTOR_1, HIGH);
    delayMicroseconds(highStatePeriod);
    digitalWrite(THYRISTOR_1, LOW);
    delayMicroseconds(displacementDelay);
    digitalWrite(THYRISTOR_2, HIGH);
    delayMicroseconds(highStatePeriod);
    digitalWrite(THYRISTOR_2, LOW);
    isEnabledOutputSignalGeneration = false;
  }
}

void periodFromConntroller() {
  stateOfInputPinFromWelderMCU = digitalRead(INPUT_FROM_WELDER_CONTROLLER);
  if (stateOfInputPinFromWelderMCU == HIGH) {
    timeOfStartPinHighLevelState = micros();
  } else {
    volatile unsigned long periodInputSignal =  micros() - timeOfStartPinHighLevelState;
    periodOutSignal = (periodInputSignal * halfPeriodOfHalfSineWave50Hz) / (halfPeriodOfHalfSineWave60Hz);
    periodOfSignalDisplacement = halfPeriodOfSineWave50HzInMicroseconds - periodOutSignal;
    isEnabledOutputSignalGeneration = true;
  }
}

void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if (pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if (pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if (pin == 3 || pin == 11) {
    switch (divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x7; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}
