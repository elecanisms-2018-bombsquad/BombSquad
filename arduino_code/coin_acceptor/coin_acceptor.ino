
const int buttonPin = 2;    // the number of the coin acceptor
const int relayPin = 12;      // the number of the LED pin
const int lightPin = 7;
const int resetPin = 8;
const int resetPinGND = 9;

int relayState = LOW;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int lastButtonState = HIGH;   // the previous reading from the input pin

unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 30;    // the debounce time; increase if the output flickers

boolean turningOn = true;


void setup() {
  pinMode(buttonPin, INPUT);
  pinMode(relayPin, OUTPUT);
  pinMode(lightPin, OUTPUT);
  pinMode(resetPinGND, OUTPUT);
  digitalWrite(resetPinGND, LOW);
  pinMode(resetPin, INPUT_PULLUP);
  randomSeed(analogRead(0));
  digitalWrite(relayPin, relayState);
}

void loop() {
  if (digitalRead(resetPin) == LOW) { // restart button
    relayState = LOW;
    return;
  }
  int reading = digitalRead(buttonPin);
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    
    if (reading != buttonState) {
      buttonState = reading;

      if (buttonState == LOW) {
        relayState = !relayState;
      }
    }
  }

  digitalWrite(relayPin, relayState);

  // Flicker on startup
  if ((relayState == HIGH) && turningOn) {
    turningOn = false;
    delay(4000);  
    for (int i = 0; i < 10; i++) {
      digitalWrite(lightPin, HIGH);
      delay(random(100 - i * 9));
      digitalWrite(lightPin, LOW);
      delay(random(100 - i * 9));
    }
    digitalWrite(lightPin, HIGH);
  } 

  // Randomly flicker
  if (relayState == HIGH) {
    if (random(100000) < 10) {
      digitalWrite(lightPin, LOW);
      delay(random(100,300));
      for (int i = 0; i < 10; i++) {
        digitalWrite(lightPin, HIGH);
        delay(random(100 - i * 9));
        digitalWrite(lightPin, LOW);
        delay(random(100 - i * 9));
      }
      digitalWrite(lightPin, HIGH);
    }
    if (random(100000) < 50) {
      digitalWrite(lightPin, LOW);
      delay(random(10,100));
      for (int i = 0; i < 6; i++) {
        digitalWrite(lightPin, HIGH);
        delay(random(60 - i * 9));
        digitalWrite(lightPin, LOW);
        delay(random(60 - i * 9));
      }
      digitalWrite(lightPin, HIGH);
    }
  }
  
  // reset if we're turning off.
  if (relayState == LOW) {
    digitalWrite(lightPin, LOW); 
    turningOn = true;
  }
  lastButtonState = reading;
  delay(10);
}
