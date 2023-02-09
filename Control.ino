// Motorcontroller:
// Rotes Kabel von Peltier in OUT1, Schwarzes in OUT2
// ENA an Pin 6, IN1 an PIN 5, IN2 an PIN 4

// Temperaturfühler:
// CS an PIN 10, SDI an PIN 11, SOO an PIN 12, CLK an PIN 13


#include <Adafruit_MAX31865.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h> 
#define ErrorHistorySize 20

LiquidCrystal_I2C lcd(0x27, 16, 2); 
Adafruit_MAX31865 TempSensor = Adafruit_MAX31865(10, 11, 12, 13);
uint16_t rtd;
float ratio, R;
bool heat;
int heat_value, arraypos = 0, derivativewait = 0;
double Vr = 18, tn = 65000, tv = 10000, dt = 50.0;
double ErrorHistory[ErrorHistorySize];
double error, integral, derivative = 0, controlSignal, T, setpoint, prevSetpoint, sum1, avg1, sum2, avg2;


// Werte des Referenz- und Messwiderstandes. In diesem Fall 430 und 100 Ohm
#define RREF      430.0
#define RNOMINAL  100.0

// Pins des Controllers des Peltier-Elements
int en = 6, in1 = 4, in2 = 5;

#define ARRAY_SIZE 33  // Größe des Arrays

float myArray[ARRAY_SIZE][2] = {
  {94, 2},
  {94, 0.25},
  {65, 0.3},
  {72, 1},
  {94, 0.25},
  {65, 0.3},
  {72, 1},
  {94, 0.25},
  {65, 0.3},
  {72, 1},
  {94, 0.25},
  {65, 0.3},
  {72, 1},
  {94, 0.25},
  {65, 0.3},
  {72, 1},
  {94, 0.25},
  {65, 0.3},
  {72, 1},
  {94, 0.25},
  {65, 0.3},
  {72, 1},
  {94, 0.25},
  {65, 0.3},
  {72, 1},
  {94, 0.25},
  {65, 0.3},
  {72, 1},
  {94, 0.25},
  {65, 0.3},
  {72, 1},
  {72, 5},
  {4, 1},
};

// Methode um Heizen oder Kühlen umzuschalten, sowie Intensität festzulegen
int setTemp(bool heat, float heat_value) {
  //Ob gekühlt oder gehizt wird, wird durch die Richtung der angelegten Spannung bestimmt.
  //Ist heat true, wird die Quickcool-Seite des Peltiers kalt, ansonsten warm.
  if(heat == true){
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  }else{
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);    
  }
  //Um den Betrag der Spannung festzulegen, wird über PWM ein "analoges" Signal an den Motorcontroller gesendet.
  //Es werden Werte zwischen 0 und 20 akzeptiert, geringere oder höhere Werte werden als die entsprechende Grenze interpretiert.
  heat_value = constrain(heat_value, 0, 255);
//  heat_value = map(heat_value, 0, 20, 0, 255);
  analogWrite(en, heat_value);
  return(heat_value);
}

void writeTemp(){
  //Sendet die Messwerte an den seriellen Monitor
 // Serial.print("RTD value: "); Serial.println(rtd);
 // Serial.print("Ratio = "); Serial.println(ratio,8);
 // Serial.print("Resistance = "); Serial.println(R);
 // Serial.print("Temperature = "); Serial.println(T);
 // Serial.println();
  Serial.print(millis()); Serial.print(",");
  Serial.print(T); Serial.print(",");
  Serial.print(error); Serial.print(",");
  Serial.print(integral); Serial.print(",");
  Serial.print(derivative, 20); Serial.print(",");
  Serial.println(heat_value);

  //Sendet die Messwerte an den LCD
  lcd.setCursor(0, 0);
  lcd.print(myArray[arraypos][1]); 
  lcd.setCursor(0, 1);
  lcd.print(T); 
  lcd.setCursor(6, 1);
  lcd.print("C");

  lcd.setCursor(8, 0);
  lcd.print(heat_value);
  lcd.setCursor(8, 1);
  lcd.print(setpoint); 
  lcd.setCursor(14, 1);
  lcd.print("C");
}

void setup() {
  //Öffnet die Verbindung zum seriellen Monitor
  Serial.begin(115200);
  //Im Setup wird der LCD gestartet 
  lcd.init();
  lcd.backlight();
  //Legt die Art des Thermosensors fest
  TempSensor.begin(MAX31865_2WIRE);
  //Initiiert die Pins für die Peltier-Kontrolle
  pinMode(en, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);

  for (int i = 0; i < ARRAY_SIZE; i++) {
     myArray[i][1] = myArray[i][1] * 60000;
  }

  for (int i = 0; i < ErrorHistorySize; i++) {
    ErrorHistory[i] = 0;
  }
}


void loop() {
  if(myArray[arraypos][1] <= 0){
    if(arraypos < (ARRAY_SIZE-1)){
      arraypos++;
      prevSetpoint = setpoint;
      setpoint = myArray[arraypos][0];
      if (setpoint != prevSetpoint)  {    
        integral = 0;
      }
      for (int i = 0; i < ErrorHistorySize; i++) {
        ErrorHistory[i] = 0;
       }
    }
  }

  //Liest den momentanen Widerstand und die resultierende Temperatur aus
  rtd = TempSensor.readRTD();
  ratio = rtd;
  ratio /= 32768;
  T = TempSensor.temperature(RNOMINAL, RREF);
  R = RREF*ratio;

  if((T > (setpoint - 1)) && (T < (setpoint + 1))){
    myArray[arraypos][1] = myArray[arraypos][1] - dt;
  }

  error = setpoint - T;


  if((-20 < error) && (error < 20)){
   integral += error * dt;
  }

  for (int i = 0; i < ErrorHistorySize - 1; i++) {
    ErrorHistory[i] = ErrorHistory[i + 1];
  }

  ErrorHistory[ErrorHistorySize - 1] = error;
  
  sum1 = 0;
  for (int i = 0; i < ErrorHistorySize / 2; i++){
    sum1 += ErrorHistory[i];
  }
  avg1 = sum1 / (ErrorHistorySize / 2);

  sum2 = 0;
  for (int i = ErrorHistorySize / 2; i < ErrorHistorySize; i++){
    sum2 += ErrorHistory[i];
  }
  avg2 = sum2 / (ErrorHistorySize / 2);

  if (derivativewait > 20) {
    derivative = (avg2 - avg1) / (ErrorHistorySize * dt);
  }else{
    derivativewait++;
  }

  controlSignal = Vr*(error + (1/tn) * integral + tv * derivative);

  if(error > 0){
    heat_value = setTemp(heat=true, heat_value=abs(controlSignal));
  }
  if(error < 0){
    heat_value = setTemp(heat=false, heat_value=abs(controlSignal));
  }
  writeTemp();
  delay(dt);
}