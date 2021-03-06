
#include <Wire.h>   // incluye libreria para interfaz I2C
#include <RTClib.h>

/////////// MODULO RTC CLOCK /////////////

bool evento_inicio = true;  // variable de control para inicio de evento con valor true
bool evento_fin = true;   // variable de control para finalizacion de evento con valor true

//Variables

//Objects
RTC_DS3231 rtc;
//////////////////////////////////////////

//////////// ENTRADAS DIGITAL ////////////
const int  switchOnePin = 6;    // digital in 5 (pin the switch one is attached to)
const int  switchTwoPin = 5;    // digital in 6 (pin the switch two is attached to)
const int  switchThreePin = 7;  // digital in 7 (pin the switch three is attached to)

int switchOneState = 0;         // current state of the switch
int lastSwitchOneState = 0;     // previous state of the switch

int switchTwoState = 0;
int lastSwitchTwoState = 0;

int switchThreeState = 0;
int lastSwitchThreeState = 0;

int estado = 0; 
int lastestado =0;
int uso = 0; 

//////////////////////////////////////////

//////////// SALIDA DIGITAL //////////////
# define RELE 4     // constante RELE con valor 3 que corresponde a pin digital 3
        // donde se encuentra conectado el modulo de rele de la electrovalvula. 
//////////////////////////////////////////

///////// SENSOR HUMEDAD SUELO //////////
#define AOUT 0 // Pin analógico "A0" para conectar la salida del sensor de humedad capacitivo
 
const int Valor_Sensor_Aire = 606; // Valor calculado con el programa de calibración con el sensor al aire //405
const int Valor_Sensor_Agua = 246; // Valor calculado con el programa de calibración con el sensor sumergido en agua //264
 
int valor_sensor = 0; // Variable que almacena el valor de salida del sensor de humedad capacitivo
int porcentaje = 0; // Variable que almacena el porcentaje de humedad relativa del terreno
//////////////////////////////////////////

        
void setup () {

 evento_inicio = true;
  
 Serial.begin(9600);    // inicializa comunicacion serie a 9600 bps
 
 pinMode(RELE, OUTPUT);   // pin 4 como salida

 pinMode(switchOnePin, INPUT); // pin 6 como entrada "Off"
 pinMode(switchTwoPin, INPUT); // pin 5 como entrada "On"
 pinMode(switchThreePin, INPUT); // pin 7 como entrada "Auto"

 pinMode(LED_BUILTIN, OUTPUT); // initialize digital pin LED_BUILTIN as an output. (LED L del Arduino UNO)

 Wire.begin();

 if (! rtc.begin()) {
 Serial.println("Módulo RTC no encontrado !");
 while (1);
 }
 //rtc.adjust(DateTime(__DATE__, __TIME__));
  
}

void loop () {

// Se presenta fecha y hora
 DateTime fecha = rtc.now();

 Serial.print(fecha.day());
 Serial.print("/");
 Serial.print(fecha.month());
 Serial.print("/");
 Serial.print(fecha.year());
 Serial.print(" ");
 Serial.print(fecha.hour()); 
 Serial.print(":"); 
 Serial.print(fecha.minute());
 Serial.print(":"); 
 Serial.println(fecha.second());

// Se presenta el porcentaje a través del monitor serie
 Serial.print("HUMEDAD: ");
 Serial.print(valor_sensor);
 Serial.print(" unidad. ");
 Serial.print(porcentaje);
 Serial.println("% HR");

 switchOneState   = digitalRead(switchOnePin);
 switchTwoState   = digitalRead(switchTwoPin);
 switchThreeState = digitalRead(switchThreePin); 

 estado = (switchOneState * 1) + (switchTwoState * 2) + ( switchThreeState * 4);
 
 valor_sensor = analogRead(AOUT); // Leemos el valor de la salida analógica del sensor capacitivo, conectada al pin analógico "A0"
 porcentaje = map(valor_sensor, Valor_Sensor_Agua, Valor_Sensor_Aire, 100, 0); // Se calcula el porcentaje de humedad relativa teniendo en cuenta los dos límites

  if(porcentaje < 0) porcentaje = 0; // Evita porcentajes negativos en la medida del sensor
  if(porcentaje > 100) porcentaje = 100; // Evita porcentajes negativos en la medida del sensor

//***OPCIONES***//

// 0 - Off
// Las 3 entradas digitales en LOW, se apaga todo.
 if ( estado != lastestado && estado == 0 ){
   evento_inicio = true; //Por si estaba en el modo 2 y se quiere volver al modo 2
   digitalWrite(RELE, LOW);        // desactiva modulo de rele con nivel bajo
   digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
   Serial.println( "Electrovalvula apagada" );     // muestra texto en monitor serie
   Serial.println( "Estado 0 - ON" ); 
   uso = 0;   
 }

// 1 - On
 if ( (lastSwitchOneState != switchOneState) && switchOneState == HIGH ){
   evento_inicio = true; //Por si estaba en el modo 2 y se quiere volver al modo 2
   digitalWrite(RELE, HIGH);       // activa modulo de rele con nivel alto
   digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
   Serial.println( "Electrovalvula encendida" );   // muestra texto en monitor serie
   Serial.println( "Estado 1 - ON" );
   uso = 0;
 } 

 
// 2 - Auto

 if ( (lastSwitchTwoState != switchTwoState) && switchTwoState == HIGH ){
   uso = 0;
   digitalWrite(RELE, LOW);        // desactiva modulo de rele con nivel bajo
   digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
   Serial.println( "Electrovalvula apagada" );     // muestra texto en monitor serie 
   Serial.println( "Estado 2 - ON - Se apaga la electrovalvula antes de empezar" );
 } 
 if ( (fecha.hour()) == 21 && (fecha.minute()) == 00 && porcentaje <= 50 && switchTwoState == HIGH ){ // si hora = 21 y minutos = 00 y humedad de suelo menor igual que 50%
    if ( evento_inicio == true ){     // si evento_inicio es verdadero
      digitalWrite(RELE, HIGH);       // activa modulo de rele con nivel alto
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      Serial.println( "Electrovalvula encendida" );   // muestra texto en monitor serie
      evento_inicio = false;        // carga valor falso en variable de control
      Serial.println( "Estado 2 - ON" );
    }             // para evitar ingresar mas de una vez
  }

 if ((((fecha.hour()) == 22 && (fecha.minute()) == 30 ) || porcentaje > 90 )&& switchTwoState == HIGH ){ // si ( hora = 22 y minutos = 30 ) o humedad del suelo mayor al 90%
    if ( evento_fin == true ){        // si evento_fin es verdadero
      digitalWrite(RELE, LOW);        // desactiva modulo de rele con nivel bajo
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      Serial.println( "Electrovalvula apagada" );     // muestra texto en monitor serie
      evento_fin = false;       // carga valor falso en variable de control
      Serial.println( "Estado 2 - OFF - Auto" );
    }             // para evitar ingresar mas de una vez
  }


// 3 - Test // Enciende cuando la humedad es menos de 50% y se apaga cuando es mayor que 90% 
 if ( (lastSwitchThreeState != switchThreeState) && porcentaje <= 50 && switchThreeState == HIGH ){
   uso = 1;
   digitalWrite(RELE, HIGH);       // activa modulo de rele con nivel alto
   digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
   Serial.println( "Electrovalvula encendida" );   // muestra texto en monitor serie
   Serial.println( "Estado 3 - ON" );
 } else if ( porcentaje > 90 && switchThreeState ==  HIGH ){ // Se apaga, si se llego a 90% de humedad
   uso = 0;
   digitalWrite(RELE, LOW);        // desactiva modulo de rele con nivel bajo
   digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
   Serial.println( "Electrovalvula apagada" );     // muestra texto en monitor serie 
   Serial.println( "Estado 3 - OFF - Auto" );
 } else if ( porcentaje < 50 && switchThreeState ==  HIGH && uso == 0){ // Se apaga, si se llego a 90% de humedad
   uso = 1;
   digitalWrite(RELE, HIGH);       // activa modulo de rele con nivel alto
   digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
   Serial.println( "Electrovalvula encendida" );   // muestra texto en monitor serie
   Serial.println( "Estado 3 - ON" );
 }
 

 if ( (fecha.hour()) == 2 && (fecha.minute()) == 0 ){  // si hora = 2 y minutos = 0 restablece valores de
    evento_inicio = true;       // variables de control en verdadero
    evento_fin = true;
  }

// previous state of the switch
 lastSwitchOneState = switchOneState;
 lastSwitchTwoState = switchTwoState;
 lastSwitchThreeState = switchThreeState;
 lastestado =  estado;
   delay(3000);           // demora de 3 segundo
   Serial.println( "**********" ); 
}
