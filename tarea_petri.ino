
#include <VirtualWire.h>
#include <Wire.h>

#define DEBUG true
#define ENVIO_PULSO 11
#define RECIBIR_PULSO 12
#define IR_PIN 0
#define DIST_OBSTACULO 10
#define AVANZAR_TIEMPO 1000
#define GIRAR_TIEMPO 300
#define ACCION 6
//Direccion I2C de la IMU
#define MPU 0x68
 
//Ratios de conversion
#define A_R 16384.0
#define G_R 131.0
 
//Conversion de radianes a grados 180/PI
#define RAD_A_DEG 57.295779
 
//MPU-6050 da los valores en enteros de 16 bits
//Valores sin refinar
int16_t AcX, AcY, AcZ, GyX, GyY, GyZ;
 
//Angulos
float Acc[2];
float Gy[2];
float Angle[2];
float anguloI = 0;
float ultimoAngulo = -3;
float tiempoActual,tiempoAnterior;

char ID = 'V';
char GID = 'B';

int irRead;
int distancia;
long tiempo;
long randNumber;

int llanta_izq1 = 2;
int llanta_izq2 = 3;
int llanta_der1 = 4;
int llanta_der2 = 5;
 
byte comando[VW_MAX_MESSAGE_LEN];
byte comandoLenght = VW_MAX_MESSAGE_LEN;

char ORIENTACION_ACTUAL = 'N';
char X,Y;

 

void setup() {
  
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);

  Serial.begin(9600);
  
  
  pinMode(llanta_izq1,OUTPUT);
  pinMode(llanta_izq2,OUTPUT);
  pinMode(llanta_der1,OUTPUT);
  pinMode(llanta_der2,OUTPUT);
  
  vw_set_ptt_inverted(true);
  vw_setup(2000);
  vw_rx_start();
  
  tiempoActual = millis();
  
  analogWrite(ACCION,40);

}

void loop() {
solicitarPosicion();
  delay(1000);
}


void debug(char *mensaje){
  if(DEBUG){
    Serial.println(mensaje);
  }
}


//Acciones

void solicitarPosicion(){
  
  boolean continuar = false;
  
  do{
    //enviarPregunta();
    if(vw_wait_rx_max(5000)){

     
     if (vw_get_message(comando, &comandoLenght))
      {
        Serial.println("Se recibio un mensaje");
        char cmd = (char)comando[0];
        Serial.println((char*)comando);
        
        switch(cmd){
          case 'U':
            avanzar();
            break;
          case 'D':
            atras();
            delay(1000);
            detener();
            break;
          case 'L':
            giraIzquierda();
            delay(500);
            detener();
            break;
          case 'R':
            giraDerecha();
            delay(500);
            detener();
            break;
          case 'O':
            analogWrite(ACCION,40);
            break;
          case 'C':
            analogWrite(ACCION,240);
            break;
        }
        
      }else{
        Serial.println("Mensaje dañado");
      }
      
    }else{
      Serial.println("Expiro el tiempo de espera, hay que reintentar");
    }
    
  }while(!continuar);
}

void enviarPregunta(){
  Serial.println("Preguntando mi posicion");
  
  char command[14];
  memset(command,0, sizeof(command));
  // UB11110111S00
  
  // U / B <- Unico / Broadcast
  // B <- mi ID
  // 0 <- Hacia el servidor
  // 8 bits de posicion
  // S <- orientacion
  // x
  // y
  
  strcpy(command,"UB000000000000");
  
  vw_send((uint8_t *)command, strlen(command));
  vw_wait_tx();
  
}

void enviarATodos(){
  char command[14];
  memset(command,0, sizeof(command));
  // UB11110111S00
  
  // U / B <- Unico / Broadcast
  // B <- mi ID
  // 0 <- Hacia el servidor
  // 8 bits de posicion
  // S <- orientacion
  // x
  // y
  int i=0;
  for(i=0;i<comandoLenght;i++){
    command[i] = char(comando[i]);
  }
  
  command[0] = 'B';
  command[1] = ID;
  command[2] = '0';
   
  vw_send((uint8_t *)command, strlen(command));
  vw_wait_tx();
}

void enviarMensaje(char *mensaje){

  vw_send((uint8_t *)mensaje, strlen(mensaje));
  vw_wait_tx();

}

int actuar(){
 
 //[0][1][2]
 //[3][R][4]
 //[5][6][7]
  
  // 0 - libre
  // 1 - obstaculo
  // 2 - pelota
  
  
  char orientacion = (char)comando[11];
  char alrededor[8];
  memset(alrededor,0x00, sizeof(alrededor));
  
  int i;
  for(i=0;i<8;i++){
    alrededor[i] = char(comando[i+3]);
    Serial.println(alrededor[i]);
  }
  
  //Buscar si hay una pelota
  
  int accion = buscarObjecto(orientacion,'2',alrededor);

  if(accion != 0){
  
    return accion;
  
  }else{
  
        //Decidir a donde moverse
        i = 0;
       
         switch(orientacion){
         
             case 'N':
                  if(alrededor[1] == '0'){
                    avanzar();
                    break;
                  }       
      
                  if(alrededor[3] == '0'){
                    gizquierda();
                    avanzar();
                    break;
                  }     
                
                  if(alrededor[4] == '0'){
                    gderecha();
                    avanzar();
                    break;
                  }  
               
                  if(alrededor[6] == '0'){
                    girar();
                    avanzar();
                    break;
                  }     
             
               break;
               
             case 'S':
                  if(alrededor[1] == '0'){
                    girar();
                    avanzar();
                    break;
                  }       
      
                  if(alrededor[3] == '0'){
                    gderecha();
                    avanzar();
                    break;
                  }     
                
                  if(alrededor[4] == '0'){            
                    gizquierda();
                    avanzar();
                    break;
                  }  
               
                  if(alrededor[6] == '0'){
                    avanzar();
                    break;
                  }     
             
               break;
               
              case 'E':
                  if(alrededor[1] == '0'){
                    gizquierda();
                    avanzar();
                    break;
                  }       
      
                  if(alrededor[3] == '0'){
                    girar();
                    avanzar();
                    break;
                  }     
                
                  if(alrededor[4] == '0'){            
                    avanzar();
                    break;
                  }  
               
                  if(alrededor[6] == '0'){
                    gderecha();
                    avanzar();
                    break;
                  }     
             
               break;
               
               case 'O':
                  if(alrededor[1] == '0'){
                    gderecha();
                    avanzar();
                    break;
                  }       
      
                  if(alrededor[3] == '0'){
                    avanzar();
                    break;
                  }     
                
                  if(alrededor[4] == '0'){   
                    girar();
                    avanzar();
                    break;
                  }  
               
                  if(alrededor[6] == '0'){             
                    gizquierda();
                    avanzar();
                    break;
                  }     
             
               break;
         
         }
   
       return 0;
  }
  
}

int buscarObjecto(char orientacion,char objeto,char alrededor[8]){

    
     switch(orientacion){
   
       case 'N':
            if(alrededor[1] == objeto){
               return 1;
            }       

            if(alrededor[3] == objeto){
              gizquierda();
              return 1;
            }     
          
            if(alrededor[4] == objeto){
              gderecha();
              return 1;
            }  
         
            if(alrededor[6] == objeto){
              girar();
              return 1;
            }     
       
         break;
         
       case 'S':
            if(alrededor[1] == objeto){
              girar();
              return 1;
            }       

            if(alrededor[3] == objeto){
              gderecha();
              return 1;
            }     
          
            if(alrededor[4] ==  objeto){            
              gizquierda();
              return 1;
            }  
         
            if(alrededor[6] == objeto){
              return 1;
            }     
       
         break;
         
        case 'E':
            if(alrededor[1] == objeto){
              gizquierda();
                return 1;
            }       

            if(alrededor[3] == objeto){
              girar();
              return 1;
            }     
          
            if(alrededor[4] == objeto){            
              return 1;
            }  
         
            if(alrededor[6] == objeto){
              gderecha();
              return 1;
            }     
       
         break;
         
         case 'O':
            if(alrededor[1] == objeto){
              gderecha();
              return 1;
            }       

            if(alrededor[3] == objeto){
              return 1;
            }     
          
            if(alrededor[4] == objeto){   
              girar();
              return 1;
            }  
         
            if(alrededor[6] == objeto){             
              gizquierda();
              return 1;
            }     
       
         break;
   
   }
   
   if(alrededor[0] == objeto || alrededor[2] == objeto || alrededor[5] == objeto || alrededor[7] == objeto){
     debug("El objetivo esta en la esquina, necesito ayuda");
     return 2;
   }
   
   return 0;

}

int planear(int accion){
  
  if(accion == 1){
    
    debug("Intentando agarrar la pelota");
    detener();
    delay(1000);
    
    /*
    * Intenta acercarse a la pelota
    */
    
    
        int timer = 0;
        int timeCount = AVANZAR_TIEMPO/20;
        boolean pelotaAtrapada = false;
        
        do{
          
          irRead=analogRead(IR_PIN);
          distancia =(6762/(irRead-3))-4;
          Serial.println(distancia);
          
          if(distancia < DIST_OBSTACULO){
              analogWrite(ACCION,240);
              pelotaAtrapada = true; //Quiza la tiene
              break;
          }
          
          adelante();
          timer += timeCount;
          delay(timeCount);
        }while(timer < AVANZAR_TIEMPO);
        
        detener();
    
    /*
    *
    */
    
    
      if(pelotaAtrapada){
        debug("Creo que tengo la pelota");
        return 1;
      }else{
        debug("Fuck... se me fue");
        return 0;
      }
      
  }else if(accion == 2){
    debug("La pelota esta en una esquina, necesito ayuda");
    
    //Enviar mensaje a todos
    return 2;
  }
  
}

void avanzar(){
  //adelante();
  //delay(AVANZAR_TIEMPO);
  //detener();
  
  int timer = 0;
  int timeCount = AVANZAR_TIEMPO/20;
  
  do{
    
    irRead=analogRead(IR_PIN);
    distancia =(6762/(irRead-3))-4;
    Serial.println(distancia);
    if(distancia < DIST_OBSTACULO){
        Serial.println("Hay un obstaculo");
        break;
    }else{
      Serial.println("Avanza");
    }
    Serial.println("sas");
    adelante();
    timer += timeCount;
    delay(timeCount);
  }while(timer < AVANZAR_TIEMPO);
  
  detener();
}

void gizquierda(){
  /*
  giraIzquierda();
  delay(GIRAR_TIEMPO);
  detener();*/
  giraIzquierda();
  giro(1);
  detener();  
}

void gderecha(){
  /*
  giraDerecha();
  delay(GIRAR_TIEMPO);
  detener();*/
  giraDerecha();
  giro(0);
  detener();
}

void girar(){
  /*
  giraDerecha();
  delay(GIRAR_TIEMPO*2);
  detener();
  */
  giraDerecha();
  giro(0);
  giraDerecha();
  giro(0);
  detener();
}


//Leer el giroscopio

void giro(int g){
  
  //http://www.invensense.com/products/motion-tracking/6-axis/mpu-6050/
  
  do{
        
   Wire.beginTransmission(MPU);
   Wire.write(0x3B); //Pedir el registro 0x3B - corresponde al AcX
   Wire.endTransmission(false);
   Wire.requestFrom(MPU,6,true); //A partir del 0x3B, se piden 6 registros
   AcX=Wire.read()<<8|Wire.read(); //Cada valor ocupa 2 registros
   AcY=Wire.read()<<8|Wire.read();
   AcZ=Wire.read()<<8|Wire.read();
 
    //Se calculan los angulos Y, X respectivamente.
   Acc[1] = atan(-1*(AcX/A_R)/sqrt(pow((AcY/A_R),2) + pow((AcZ/A_R),2)))*RAD_TO_DEG;
   Acc[0] = atan((AcY/A_R)/sqrt(pow((AcX/A_R),2) + pow((AcZ/A_R),2)))*RAD_TO_DEG;
 
   //Leer los valores del Giroscopio
   Wire.beginTransmission(MPU);
   Wire.write(0x43);
   Wire.endTransmission(false);
   Wire.requestFrom(MPU,4,true); //A diferencia del Acelerometro, solo se piden 4 registros
   GyX=Wire.read()<<8|Wire.read();
   GyY=Wire.read()<<8|Wire.read();
 
   //Calculo del angulo del Giroscopio
   Gy[0] = (GyX/G_R); //+- 250
   Gy[1] = (GyY/G_R);
 
 //  Serial.print("Angle AGX: "); Serial.print(Gy[0]); Serial.print("\n");
    
   if(Gy[0] < -2 && Gy[0] > -3){
     Gy[0] = Gy[0] + 2.5;
   }
   
   Serial.print("Angle GX: "); Serial.print(Gy[0]); Serial.print("\n");
   //Serial.print("Angle GY: "); Serial.print(Gy[1]); Serial.print("\n------------\n");

   
   if(Gy[0] > -1 && Gy[0] < 1){
     Gy[0] = 0;
   }else{
     anguloI = anguloI + (Gy[0]*0.020);
   }
   
   
   
  // Serial.println(anguloI);
   
   if(g == 0){
     if(anguloI >= ultimoAngulo + 50){
       detener();
       ultimoAngulo = anguloI;
       break;
     }
   }else if(g == 1){
     if(anguloI <= ultimoAngulo - 50){
       detener();
       ultimoAngulo = anguloI;
       break;
     }
   }
   
   
   delay(20);
  }while(true);
  
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
  digitalWrite(llanta_izq2,HIGH);
  digitalWrite(llanta_der1,HIGH);
  digitalWrite(llanta_der2,LOW);
}

void giraDerecha(){
  digitalWrite(llanta_izq1,HIGH);
  digitalWrite(llanta_izq2,LOW);
  digitalWrite(llanta_der1,LOW);
  digitalWrite(llanta_der2,HIGH);
}
