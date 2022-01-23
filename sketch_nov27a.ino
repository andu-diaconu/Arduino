#include <AFMotor.h>
#include <Ultrasonic.h>

#define stanga 0
#define dreapta 1
#define inainte 2
#define spate 3

AF_DCMotor stanga_spate(1);
AF_DCMotor dreapta_spate(2);
AF_DCMotor dreapta_fata(3);
AF_DCMotor stanga_fata(4);

Ultrasonic senzor_fata(34,35);
Ultrasonic senzor_spate(40,41);
Ultrasonic senzor_lateral_fata(36,37);
Ultrasonic senzor_lateral_spate(38,39);

byte starea_parcarii = 0;

int lungime_masina = 28;
int latime_masina = 15;

int IR_pin = 21;
volatile int val_logica = 0;
int rotiri = 0;
int logica_curenta = 0;
int logica_precedenta = 0;

void opreste(){
  stanga_spate.run(RELEASE);
  dreapta_spate.run(RELEASE);
  dreapta_fata.run(RELEASE);
  stanga_fata.run(RELEASE);
}

void deplasare(byte directie, byte viteza){
  
  if (directie == 0){
    stanga_fata.setSpeed(viteza);
    stanga_spate.setSpeed(viteza);
    dreapta_fata.setSpeed(viteza);
    dreapta_spate.setSpeed(viteza);
    
    stanga_fata.run(BACKWARD);
    stanga_spate.run(BACKWARD);
    dreapta_fata.run(FORWARD);
    dreapta_spate.run(FORWARD);
  }
  
  if (directie == 1){
    stanga_fata.setSpeed(viteza);
    stanga_spate.setSpeed(viteza);
    dreapta_fata.setSpeed(viteza);
    dreapta_spate.setSpeed(viteza);
    
    stanga_fata.run(FORWARD);
    stanga_spate.run(FORWARD);
    dreapta_fata.run(BACKWARD);
    dreapta_spate.run(BACKWARD);
    }
   
  if (directie == 2){
    stanga_fata.setSpeed(viteza);
    stanga_spate.setSpeed(viteza);
    dreapta_fata.setSpeed(viteza);
    dreapta_spate.setSpeed(viteza);
    
    stanga_fata.run(FORWARD);
    stanga_spate.run(FORWARD);
    dreapta_fata.run(FORWARD);
    dreapta_spate.run(FORWARD);
  }
    
  if (directie == 3){
    stanga_fata.setSpeed(viteza);
    stanga_spate.setSpeed(viteza);
    dreapta_fata.setSpeed(viteza);
    dreapta_spate.setSpeed(viteza);
    
    stanga_fata.run(BACKWARD);
    stanga_spate.run(BACKWARD);
    dreapta_fata.run(BACKWARD);
    dreapta_spate.run(BACKWARD);
  }
}

bool updateStareParcare(){

  //Masina merge inainte cu viteza setata intrerupta fiind doar de if-uri
  //Daca se afla in vreo stare care nu e trata in if-uri (de ex doar primul senzor detecteaza loc de parcare) masina foloseste ultima comanda (de obicei deplaseaza(inainte,77))
  
  long distanta_lateral_spate = senzor_lateral_spate.Ranging(CM);
  long distanta_lateral_fata = senzor_lateral_fata.Ranging(CM);

  //suntem cu totul in dreptul unui obstacol
  if(distanta_lateral_fata < latime_masina && distanta_lateral_spate < latime_masina && starea_parcarii == 0 ){
    //am inceput catuarea dar inca nu am gasit nimic liber
    Serial.println("INCEPE PARCAREA");
    //deplasare(inainte, 77);
    deplasare(inainte, 120);
    starea_parcarii = 1;
  }

  //detectam un loc liber de parcare
  if(distanta_lateral_fata > latime_masina && distanta_lateral_spate > latime_masina && starea_parcarii == 1){
    //mergem inainte - trebuie sa trecem de locul liber
    Serial.println("S-A GASIT LOC DE PARCARE");
    opreste();
    delay(500);
    deplasare(inainte,120);
    starea_parcarii = 2;
  }

  // senzorul de pe lateral spate detecteaza din nou loc ocupat
  if(distanta_lateral_spate<latime_masina && distanta_lateral_fata < latime_masina && starea_parcarii == 2){
    //ne oprim
    opreste();
    delay(1000);// pentru 1 secunda
    starea_parcarii = 3;
  }

  return starea_parcarii;
}

void numaraRotiri(int count){

  for (int i = 0 ;i<= count; i+1){
    val_logica = digitalRead(IR_pin);
    if (val_logica == LOW)
      logica_curenta = 0;
    else
      logica_curenta = 1;
    
    if(logica_curenta != logica_precedenta){
      if(logica_curenta == 1){
        rotiri = rotiri + 1;
        i = i+1;
      }
      else{
        i = i ;
      }
      logica_precedenta = logica_curenta;
    }
    
    if (i == count){
      stanga_spate.run(RELEASE);
      dreapta_spate.run(RELEASE);
      dreapta_fata.run(RELEASE);
      stanga_fata.run(RELEASE);
      val_logica = 0;
       rotiri = 0;
       logica_curenta = 0;
       logica_precedenta = 0; 
    }
  }
}

void park(){
  
  updateStareParcare();

  //incepe parcarea laterala
  if(starea_parcarii == 3){
    
      deplasare(spate,98);     
      numaraRotiri(24);
      Serial.println("Imediat incepe virarea");
      
      opreste();
      delay(1000);
        
      // Rotire 45 grade
      deplasare(dreapta,200);
      numaraRotiri(14);
      Serial.println("S-a virat la 45 de grade");
      
      opreste();
      delay(1000);// pentru juma de secunda
  
      starea_parcarii = 5;
    }

   if(starea_parcarii == 5){
      long distanta_spate = senzor_spate.Ranging(CM);
      deplasare(spate,150);
      if(distanta_spate>0 && distanta_spate<12){
         opreste();
         delay(500);     
         starea_parcarii = 6;
      }
    }

    if(starea_parcarii == 6){
      deplasare(stanga,200);
      numaraRotiri(20);
      opreste();
      delay(500);
      starea_parcarii = 4;
//      long dist_lateral_fata = senzor_lateral_fata.Ranging(CM);
//      long dist_lateral_spate = senzor_lateral_spate.Ranging(CM);
//      if(dist_lateral_fata == dist_lateral_spate){
//        opreste();
//        delay(500);
//        starea_parcarii = 4;
//      }
  }

  if(starea_parcarii == 4){
    deplasare(inainte, 98);
      long dist_in_fata = senzor_fata.Ranging(CM);
      if(dist_in_fata < 8){
        opreste();
        starea_parcarii = 7; //FINISH - sa nu mai intre in bucla degeaba
        Serial.println("PARCARE FINALIZATA");
      }
  }
  
}

void setup() {
Serial.begin(9600);
attachInterrupt(5, numaraRotiri, CHANGE);
pinMode (IR_pin, INPUT) ;
  opreste();
  delay(2000);
}

void loop() {
  park();
}
