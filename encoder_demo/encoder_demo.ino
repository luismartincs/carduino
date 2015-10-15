
int llanta_izq1 = 4;
int llanta_izq2 = 5;
int llanta_der1 = 6;
int llanta_der2 = 7;
volatile int countL = 0;
volatile int countD = 0;
volatile int vueltaL = 1;
volatile int vueltaD = 1;
volatile int vaAdelante = 0;
void setup() {
  Serial.begin(9600);
  pinMode(2,INPUT);
  pinMode(3,INPUT); 
  pinMode(13,OUTPUT);
  pinMode(llanta_izq1,OUTPUT);
  pinMode(llanta_izq2,OUTPUT);
  pinMode(llanta_der1,OUTPUT);
  pinMode(llanta_der2,OUTPUT);
  attachInterrupt(0, encoderIzquierdo, CHANGE);
  attachInterrupt(1, encoderDerecho, CHANGE);
  digitalWrite(13, LOW); 
}

void loop() {
  /*
   int estadoSensor = digitalRead(2);
   Serial.println(estadoSensor);
   if (estadoSensor == HIGH) {     
     digitalWrite(13, HIGH);  
   }else {
     digitalWrite(13, LOW); 
   }
   delay(10);*/
   

   reset();
   vaAdelante = 1;
   adelante();
   while(vueltaL == 1 && vueltaD == 1);
   //Serial.println(count);
   detener();
   delay(1000);

 
   reset();
   giraDerecha();
   while(vueltaD == 1);
   detener();
   delay(1000);
   
   
   reset();
   giraIzquierda();
   while(vueltaL == 1);
   detener();
   delay(1000);
    
   delay(5000);
}

void reset(){
   vueltaL = 1;
   vueltaD = 1;
   countL = 0;
   countD = 0;
   vaAdelante = 0;
}

void encoderDerecho(){
  if(digitalRead(3) == HIGH && vaAdelante == 1){
    countD++;
  }
  Serial.println(countD);
  if(countD == 20){
    vueltaD = 0;
  }
}

void encoderIzquierdo(){
  if(digitalRead(2) == HIGH && vaAdelante == 1){
    countL++;
  }
  Serial.println(countL);
  if(countL == 20){
    vueltaD = 0;
  }
}

//Movimiento de las llantas

void detener(){
  digitalWrite(llanta_izq1,LOW);
  digitalWrite(llanta_izq2,LOW);
  digitalWrite(llanta_der1,LOW);
  digitalWrite(llanta_der2,LOW);
}

void adelante(){
  
  digitalWrite(llanta_izq1,HIGH);
  digitalWrite(llanta_izq2,LOW);
  digitalWrite(llanta_der1,HIGH);
  digitalWrite(llanta_der2,LOW);
  
}

void atras(){
  digitalWrite(llanta_izq1,LOW);
  digitalWrite(llanta_izq2,HIGH);
  digitalWrite(llanta_der1,LOW);
  digitalWrite(llanta_der2,HIGH);
}

void giraIzquierda(){
  digitalWrite(llanta_izq1,LOW);
  digitalWrite(llanta_izq2,LOW);
  digitalWrite(llanta_der1,HIGH);
  digitalWrite(llanta_der2,LOW);
}

void giraDerecha(){
  digitalWrite(llanta_izq1,HIGH);
  digitalWrite(llanta_izq2,LOW);
  digitalWrite(llanta_der1,LOW);
  digitalWrite(llanta_der2,LOW);
}
