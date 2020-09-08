/** Librerias externas **/
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

/************************** Constantes y Globales *****************************/

// Constantes y caracteres especiales
byte desmarcado[8] = { B11111, B10001, B10001, B10001, B10001, B10001, B10001, B11111 };
byte marcado[8]    = { B00000, B00001, B00011, B10110, B11100, B01000, B00000, B00000 };


// Nota: Los siguientes "DEFINE" son únicamente para
// mejorar la lectura del código al momento de codificar.
#define BTN_EXIT  0         // B1
#define BTN_MENU  1         // B2
#define BTN_SEL   2         // B3
#define BTN_SET   3         // B4

// Estado de nuestro autómata
#define S_OFF     0         // Estado OFF (Stand By), inicio
#define S_MENU    1         // Estado MENU, maquina encendida, empezamos a movernos por el menu
#define S_AUTO    2         // Estado AUTO, modo Automático
#define S_MANUAL  3         // Estado MANUAL, nodo de funcionamiento manual
#define S_INFO    4         // Estado INFO, muestra información del estado activo
#define S_CONFIG  5         // Estado CONFIG, configuracion del automata, Fecha/Hora y Nro de Box
  // Sub estados de configuracion
#define S_BOX     50        // Sub-Estado CONFIG, configura el Nro de BOX 00
#define S_TPES    51        // Sub-Estado CONFIG, configura el tiempo en milisegundos de accion de las pesoneras
#define S_DT      52        // Sub-Estado CONFIG, configura la fecha y la hora

#define S_ANO     520       // Sub-Estado CONFIG, configura el Año        0000
#define S_MES     521       // Sub-Estado CONFIG, configura el Mes        00
#define S_DIA     522       // Sub-Estado CONFIG, configura el Día        00
#define S_HORA    523       // Sub-Estado CONFIG, configura la Hora       00
#define S_MIN     524       // Sub-Estado CONFIG, configura los Minutos   00

#define S_CHECK   600       // Sub-Estado para confirmar el cambio de la Fecha y Hora


// Pines a Usar en el PIC
#define LED_OFF   6         // OFF Stand By (Led Rojo)
#define LED_ON    7         // ON Activo (Led Verde)
#define K1        11        // Rele K1
#define K2        10        // Rele K2
#define K3        9         // Rele K3
#define K4        8         // Rele K4
uint8_t button[]={5,4,3,2}; // Este arreglo contiene los pines de los botones B1, B2, B3, B4 respectivamente

/************** VARIABLES **************/
uint16_t estado           = S_OFF;  // Variable estado actual
int      vaca             = 1;      // Identificador de la vaca
double   lit              = 0;      // Litros de leche ordeñados
boolean  automatico       = false;  // Variable global del estado de automatico
boolean  manual           = false;  // Variable global del estado de manual
uint8_t  button_estate[4];          // Este arreglo contiene el último estado conocido de cada línea
unsigned long tempo       = 0;      // Tiempo del ordeñe
String   tiempo;
int      aaaa             = 2020;      // Variable Año
int      mm               = 1;      // Variable Mes        
int      dd               = 1;      // Variable Día
int      hh               = 0;      // Variable Horas
int      ii               = 0;      // Variable Minutos
int      box              = 0;      // El Nº de BOX donde ira el micro
int      tpes             = 500;    // Tiempo en Milis que dura el activado y desactivado de las pesoneras
boolean  confirma         = false;  // Variable de confirmacion de cambios
DateTime ahora;                     // Variable Dia y tiempo actual

// ******************* Helpers / Ayudantes **********************
// Creamos el LCD y Reloj
LiquidCrystal_I2C lcd(0x27, 16, 2); // Inicia el LCD en la dirección 0x27, con 16 caracteres y 2 líneas
RTC_DS3231 rtc;                     // Creamos reloj


void setup() {  
  Serial.begin(9600);             // Inicializamos el puerto serie
  setup_inicio();
  startReloj();
  printOff();
} // Fin setup


/* Máquina de estados - Automata */
void loop() {
  switch (estado) {
    case S_OFF:        /*** INICIO ESTADO S_OFF ***/
      Serial.print("Estado S_OFF: ");
      Serial.println(S_OFF);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de OFF a MENU
        // Pasamos al estado menu
        estado = S_MENU;
        digitalWrite(LED_OFF, LOW);
        digitalWrite(LED_ON, HIGH);        
        printMenu();
        break; // Fin estado Menu
      }
      break;      // *** FIN ESTADO S_OFF ****
      // *************************************

    case S_MENU:       /*** INICIO ESTADO S_MENU ***/
      Serial.print("Estado S_MENU: ");
      Serial.println(S_MENU);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de MENU a AUTOMATICO
        // Pasamos al estado AUTOMATICO
        estado = S_AUTO;
        printAutomatico(automatico);
        break; // Fin estado automatico
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de MENU a OFF
        // Pasamos a Estado OFF
        estado = S_OFF;
        digitalWrite(LED_OFF, HIGH);
        digitalWrite(LED_ON,   LOW);
        printOff();
        break; // Fin estado OFF
      }
      break;      // *** FIN ESTADO S_MENU ****
      // **************************************

    case S_AUTO:       /*** INICIO ESTADO S_AUTO ***/
      Serial.print("Estado S_AUTO: ");
      Serial.println(S_AUTO);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de AUTOMATICO a MANUAL
        // Pasamos al estado MANUAL
        estado = S_MANUAL;
        printManual(manual);
        break; // Fin estado MANUAL
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de AUTOMATICO a MENU
        // Pasamos a estado MENU
        estado = S_MENU;
        printMenu();
        break; // Fin estado MENU
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" de AUTOMATICO a SEL
        // Seguimos en AUTOMATICO solo seleccionamos la accion
        estado = S_AUTO;
        if (!manual) printAutomatico(!automatico); // Si esta manual no podemos poner automatico
        else printError("Modo Manu ACTIVO");
        break; // Fin Accion
      }
      break;      // *** FIN ESTADO S_AUTO ****
      // **************************************

    case S_MANUAL:       /*** INICIO ESTADO S_MANUAL ***/
      Serial.print("Estado S_MANUAL: ");
      Serial.println(S_MANUAL);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de MANUAL a INFO
        // Pasamos a estado INFO
        estado = S_INFO;
        printInfo("T: 00:00", "   00.0L");
        break; // Fin estado INFO
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de MANUAL a MENU
        // Pasamos a estado MENU
        estado = S_MENU;
        printMenu();
        break; // Fin menu
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" de MANUAL a SEL
        // Estado MANUAL Seleccionamos
        estado = S_MANUAL;
        if (!automatico) printManual(!manual);     // Si esta automatico no podemos poner manual
        else printError("Modo auto ACTIVO");
        break; // Fin Seleccion
      }
      break;      // *** FIN ESTADO S_MANUAL ***
      // ***************************************

    case S_INFO:       /*** INICIO ESTADO S_INFO ***/
      Serial.print("Estado S_INFO: ");
      Serial.println(S_INFO);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de INFO a CONFIG
        estado = S_CONFIG;
        printConfig();
        break;
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de INFO a MENU
        estado = S_MENU;
        printMenu();
        break;
      }
      break;      // *** FIN ESTADO S_INFO ****
      // **************************************

    case S_CONFIG:     /*** INICIO ESTADO S_CONFIG ***/
      Serial.print("Estado S_CONFIG: ");
      Serial.println(S_CONFIG);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de CONFIG a AUTO
        estado = S_AUTO;
        printAutomatico(automatico);
        break;
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de CONFIG a MENU
        estado = S_MENU;
        printMenu();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" de CONFIG a BOX ******ANO
        //estado = S_ANO;
        //printSetFechaHora;
        estado = S_BOX;
        printSetBox();
        break;
      }
      break;      // *** FIN ESTADO S_CONFIG ****
      // ****************************************

      // Añadir las transiciones para los estados S_BOX, S_TPES, S_DT
      
    case S_BOX:     /*** INICIO ESTADO S_BOX ***/  
      Serial.print("Estado S_BOX: "); 
      Serial.println(S_BOX);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de BOX a TPES ******ANO
        estado = S_TPES;
        printSetTiempoPes();
        break;    
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de BOX a CONFIG
        estado = S_CONFIG;
        printConfig();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos BOX
        box++;
        printSetBox();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos BOX
        if (box>0) {
          box--;
        } else {
          box = 0;
        }
        printSetBox();
        break;
      }        
      break;      // *** FIN ESTADO S_BOX  ****
      // **************************************

    case S_TPES:     /*** INICIO ESTADO S_TPES ***/ 
      Serial.print("Estado S_TPES: ");
      Serial.println(S_TPES);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de TPES a DT
        estado = S_DT;
        printDT();
        break;    
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de TPES a CONFIG
        estado = S_CONFIG;
        printConfig();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos TPES
        tpes += 50;
        printSetTiempoPes();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos TPES
        if (tpes>0) {
          tpes -= 50;
        } else {
          tpes = 0;
        }
        printSetTiempoPes();
        break;
      }        
      break;      // *** FIN ESTADO S_TPES ****
      // **************************************      

    case S_DT:     /*** INICIO ESTADO S_DT ***/
      Serial.print("Estado S_DT: ");
      Serial.println(S_DT);
      Serial.print("La Fecha y Hora es: ");
      ahora = rtc.now();
      Serial.print(ahora.year());
      Serial.print("/");
      Serial.print(ahora.month());
      Serial.print("/");
      Serial.print(ahora.day());
      Serial.print(" ");
      Serial.print(ahora.hour());
      Serial.print(":");
      Serial.println(ahora.minute());
      
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de DT a BOX
        estado = S_BOX;
        printSetBox();
        break;
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de DT a CONFIG
        estado = S_CONFIG;
        printConfig();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" de DT a ANO
        Serial.println("PRESIONAMOS B3 SEL");
        estado = S_ANO;
        Serial.print("ESTADO = S_ANO = ");
        Serial.println(estado);
   
        Serial.println(S_ANO);
        printSetFechaHora();
        break;
      }
      break;      // *** FIN ESTADO S_DT ****
      // ****************************************

    case S_ANO:     /*** INICIO ESTADO S_ANO ***/
      Serial.print("Estado S_ANO: ");
      Serial.println(S_ANO);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de ANO a MES
        estado = S_MES;
        printSetFechaHora();
        break;
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de ANO a CHECK
        estado = S_CHECK;
        printCheck(confirma);
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos AÑO
        aaaa++;
        printSetFechaHora();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos AÑO
        aaaa--;
        printSetFechaHora();
        break;
      }
      break;      // *** FIN ESTADO S_ANO  ****
      // **************************************

    case S_MES:     /*** INICIO ESTADO S_MES ***/  
      Serial.print("Estado S_MES: "); 
      Serial.println(S_MES);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de MES a DIA
        estado = S_DIA;
        printSetFechaHora();
        break;    
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de MES a CHECK
        estado = S_CHECK;
        printCheck(confirma);
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos MES
        if (mm<12) {
          mm++;
        } else {
          mm = 1;
        }
        printSetFechaHora();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos MES
        if (mm>1) {
          mm--;
        } else {
          mm = 12;
        }
        printSetFechaHora();
        break;
      }     
      break;      // *** FIN ESTADO S_MES  ****
      // **************************************
      
    case S_DIA:     /*** INICIO ESTADO S_DIA ***/
      Serial.print("Estado S_DIA: ");
      Serial.println(S_DIA);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de DIA a HORA
        estado = S_HORA;
        printSetFechaHora();
        break;    
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de DIA a CHECK
        estado = S_CHECK;
        printCheck(confirma);
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos DIA
        if (dd<31) {
          dd++;
        } else {
          dd = 1;
        }
        printSetFechaHora();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos DIA
        if (dd>1) {
          dd--;
        } else {
          dd = 31;
        }
        printSetFechaHora();
        break;
      }      
      break;        //*** FIN ESTADO S_DIA ****
      // **************************************

    case S_HORA:     /*** INICIO ESTADO S_HORA ***/  
      Serial.print("Estado S_HORA: "); 
      Serial.println(S_HORA);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de HORA a MIN
        estado = S_MIN;
        printSetFechaHora();
        break;    
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de HORA a CHECK
        estado = S_CHECK;
        printCheck(confirma);
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos HORA
        if (hh<23) {
          hh++;
        } else {
          hh = 0;
        }
        printSetFechaHora();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos HORA
        if (hh>0) {
          hh--;
        } else {
          hh = 23;
        }
        printSetFechaHora();
        break;
      }        
      break;      // *** FIN ESTADO S_HORA ****
      // **************************************

    case S_MIN:     /*** INICIO ESTADO S_MIN ***/  
      Serial.print("Estado S_MIN: "); 
      Serial.println(S_MIN);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de MIN a ANO
        estado = S_ANO;
        printSetFechaHora();
        break;    
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de MIN a CHECK
        estado = S_CHECK;
        printCheck(confirma);
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos MIN
        if (ii<59) {
          ii++;
        } else {
          ii = 0;
        }
        printSetFechaHora();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos MIN
        if (ii>0) {
          ii--;
        } else {
          ii = 59;
        }
        printSetFechaHora();
        break;
      }        
      break;      // *** FIN ESTADO S_MIN  ****
      // **************************************

    case S_CHECK:     /*** INICIO ESTADO S_CHECK ***/ 
      Serial.print("Estado S_CHECK: ");
      Serial.println(S_CHECK);
      if (presionaBoton(BTN_EXIT))  { // Transición BTN_EXIT  "B1" CHECK a DT
        // Si cinfirmo los cambios, guardar los datos modificados
        if (confirma) rtc.adjust(DateTime(aaaa,mm,dd,hh,ii,0));        
        estado = S_DT;
        printDT();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" ACEPTAR CAMBIOS
        printCheck(!confirma);
        break;
      }        
      break;      // *** FIN ESTADO S_MIN  ****
      // **************************************
      
  } // Fin switch
} // Fin loop()


// Asume la existencia de un arreglo button
uint8_t presionaBoton(int btn) {
  uint8_t valor_nuevo = digitalRead(button[btn]);
  uint8_t result = (button_estate[btn] != valor_nuevo) && valor_nuevo == 1;
  button_estate[btn] = valor_nuevo;
  // Serial.println(button[btn]);
  return result;
}

void startReloj(){
  // Preparamops el Reloj
  // *****************************************************
  if (!rtc.begin()) {
    Serial.println(F("No se encuentra el RTC"));
    while (1);
  }
  
  // Si se ha perdido la corriente, fijar fecha y hora
  if (rtc.lostPower()) {
    // Fijar a fecha y hora de compilacion
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // Fijar a fecha y hora específica. En el ejemplo, 21 de Enero de 2016 a las 03:00:00
    // rtc.adjust(DateTime(2016, 1, 21, 3, 0, 0));
  }
  // Inicializamos las Variables de tiempo
  ahora = rtc.now();
  aaaa  = ahora.year();
  mm    = ahora.month();
  dd    = ahora.day();
  hh    = ahora.hour();
  ii    = ahora.minute();
  
  // Fin Configuracion Reloj
  // ******************************************************  
}

void setup_inicio(){
  // Configuramos Botones como PULL-UP para ahorrar resistencias
  pinMode(button[BTN_MENU], INPUT_PULLUP);
  pinMode(button[BTN_EXIT], INPUT_PULLUP);
  pinMode(button[BTN_SEL],  INPUT_PULLUP);
  pinMode(button[BTN_SET],  INPUT_PULLUP);

  // Configuramos Leds
  pinMode(LED_ON,  OUTPUT);
  pinMode(LED_OFF, OUTPUT);
  digitalWrite(LED_ON,  LOW);
  digitalWrite(LED_OFF, LOW);

  // Configuramos Reles
  // ************************************************************************************
  pinMode(K1, OUTPUT);
  pinMode(K2, OUTPUT);
  pinMode(K3, OUTPUT);
  pinMode(K4, OUTPUT);
  // Para usar el terminal NO del rele las salidas deben estar en HIGH (Lógica inversa)
  digitalWrite(K1, HIGH);
  digitalWrite(K2, HIGH);
  digitalWrite(K3, HIGH);
  digitalWrite(K4, HIGH);
  // *************************************************************************************
  
  // Se asume que el estado inicial es HIGH
  button_estate[0] = HIGH;
  button_estate[1] = HIGH;
  button_estate[2] = HIGH;
  button_estate[3] = HIGH;  

  lcd.init();                     // initialize the lcd
  lcd.createChar(0, desmarcado);  // Creamos el caracter de desmarcado
  lcd.createChar(1, marcado);     // Creamos el caracter de marcado

  // por defecto
  digitalWrite(LED_OFF, HIGH); 
  
}

/******************* Utilitarios de Despligue ****************************/
void printOff() {
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("<< VIA LACTEA >>");
  lcd.setCursor(0, 1);
  lcd.print(" OFF - STAND BY");
  delay(2000);
  lcd.noBacklight();      // Apagamos, la retroilumincion
  
  // Pruebas Reles
  digitalWrite(K2, HIGH);
}

void printMenu() {
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("[OPCIONES]");
  lcd.setCursor(0, 1);
  lcd.print("2 para cambiar");

  // Pruebas Reles
  digitalWrite(K2, LOW);
}

void printManual(boolean x) {
  manual = x;             // Variable global
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MODO MANUAL");
  lcd.setCursor(0, 1);
  lcd.print("Activado: ");
  if (manual) lcd.write(1);
  else lcd.write(0);
}

void printAutomatico(boolean x) {
  automatico = x;         // Variable global
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("MODO AUTOMATICO");
  lcd.setCursor(0, 1);
  lcd.print("Activado: ");
  if (automatico) lcd.write(1);
  else lcd.write(0);
}

void printInfo(String tiempo, String lit) {
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("[INFO] Vaca: 001");
  lcd.setCursor(0, 1);
  lcd.print(tiempo+lit);
}

void printError(String msg) {
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("[ERROR]");
  lcd.setCursor(0, 1);
  lcd.print(msg);
}

void printConfig() {
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("[CONFIGURACION]");
  lcd.setCursor(0, 1);
  lcd.print("3 para opciones");
}

void printSetFechaHora() {
  char fechaHora[17];
  String seteo = "Nada que setear";
  Serial.print("ENTRA A printSetFechaHora(). VAR ESTADO = " );
  Serial.println(estado);
  switch (estado){
    case S_ANO:  seteo = "Set Ano:";      break;
    case S_MES:  seteo = "Set Mes:";      break;
    case S_DIA:  seteo = "Set Dia:";      break;
    case S_HORA: seteo = "Set Hora:";     break;
    case S_MIN:  seteo = "Set Minutos:";  break;
  }
  
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(seteo);
  lcd.setCursor(0, 1);
  sprintf(fechaHora, "%04d-%02d-%02d %02d:%02d", aaaa, mm, dd, hh, ii); 
  lcd.print(fechaHora);
}

void printSetBox() {
  char linea1[16];
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Set BOX:");
  lcd.setCursor(0, 1);
  sprintf(linea1, "Box Nro: %02d", box);
  lcd.print(linea1);
}

void printSetTiempoPes() {
  char linea1[16];
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tiempo Pesonera:");
  lcd.setCursor(0, 1);
  sprintf(linea1, "Milis: %04d", tpes);
  lcd.print(linea1);  
}

void printDT(){
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FECHA Y HORA");
  lcd.setCursor(0, 1);
  lcd.print("3 para entrar");
}

void printCheck(boolean x) {
  confirma = x;           // Variable global
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("HAY CAMBIOS");
  lcd.setCursor(0, 1);
  lcd.print("Confirma ? "); 
  if (confirma) lcd.write(1); 
  else lcd.write(0); 
}