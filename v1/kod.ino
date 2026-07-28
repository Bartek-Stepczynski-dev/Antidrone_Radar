#include <Servo.h>

// Definicje pinów i stałych
const int TRIG_PIN = 12;
const int ECHO_PIN = 11;
const int LED_PIN = 13;
const int SERVO_PIN = 9;

const int MIN_ANGLE = 15;
const int MAX_ANGLE = 165;
const int DETECT_DIST = 30; // [cm]

Servo radarServo;

int servoAngle = MIN_ANGLE;
int sweepDirection = 1; // przesunięcie o jeden stopień

// Zmienne do obsługi czasu za pomocą  funkcji millis()
unsigned long czasOstatniegoKrokuSerwa = 0;
unsigned long czasWykryciaCelu = 0;
unsigned long czasOstatniegoPomiaru = 0; 
bool celJestAktywny = false;

// Stałe czasowe. 
const unsigned long INTERWAL_RUCHU_SERWA = 30; // co ile ms krok serwa
const unsigned long CZAS_ZATRZYMANIA_NA_CELU = 500; // czas zatrzymania po zgubieniu celu
const unsigned long INTERWAL_POMIARU = 50; // Pomiar maksymalnie co 50 ms!

// Przechowuje ostatnią zmierzoną odległość
long ostatniaOdleglosc = 0;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT); // bo on  odbiera sygnały
  pinMode(LED_PIN, OUTPUT);
  
  radarServo.attach(SERVO_PIN);
  radarServo.write(MIN_ANGLE);
  digitalWrite(LED_PIN, LOW); // początek układu z zgaszaną dioda.
  
  delay(500); 
}

void loop() {
  unsigned long aktualnyCzas = millis();

  //Mierzy odległość co 50 ms, żeby nie zablokować czujnika.
  if (aktualnyCzas - czasOstatniegoPomiaru >= INTERWAL_POMIARU) {
    czasOstatniegoPomiaru = aktualnyCzas;
    ostatniaOdleglosc = zmierzOdleglosc();

    // Wizualizacja w Serial Monitorze
    if (ostatniaOdleglosc > 0 && ostatniaOdleglosc <= 400) {
      Serial.print("Kat: "); Serial.print(servoAngle);
      Serial.print(" | Odleglosc: "); Serial.print(ostatniaOdleglosc);
      Serial.println(" cm");
    } else {
      Serial.println("Poza zasiegiem");
    }
  }

  //Logika detekcji celu bazująca na ostatniej znanej odległości
  if (ostatniaOdleglosc > 0 && ostatniaOdleglosc <= DETECT_DIST) {
    digitalWrite(LED_PIN, HIGH);
    celJestAktywny = true;
    czasWykryciaCelu = aktualnyCzas;
  } else {
    if (aktualnyCzas - czasWykryciaCelu >= CZAS_ZATRZYMANIA_NA_CELU) {
      digitalWrite(LED_PIN, LOW);
      // Zapobiega ruszaniu przed upływem czasu
      celJestAktywny = false;
    }
  }

  // Sposób ruchu serwa gdy nie ma aktywnego celu
  if (!celJestAktywny) {
    if (aktualnyCzas - czasOstatniegoKrokuSerwa >= INTERWAL_RUCHU_SERWA) {
      czasOstatniegoKrokuSerwa = aktualnyCzas;
      
      servoAngle += sweepDirection;
      radarServo.write(servoAngle);

      if (servoAngle >= MAX_ANGLE || servoAngle <= MIN_ANGLE) {
        sweepDirection = -sweepDirection; // odwraca ruch serva przy jego zdeklarowanych maksymanych kątach
      }
    }
  }
}

// Funkcja pomiarowa
long zmierzOdleglosc() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long czasTrwania = pulseIn(ECHO_PIN, HIGH); 
  long odleglosc_cm = czasTrwania * 0.0343 / 2;
  return odleglosc_cm; // zwraca wartości
}
