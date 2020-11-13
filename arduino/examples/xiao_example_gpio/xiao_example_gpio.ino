#define tact0 8
#define tact1 9
#define tact2 10
#define pinAir 6
#define pinWell 7
#define pinHeater 3

void setup(){
  pinMode(tact0, INPUT);
  pinMode(tact1, INPUT);
  pinMode(tact2, INPUT);
  pinMode(pinHeater, OUTPUT);
  Serial.begin(9600);
}

int air, well;
int heater = 0;

void loop()
{
  if(digitalRead(tact0) == HIGH) {
    Serial.print("tact0=HIGH ");
  } else {
    Serial.print("tact0=LOW  ");
  }
  
  if(digitalRead(tact1) == HIGH) {
    Serial.print("tact1=HIGH ");
  } else {
    Serial.print("tact1=LOW  ");
  }
  
  if(digitalRead(tact2) == HIGH) {
    Serial.print("tact2=HIGH ");
  } else {
    Serial.print("tact2=LOW  ");
  }
  
  air = analogRead(pinAir);
  Serial.print("air=");
  Serial.print(air);
  Serial.print(" ");
  
  well = analogRead(pinWell);
  Serial.print("well=");
  Serial.print(well);
  Serial.print(" ");
  
  if(heater < 30) {
    Serial.print("heater=HIGH ");
    digitalWrite(pinHeater, HIGH);
  } else if(heater < 60) {
    Serial.print("heater=LOW  ");
    digitalWrite(pinHeater, LOW);
  } else {
    heater = 0;
  }
  heater++;
  
  Serial.println();
  delay(100);
}
