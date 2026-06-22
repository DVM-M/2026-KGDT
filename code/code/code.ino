#include <PS2Controller_AVR.h>

#define DEBUG_LOG

#define FRONT_LEFT_MOTOR_FORWARD_PIN 2
#define FRONT_LEFT_MOTOR_BACKWARD_PIN 32

#define FRONT_RIGHT_MOTOR_FORWARD_PIN 3
#define FRONT_RIGHT_MOTOR_BACKWARD_PIN 30

#define REAR_LEFT_MOTOR_FORWARD_PIN 4
#define REAR_LEFT_MOTOR_BACKWARD_PIN 28

#define REAR_RIGHT_MOTOR_FORWARD_PIN 5
#define REAR_RIGHT_MOTOR_BACKWARD_PIN 26

int leftJoystickX = 0;
int leftJoystickY = 0;
int leftJoystickAngle = 0;
int leftJoystickMagnitude = 0;

int rightJoystickX = 0;
int rightJoystickY = 0;
int rightJoystickAngle = 0;
int rightJoystickMagnitude = 0;


int sub_speed = 124;
int double_speed = 200;

/* ── Configuration ── */
static const uint8_t ATT_PIN = 42; /* Change to any free digital output */

PS2Controller ps2(ATT_PIN);

/* ── Helpers ── */
static void printError(uint8_t code) {
  switch (code) {
    case PS2X_ERR_NO_CONTROLLER:
      Serial.println(F("Error 1: No controller found."));
      Serial.println(F("         Check wiring and DATA pull-up resistor."));
      break;
    case PS2X_ERR_NOT_ACCEPTING:
      Serial.println(F("Error 2: Controller found but not accepting commands."));
      Serial.println(F("         Try: ps2.byteDelayUs = 30  before ps2.begin()"));
      Serial.println(F("         Or lower clock: ps2.spiClockHz = 62500UL"));
      break;
    case PS2X_ERR_NO_PRESSURE:
      Serial.println(F("Error 3: Pressures requested but not available on this controller."));
      break;
    default:
      Serial.print(F("Unknown error code: "));
      Serial.println(code);
      break;
  }
}

/* ─────────────────────────────────────────────────────────────────────────── */

void setup() {
  Serial.begin(115200);
  configPinout();

  Serial.println(F("PS2Controller_AVR — BasicRead example"));
  Serial.println(F("--------------------------------------"));

  /* ── Tuning (change BEFORE begin() if needed) ───────────────────── *
     * The default 125 kHz clock samples MISO 4 µs after SCK falls,        *
     * matching the proven PS2X_lib bit-bang timing.  This maximises        *
     * compatibility with clones and wireless adapters.                      *
     * If the controller IS detected but produces unstable readings, or      *
     * you want faster polling, try PS2X_CLK_FREQ_FAST (250 kHz).          */
  // ps2.spiClockHz  = PS2X_CLK_FREQ_FAST;   /* 250 kHz — genuine DS2    */
  // ps2.spiClockHz  = PS2X_CLK_FREQ_FASTEST; /* 500 kHz — verified HW   */
  // ps2.byteDelayUs = PS2X_BYTE_DELAY_FAST;  /* 12 µs — faster polling  */

  uint8_t err = ps2.begin(/* pressures= */ false, /* rumble= */ false);

  if (err != PS2X_OK) {
    printError(err);
    Serial.println(F("Halting. Fix the issue then reset the board."));
    while (true) { ; }
  }

  Serial.print(F("Controller found. Type: "));
  switch (ps2.readType()) {
    case PS2X_DUALSHOCK: Serial.println(F("DualShock 2")); break;
    case PS2X_WIRELESS_DS: Serial.println(F("Wireless DualShock")); break;
    case PS2X_GUITAR_HERO: Serial.println(F("Guitar Hero")); break;
    case PS2X_GH_DIGITAL: Serial.println(F("Guitar Hero (digital mode)")); break;
    default: Serial.println(F("Unknown")); break;
  }
  Serial.println();
}

void loop() {
  /* Poll the controller every 50 ms */
  bool ok = ps2.read();

  if (!ok) {
    Serial.println(F("[read() returned false — controller not in analog mode]"));
    stand();
    delay(50);
    return;
  } else {
    ps2.read();

    leftJoystickX = ps2.analog(PSS_LX);
    leftJoystickY = ps2.analog(PSS_LY);
    rightJoystickX = ps2.analog(PSS_RX);
    rightJoystickY = ps2.analog(PSS_RY);

#ifdef DEBUG_LOG
    Serial.print("LX:");
    Serial.print(leftJoystickX);
    Serial.print(", ");
    Serial.print("LY:");
    Serial.print(leftJoystickY);
    Serial.print(", ");
    Serial.print("RX:");
    Serial.print(rightJoystickX);
    Serial.print(", ");
    Serial.print("RY:");
    Serial.print(rightJoystickY);
    Serial.print(" --- ");
#endif  // DEBUG_LOG

    rightJoystickAngle = atan2(-(rightJoystickY - 128), (rightJoystickX - 128)) * 180 / PI;
    rightJoystickMagnitude = constrain(
      sqrt((rightJoystickX - 128) * (rightJoystickX - 128) + (rightJoystickY - 128) * (rightJoystickY - 128)), 
      0, 128
    );
    Serial.print(", Right angle: "); Serial.print(rightJoystickAngle); Serial.print(", ");
    Serial.print(", Right magnitude: "); Serial.print(rightJoystickMagnitude); Serial.print(" - ");

    if (((rightJoystickAngle >= 158) && (rightJoystickAngle <= 180)) || 
        ((rightJoystickAngle < -158) && (rightJoystickAngle >= -180))) {
      Serial.println("ROTATE LEFT");
      rotateLeft(sub_speed);
    } 
    else if (((rightJoystickAngle >= -23) && (rightJoystickAngle <= -1)) || 
             ((rightJoystickAngle >= 0) && (rightJoystickAngle < 23))) {
      Serial.println("ROTATE RIGHT");
      rotateRight(sub_speed);
    }
    else if ((leftJoystickX > 64 && leftJoystickX < 190 && 
              leftJoystickY > 64 && leftJoystickY < 190)) {
      Serial.print(", ");
      Serial.println("Center/No movement");
      stand();
    }
    else {
      leftJoystickAngle = atan2(-(leftJoystickY - 128), (leftJoystickX - 128)) * 180 / PI;
      leftJoystickMagnitude = constrain(
        sqrt((leftJoystickX - 128) * (leftJoystickX - 128) + (leftJoystickY - 128) * (leftJoystickY - 128)), 
        0, 128
      );
      Serial.print(", Left angle: "); Serial.print(leftJoystickAngle); Serial.print(", ");
      Serial.print(", Left magnitude: "); Serial.print(leftJoystickMagnitude); Serial.print(" - ");
    
      if ((leftJoystickAngle >= 67) && (leftJoystickAngle < 113)) {
        moveForward(sub_speed);
        Serial.println(sub_speed);
      }
      else if ((leftJoystickAngle >= 113) && (leftJoystickAngle < 158)) {
        moveRightForward(sub_speed);
        Serial.println(sub_speed);
      }
      else if (((leftJoystickAngle >= 158) && (leftJoystickAngle <= 180)) || 
               ((leftJoystickAngle < -158) && (leftJoystickAngle >= -180))) {
        moveRight(sub_speed);
        Serial.println(sub_speed);
      }
      else if ((leftJoystickAngle >= -158) && (leftJoystickAngle < -113)) {
        moveRightBackward(sub_speed);
        Serial.println(sub_speed);
      }
      else if ((leftJoystickAngle >= -113) && (leftJoystickAngle < -68)) {
        moveBackward(sub_speed);
        Serial.println(sub_speed);
      }
      else if ((leftJoystickAngle >= -68) && (leftJoystickAngle < -23)) {
        moveLeftBackward(sub_speed);
        Serial.println(sub_speed);
      }
      else if (((leftJoystickAngle >= -23) && (leftJoystickAngle <= -1)) || 
               ((leftJoystickAngle >= 0) && (leftJoystickAngle < 23))) {
        moveLeft(sub_speed);
        Serial.println(sub_speed);
      }
      else if ((leftJoystickAngle >= 23) && (leftJoystickAngle < 68)) {
        moveLeftForward(sub_speed);
        Serial.println(sub_speed);
      }
      
      if ((leftJoystickAngle >= 67) && (leftJoystickAngle < 113) && ps2.button(PSB_R2)) {
        moveForward(double_speed);
        Serial.println(double_speed);
      }
      else if ((leftJoystickAngle >= 113) && (leftJoystickAngle < 158) && ps2.button(PSB_R2)) {
        moveRightForward(double_speed);
        Serial.println(double_speed);
      }
      else if ((((leftJoystickAngle >= 158) && (leftJoystickAngle <= 180)) || 
                ((leftJoystickAngle < -158) && (leftJoystickAngle >= -180))) && ps2.button(PSB_R2)) {
        moveRight(double_speed);
        Serial.println(double_speed);
      }
      else if ((leftJoystickAngle >= -158) && (leftJoystickAngle < -113) && ps2.button(PSB_R2)) {
        moveRightBackward(double_speed);
        Serial.println(double_speed);
      }
      else if ((leftJoystickAngle >= -113) && (leftJoystickAngle < -68) && ps2.button(PSB_R2)) {
        moveBackward(double_speed);
        Serial.println(double_speed);
      }
      else if ((leftJoystickAngle >= -68) && (leftJoystickAngle < -23) && ps2.button(PSB_R2)) {
        moveLeftBackward(double_speed);
        Serial.println(double_speed);
      }
      else if ((((leftJoystickAngle >= -23) && (leftJoystickAngle <= -1)) || 
                ((leftJoystickAngle >= 0) && (leftJoystickAngle < 23))) && ps2.button(PSB_R2)) {
        moveLeft(double_speed);
        Serial.println(double_speed);
      }
      else if ((leftJoystickAngle >= 23) && (leftJoystickAngle < 68) && ps2.button(PSB_R2)) {
        moveLeftForward(double_speed);
        Serial.println(double_speed);
      }
    }
  }

  delay(50);
}



void moveForward(uint8_t speed) {
  digitalWrite(FRONT_LEFT_MOTOR_BACKWARD_PIN, HIGH);
  analogWrite(FRONT_LEFT_MOTOR_FORWARD_PIN, speed);
  
  digitalWrite(FRONT_RIGHT_MOTOR_BACKWARD_PIN, HIGH);
  analogWrite(FRONT_RIGHT_MOTOR_FORWARD_PIN, speed);

  digitalWrite(REAR_LEFT_MOTOR_BACKWARD_PIN, HIGH);
  analogWrite(REAR_LEFT_MOTOR_FORWARD_PIN, speed);

  digitalWrite(REAR_RIGHT_MOTOR_BACKWARD_PIN, HIGH);
  analogWrite(REAR_RIGHT_MOTOR_FORWARD_PIN, speed);

  Serial.println("Move Forward");
}

void moveBackward(uint8_t speed) {
  digitalWrite(FRONT_LEFT_MOTOR_BACKWARD_PIN, LOW);
  analogWrite(FRONT_LEFT_MOTOR_FORWARD_PIN, speed);
  
  digitalWrite(FRONT_RIGHT_MOTOR_BACKWARD_PIN, LOW);
  analogWrite(FRONT_RIGHT_MOTOR_FORWARD_PIN, speed);

  digitalWrite(REAR_LEFT_MOTOR_BACKWARD_PIN, LOW);
  analogWrite(REAR_LEFT_MOTOR_FORWARD_PIN, speed);

  digitalWrite(REAR_RIGHT_MOTOR_BACKWARD_PIN, LOW);
  analogWrite(REAR_RIGHT_MOTOR_FORWARD_PIN, speed);

  Serial.println("Move Backward");
}

void moveLeft(uint8_t speed) {
  analogWrite(FRONT_LEFT_MOTOR_FORWARD_PIN, 0);
  analogWrite(FRONT_RIGHT_MOTOR_FORWARD_PIN, speed);
  analogWrite(FRONT_LEFT_MOTOR_BACKWARD_PIN, speed);
  analogWrite(FRONT_RIGHT_MOTOR_BACKWARD_PIN, 0);

  analogWrite(REAR_LEFT_MOTOR_FORWARD_PIN, speed);
  analogWrite(REAR_RIGHT_MOTOR_FORWARD_PIN, 0);
  analogWrite(REAR_LEFT_MOTOR_BACKWARD_PIN, 0);
  analogWrite(REAR_RIGHT_MOTOR_BACKWARD_PIN, speed);

  Serial.println("Move Left");
}

void moveRight(uint8_t speed) {
  analogWrite(FRONT_LEFT_MOTOR_FORWARD_PIN, speed);
  digitalWrite(FRONT_RIGHT_MOTOR_FORWARD_PIN, LOW);
  analogWrite(FRONT_LEFT_MOTOR_BACKWARD_PIN, LOW);
  analogWrite(FRONT_RIGHT_MOTOR_BACKWARD_PIN, speed);

  digitalWrite(REAR_LEFT_MOTOR_FORWARD_PIN, LOW);
  analogWrite(REAR_RIGHT_MOTOR_FORWARD_PIN, speed);
  analogWrite(REAR_LEFT_MOTOR_BACKWARD_PIN, speed);
  digitalWrite(REAR_RIGHT_MOTOR_BACKWARD_PIN, LOW);

  Serial.println("Move Right");
}

void moveLeftForward(uint8_t speed) {
  analogWrite(FRONT_LEFT_MOTOR_FORWARD_PIN, 0);
  analogWrite(FRONT_RIGHT_MOTOR_FORWARD_PIN, speed);
  analogWrite(FRONT_LEFT_MOTOR_BACKWARD_PIN, 0);
  analogWrite(FRONT_RIGHT_MOTOR_BACKWARD_PIN, 0);

  analogWrite(REAR_LEFT_MOTOR_FORWARD_PIN, speed);
  analogWrite(REAR_RIGHT_MOTOR_FORWARD_PIN, 0);
  analogWrite(REAR_LEFT_MOTOR_BACKWARD_PIN, 0);
  analogWrite(REAR_RIGHT_MOTOR_BACKWARD_PIN, 0);

  Serial.println("Move Left Forward");
}

void moveRightForward(uint8_t speed) {
  analogWrite(FRONT_LEFT_MOTOR_FORWARD_PIN, speed);
  analogWrite(FRONT_RIGHT_MOTOR_FORWARD_PIN, 0);
  analogWrite(FRONT_LEFT_MOTOR_BACKWARD_PIN, 0);
  analogWrite(FRONT_RIGHT_MOTOR_BACKWARD_PIN, 0);

  analogWrite(REAR_LEFT_MOTOR_FORWARD_PIN, 0);
  analogWrite(REAR_RIGHT_MOTOR_FORWARD_PIN, speed);
  analogWrite(REAR_LEFT_MOTOR_BACKWARD_PIN, 0);
  analogWrite(REAR_RIGHT_MOTOR_BACKWARD_PIN, 0);

  Serial.println("Move Right Forward");
}

void moveLeftBackward(uint8_t speed) {
  analogWrite(FRONT_LEFT_MOTOR_FORWARD_PIN, 0);
  analogWrite(FRONT_RIGHT_MOTOR_FORWARD_PIN, 0);
  analogWrite(FRONT_LEFT_MOTOR_BACKWARD_PIN, 0);
  analogWrite(FRONT_RIGHT_MOTOR_BACKWARD_PIN, speed);

  analogWrite(REAR_LEFT_MOTOR_FORWARD_PIN, 0);
  analogWrite(REAR_RIGHT_MOTOR_FORWARD_PIN, 0);
  analogWrite(REAR_LEFT_MOTOR_BACKWARD_PIN, speed);
  analogWrite(REAR_RIGHT_MOTOR_BACKWARD_PIN, 0);

  Serial.println("Move Left Backward");
}

void moveRightBackward(uint8_t speed) {
  analogWrite(FRONT_LEFT_MOTOR_FORWARD_PIN, 0);
  analogWrite(FRONT_RIGHT_MOTOR_FORWARD_PIN, 0);
  analogWrite(FRONT_LEFT_MOTOR_BACKWARD_PIN, speed);
  analogWrite(FRONT_RIGHT_MOTOR_BACKWARD_PIN, 0);

  analogWrite(REAR_LEFT_MOTOR_FORWARD_PIN, 0);
  analogWrite(REAR_RIGHT_MOTOR_FORWARD_PIN, 0);
  analogWrite(REAR_LEFT_MOTOR_BACKWARD_PIN, 0);
  analogWrite(REAR_RIGHT_MOTOR_BACKWARD_PIN, speed);

  Serial.println("Move Right Forward");
}

void rotateLeft(uint8_t speed) {
  analogWrite(FRONT_LEFT_MOTOR_FORWARD_PIN, 0);
  analogWrite(FRONT_RIGHT_MOTOR_FORWARD_PIN, speed);
  analogWrite(FRONT_LEFT_MOTOR_BACKWARD_PIN, speed);
  analogWrite(FRONT_RIGHT_MOTOR_BACKWARD_PIN, 0);

  analogWrite(REAR_LEFT_MOTOR_FORWARD_PIN, 0);
  analogWrite(REAR_RIGHT_MOTOR_FORWARD_PIN, speed);
  analogWrite(REAR_LEFT_MOTOR_BACKWARD_PIN, speed);
  analogWrite(REAR_RIGHT_MOTOR_BACKWARD_PIN, 0);

  Serial.println("Rotate Left");
}

void rotateRight(uint8_t speed) {
  analogWrite(FRONT_LEFT_MOTOR_FORWARD_PIN, speed);
  analogWrite(FRONT_RIGHT_MOTOR_FORWARD_PIN, 0);
  analogWrite(FRONT_LEFT_MOTOR_BACKWARD_PIN, 0);
  analogWrite(FRONT_RIGHT_MOTOR_BACKWARD_PIN, speed);

  analogWrite(REAR_LEFT_MOTOR_FORWARD_PIN, speed);
  analogWrite(REAR_RIGHT_MOTOR_FORWARD_PIN, 0);
  analogWrite(REAR_LEFT_MOTOR_BACKWARD_PIN, 0);
  analogWrite(REAR_RIGHT_MOTOR_BACKWARD_PIN, speed);

  Serial.println("Rotate Right");
}

void stand() {
  analogWrite(FRONT_LEFT_MOTOR_FORWARD_PIN, 0);
  analogWrite(FRONT_RIGHT_MOTOR_FORWARD_PIN, 0);
  analogWrite(FRONT_LEFT_MOTOR_BACKWARD_PIN, 0);
  analogWrite(FRONT_RIGHT_MOTOR_BACKWARD_PIN, 0);

  analogWrite(REAR_LEFT_MOTOR_FORWARD_PIN, 0);
  analogWrite(REAR_RIGHT_MOTOR_FORWARD_PIN, 0);
  analogWrite(REAR_LEFT_MOTOR_BACKWARD_PIN, 0);
  analogWrite(REAR_RIGHT_MOTOR_BACKWARD_PIN, 0);

  Serial.println("Stand");
}

void configPinout() {
  pinMode(FRONT_LEFT_MOTOR_FORWARD_PIN, OUTPUT);
  pinMode(FRONT_LEFT_MOTOR_BACKWARD_PIN, OUTPUT);
  pinMode(FRONT_RIGHT_MOTOR_FORWARD_PIN, OUTPUT);
  pinMode(FRONT_RIGHT_MOTOR_BACKWARD_PIN, OUTPUT);
  pinMode(REAR_LEFT_MOTOR_FORWARD_PIN, OUTPUT);
  pinMode(REAR_RIGHT_MOTOR_BACKWARD_PIN, OUTPUT);
  pinMode(REAR_RIGHT_MOTOR_FORWARD_PIN, OUTPUT);
  pinMode(REAR_RIGHT_MOTOR_BACKWARD_PIN, OUTPUT);
}
