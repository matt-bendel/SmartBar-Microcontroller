#include <Adafruit_NeoPixel.h>

#define LED_PIN 6       // the pin for the LED strip
#define LED_COUNT 11

#define Z_STEP 7
#define Z_DIR 8
#define Z_EN 9

#define X_STEP 10
#define X_DIR 11
#define X_EN 12

#define STEPS_TO_DISPENSE 2100
const int  CONTACT_PIN = 2;    // the pin that the pushbutton is attached to
const int PUMP_PINS[] = {45, 43, 41, 39, 37, 35, 33, 31};
const int NUM_STEPS[] = {25, 200, 400, 600, 800, 1000, 1200};

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

const uint32_t RED = strip.Color(255, 0, 0);
const uint32_t GREEN = strip.Color(0, 255, 0);
const uint32_t BLUE = strip.Color(0, 0, 255);
const uint32_t MAGENTA = strip.Color(255, 0, 255);
const uint32_t OFF = strip.Color(0, 0, 0);

int buttonState = 1;         // current state of the button

//---------------------

//The below are for running the machine:

int drinkRequested = false;
int currentPos = 0;
int nextPos = -1;

//---------------------

void home_stepper() {
  digitalWrite(X_EN, LOW);
  digitalWrite(X_DIR, LOW);

  while (digitalRead(CONTACT_PIN)) {
    for (int i = 0; i < 5; i++) {
      if (!digitalRead(CONTACT_PIN)) {
        break;
      }
      
      digitalWrite(X_STEP, HIGH);
      delay(2);
      digitalWrite(X_STEP, LOW);
      delay(2);
    }
  }

  digitalWrite(X_STEP, LOW);
  digitalWrite(X_EN, HIGH);
  
  delay(1000);
  
  digitalWrite(X_EN, LOW);
  digitalWrite(X_DIR, HIGH);
  
  for (int i = 0; i < 25; i++) {
      digitalWrite(X_STEP, HIGH);
      delay(2);
      digitalWrite(X_STEP, LOW);
      delay(2);
  }

  digitalWrite(X_EN, HIGH);
}

void stepper_drive(int location) {
  //Set LED to Magenta
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, MAGENTA);
  }
  
  strip.show();

  uint8_t dir;
  uint16_t steps = abs(NUM_STEPS[nextPos] - NUM_STEPS[currentPos]);
  
  //Set Direction and Num Steps
  if (nextPos > currentPos) {
    dir = HIGH;
  } else if (nextPos < currentPos) {
    dir = LOW;
  } else {
    return;
  }

  //Drive To Next Position
  digitalWrite(X_EN, LOW);
  digitalWrite(X_DIR, dir);
  
  for (int i = 0; i < steps; i++) {
    digitalWrite(X_STEP, HIGH);
    delay(2);
    digitalWrite(X_STEP, LOW);
    delay(2);
  }

  digitalWrite(X_EN, HIGH);
  
  currentPos = nextPos;
  
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, BLUE);
  }

  strip.show();
  
  return;
}

void dispense_liquor(int num_shots) {
  for (int i = 0; i < LED_COUNT; i++) {
    strip.setPixelColor(i, OFF);
  }

  strip.show();

  for (int j = 0; j < num_shots; j++) {
    digitalWrite(Z_STEP, LOW);
    
    delay(500);
    
    digitalWrite(Z_EN, LOW);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
    digitalWrite(Z_DIR, HIGH);
    
    for (int i = 0; i < 500; i++) {
      digitalWrite(Z_STEP, HIGH);
      delay(5);
      digitalWrite(Z_STEP, LOW);
      delay(5);
    }
  
    digitalWrite(Z_STEP, LOW);
    digitalWrite(Z_EN, HIGH);
  
    delay(3000);
  
    digitalWrite(Z_EN, LOW);
    digitalWrite(Z_DIR, LOW);
    
    for (int i = 0; i < 500; i++) {
      digitalWrite(Z_STEP, HIGH);
      delay(5);
      digitalWrite(Z_STEP, LOW);
      delay(5);
    }
    
    digitalWrite(Z_EN, HIGH);
    
    delay(1000);
  
    digitalWrite(Z_EN, HIGH);
    
    delay(1000);
  }
  return;
}

void dispense_mixer(int pump) {
  digitalWrite(PUMP_PINS[pump - 1], LOW);
  delay(2500);
  digitalWrite(PUMP_PINS[pump - 1], HIGH);

  return;
}

uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
void rainbow_cycle(uint8_t wait) {
  uint16_t i, j;
  for (i = 0; i < 256*2; i++) {
    for (j = 0; j < strip.numPixels(); j++) {
      strip.setPixelColor(j, Wheel(((j*256 / strip.numPixels()) + i) & 255));
    }

    strip.show();
    delay(wait);
  }
}

void setup() {
  // initialize serial communication:
  Serial.begin(9600);
  
  // initialize the button pin as a input:
  pinMode(CONTACT_PIN, INPUT_PULLUP);

  digitalWrite(X_EN, HIGH);
  digitalWrite(Z_EN, HIGH);

  pinMode(X_STEP, OUTPUT);
  pinMode(X_DIR, OUTPUT);
  pinMode(X_EN, OUTPUT);
  
  pinMode(Z_STEP, OUTPUT);
  pinMode(Z_DIR, OUTPUT);
  pinMode(Z_EN, OUTPUT);
  
  //Initialize Pumps
  for (int i = 0; i < 8; i++) {
    digitalWrite(PUMP_PINS[i], HIGH);
    pinMode(PUMP_PINS[i], OUTPUT);
  }

//-------------------------------

// For the LED Strip
   strip.setBrightness(35);
   strip.begin();

   for (int i = 0; i < LED_COUNT; i++) {
     strip.setPixelColor(i, RED);
   }
    
   strip.show();

   home_stepper();

   for (int i = 0; i < LED_COUNT; i++) {
     strip.setPixelColor(i, GREEN);
   }
  
   strip.show();

   Serial.write("ready\n");
}


void loop(){
  String data = "";

  if (Serial.available() > 0) {
    data = Serial.readStringUntil('\n');
  }
  
  if (data == "init") {
    int location = Serial.readStringUntil('\n').toInt();
    while (location != 0) {
      nextPos = location;
      stepper_drive(nextPos);
      
      int num_shots = Serial.readStringUntil('\n').toInt();
      
      delay(1500);
      
      dispense_liquor(num_shots);
      location = Serial.readStringUntil('\n').toInt();
    }

    // Set LED to Magenta
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, MAGENTA);
    }

    strip.show();
    
    home_stepper();
    
    currentPos = 0;
    nextPos = -1;
    
    //Set LED to Blue
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, BLUE);
    }

    strip.show();
    
    delay(1500);
    
    int pump = Serial.readStringUntil('\n').toInt();
    while (pump != 69) {
      dispense_mixer(pump);
      pump = Serial.readStringUntil('\n').toInt();
    }

    Serial.write("complete\n");
    
    // SET LED TO RAINBOW
    rainbow_cycle(20);
    
    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, RED);
    }

    strip.show();
    
    delay(3000);

    for (int i = 0; i < LED_COUNT; i++) {
      strip.setPixelColor(i, GREEN);
    }

    strip.show();

    Serial.write("ready\n");
  }
}
