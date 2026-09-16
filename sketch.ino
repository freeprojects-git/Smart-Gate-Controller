enum State {
  CLOSED,
  OPENING,
  OPEN,
  CLOSING,
  STOPPED,
  ERROR_STATE
};

State state = CLOSED;

const int BTN_OPEN  = 2;
const int BTN_CLOSE = 3;
const int BTN_STOP  = 4;
const int BTN_RESET = 5;

const int LIMIT_OPEN  = 6;
const int LIMIT_CLOSE = 7;

const int RELAY_OPEN  = 8;
const int RELAY_CLOSE = 9;

const int LED_OPEN  = 10;
const int LED_CLOSE = 11;
const int LED_ERROR = 12;

unsigned long movementStart = 0;
const unsigned long TIMEOUT = 10000;

bool lastOpenButton = HIGH;
bool lastCloseButton = HIGH;
bool lastStopButton = HIGH;
bool lastResetButton = HIGH;

void setup() {
  Serial.begin(9600);

  pinMode(BTN_OPEN, INPUT_PULLUP);
  pinMode(BTN_CLOSE, INPUT_PULLUP);
  pinMode(BTN_STOP, INPUT_PULLUP);
  pinMode(BTN_RESET, INPUT_PULLUP);

  pinMode(LIMIT_OPEN, INPUT_PULLUP);
  pinMode(LIMIT_CLOSE, INPUT_PULLUP);

  pinMode(RELAY_OPEN, OUTPUT);
  pinMode(RELAY_CLOSE, OUTPUT);

  pinMode(LED_OPEN, OUTPUT);
  pinMode(LED_CLOSE, OUTPUT);
  pinMode(LED_ERROR, OUTPUT);

  setState(CLOSED);

  Serial.println("Smart Gate Controller started");
  Serial.println("State: CLOSED");
}

void loop() {

  checkBothLimits();

  bool openPressed =
    lastOpenButton == HIGH && digitalRead(BTN_OPEN) == LOW;

  bool closePressed =
    lastCloseButton == HIGH && digitalRead(BTN_CLOSE) == LOW;

  bool stopPressed =
    lastStopButton == HIGH && digitalRead(BTN_STOP) == LOW;

  bool resetPressed =
    lastResetButton == HIGH && digitalRead(BTN_RESET) == LOW;

  lastOpenButton = digitalRead(BTN_OPEN);
  lastCloseButton = digitalRead(BTN_CLOSE);
  lastStopButton = digitalRead(BTN_STOP);
  lastResetButton = digitalRead(BTN_RESET);

  // RESET from ERROR
  if (resetPressed && state == ERROR_STATE) {
    setState(STOPPED);
  }

  // STOP
  if (stopPressed &&
      state != ERROR_STATE &&
      state != CLOSED &&
      state != OPEN) {
    setState(STOPPED);
  }

  // OPEN command
  if (openPressed) {
    handleOpen();
  }

  // CLOSE command
  if (closePressed) {
    handleClose();
  }

  // Limit switches
  if (state == OPENING && digitalRead(LIMIT_OPEN) == LOW) {
    setState(OPEN);
  }

  if (state == CLOSING && digitalRead(LIMIT_CLOSE) == LOW) {
    setState(CLOSED);
  }

  // Movement timeout
  if ((state == OPENING || state == CLOSING) &&
      millis() - movementStart > TIMEOUT) {

    Serial.println("ERROR: movement timeout");
    setState(ERROR_STATE);
  }

  // Serial commands
  checkSerial();

  delay(20);
}

void handleOpen() {

  if (state == ERROR_STATE) {
    Serial.println("OPEN ignored: ERROR");
    return;
  }

  if (state == OPEN) {
    Serial.println("OPEN ignored: already OPEN");
    return;
  }

  if (state == OPENING) {
    Serial.println("OPEN ignored: already OPENING");
    return;
  }

  // Safe transition from CLOSING
  if (state == CLOSING) {
    digitalWrite(RELAY_CLOSE, LOW);
    Serial.println("Safe transition: CLOSING -> OPENING");
  }

  startOpening();
}

void handleClose() {

  if (state == ERROR_STATE) {
    Serial.println("CLOSE ignored: ERROR");
    return;
  }

  if (state == CLOSED) {
    Serial.println("CLOSE ignored: already CLOSED");
    return;
  }

  if (state == CLOSING) {
    Serial.println("CLOSE ignored: already CLOSING");
    return;
  }

  // Safe transition from OPENING
  if (state == OPENING) {
    digitalWrite(RELAY_OPEN, LOW);
    Serial.println("Safe transition: OPENING -> CLOSING");
  }

  startClosing();
}

void startOpening() {

  digitalWrite(RELAY_CLOSE, LOW);
  digitalWrite(RELAY_OPEN, HIGH);

  movementStart = millis();

  setState(OPENING);
}

void startClosing() {

  digitalWrite(RELAY_OPEN, LOW);
  digitalWrite(RELAY_CLOSE, HIGH);

  movementStart = millis();

  setState(CLOSING);
}

void setState(State newState) {

  state = newState;

  // Default outputs OFF
  digitalWrite(RELAY_OPEN, LOW);
  digitalWrite(RELAY_CLOSE, LOW);

  digitalWrite(LED_OPEN, LOW);
  digitalWrite(LED_CLOSE, LOW);
  digitalWrite(LED_ERROR, LOW);

  switch (state) {

    case CLOSED:
      Serial.println("State: CLOSED");
      break;

    case OPENING:
      digitalWrite(RELAY_OPEN, HIGH);
      digitalWrite(LED_OPEN, HIGH);
      Serial.println("State: OPENING");
      break;

    case OPEN:
      digitalWrite(LED_OPEN, HIGH);
      Serial.println("State: OPEN");
      break;

    case CLOSING:
      digitalWrite(RELAY_CLOSE, HIGH);
      digitalWrite(LED_CLOSE, HIGH);
      Serial.println("State: CLOSING");
      break;

    case STOPPED:
      Serial.println("State: STOPPED");
      break;

    case ERROR_STATE:
      digitalWrite(LED_ERROR, HIGH);
      Serial.println("State: ERROR");
      break;
  }
}

void checkBothLimits() {

  if (digitalRead(LIMIT_OPEN) == LOW &&
      digitalRead(LIMIT_CLOSE) == LOW) {

    digitalWrite(RELAY_OPEN, LOW);
    digitalWrite(RELAY_CLOSE, LOW);

    Serial.println("ERROR: both limits active");

    state = ERROR_STATE;

    digitalWrite(LED_OPEN, LOW);
    digitalWrite(LED_CLOSE, LOW);
    digitalWrite(LED_ERROR, HIGH);
  }
}

void checkSerial() {

  if (Serial.available()) {

    char command = Serial.read();

    switch (command) {

      case 'O':
      case 'o':
        handleOpen();
        break;

      case 'C':
      case 'c':
        handleClose();
        break;

      case 'S':
      case 's':
        setState(STOPPED);
        break;

      case 'R':
      case 'r':
        if (state == ERROR_STATE) {
          setState(STOPPED);
        }
        break;

      default:
        Serial.print("Invalid command: ");
        Serial.println(command);
        break;
    }
  }
}
