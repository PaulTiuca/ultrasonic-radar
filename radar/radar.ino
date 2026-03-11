#include <Servo.h>

const int servoPin = 9;
const int trigPin  = 10;
const int echoPin  = 11;
const int buzzerPin = 8;

Servo radarServo;
int angle = 0;
int dir = 1;
bool radarBlocked = false;
unsigned long lastServoMove = 0;
const unsigned long servoInterval = 15;

unsigned long lastSensorRead = 0;
const unsigned long sensorInterval = 50;
long currentDistance = -1;

unsigned long lastBuzzerToggle = 0;
bool buzzerOn = false;



void setup() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);

  radarServo.attach(servoPin);
  radarServo.write(angle);

  digitalWrite(buzzerPin, LOW);

  Serial.begin(9600);
}

void loop() {
  updateSensor();

  if (currentDistance > 0 && currentDistance < 40) {
    radarBlocked = true;
  } else {
    radarBlocked = false;
  }

  updateServo();
  updateBuzzer(currentDistance);

  Serial.print(angle);
  Serial.print(",");
  Serial.println(currentDistance);
}

long readDistanceOnce() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);

  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH, 25000);
  if (duration == 0) return -1;

  return duration * 0.034 / 2;
}

long getFilteredDistance() {
  long sum = 0;
  int valid = 0;

  for (int i = 0; i < 3; i++) {
    long d = readDistanceOnce();
    if (d > 2 && d < 40) {
      sum += d;
      valid++;
    }
  }

  if (valid == 0) return -1;
  return sum / valid;
}



void updateSensor() {
  if (millis() - lastSensorRead >= sensorInterval) {
    lastSensorRead = millis();
    currentDistance = getFilteredDistance();
  }
}

void updateServo() {
  if (radarBlocked) return;

  if (millis() - lastServoMove >= servoInterval) {
    lastServoMove = millis();

    angle += dir;

    if (angle >= 180 || angle <= 0) {
      dir *= -1;
    }

    radarServo.write(angle);
  }
}



void updateBuzzer(long distance) {
  if (distance <= 0 || distance > 40) {
    digitalWrite(buzzerPin, LOW);
    buzzerOn = false;
    return;
  }

  int interval = map(distance, 2, 40, 100, 1000);
  interval = constrain(interval, 100, 1000);

  if (millis() - lastBuzzerToggle >= interval) {
    lastBuzzerToggle = millis();
    buzzerOn = !buzzerOn;
    digitalWrite(buzzerPin, buzzerOn ? HIGH : LOW);
  }
}
