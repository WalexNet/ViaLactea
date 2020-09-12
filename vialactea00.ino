/*
 * Título del boceto

Describe lo que hace en términos sencillos. Consulte los componentes
unido a los distintos pines.

El circuito:
* enumere los componentes adjuntos a cada entrada
* enumere los componentes adjuntos a cada salida

Día de creación mes año
Por el nombre del autor
Año modificado día mes
Por el nombre del autor

http: //url/of/online/tutorial.cc
 * 
 */

/***** Librerias externas *****/
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>
/***** Librerias  Propias *****/
#include <InterPin.h>

/*** Archivos  del Proyecto ***/

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
int      aaaa             = 2020;   


// Variable Año
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

// Configuramos reles prueba **************************************************************
InterPin electroValvulas34(K3, K4, tpes, tpes);
//InterPin electroValvulas34(6, 7, tpes, tpes);


void setup() { 
  Serial.begin(9600); // Inicializamos el puerto serie
  setup_inicio();     // Inicializamos todo lo necesario para empezar
  startReloj();       // Inicializamops el Reloj
  tOff();             // Arancamos en estado S_OFF
} // Fin setup


/* Máquina de estados - Automata */
void loop() {
  if (manual || automatico) electroValvulas34.Update();
  else electroValvulas34.Off();
  
  switch (estado) {
    case S_OFF:        /*** INICIO ESTADO S_OFF ***/
      Serial.print("Estado S_OFF: ");
      Serial.println(S_OFF);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de OFF a MENU
        tMenu();
        break; // Fin estado Menu
      }
      break;      // *** FIN ESTADO S_OFF ****
      // *************************************

    case S_MENU:       /*** INICIO ESTADO S_MENU ***/
      Serial.print("Estado S_MENU: ");
      Serial.println(S_MENU);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de MENU a AUTOMATICO
        // Pasamos al estado AUTOMATICO
        tAutomatico();
        break; // Fin estado automatico
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de MENU a OFF
        // Pasamos a Estado OFF
        tOff();
        break; // Fin estado OFF
      }
      break;      // *** FIN ESTADO S_MENU ****
      // **************************************

    case S_AUTO:       /*** INICIO ESTADO S_AUTO ***/
      Serial.print("Estado S_AUTO: ");
      Serial.println(S_AUTO);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de AUTOMATICO a MANUAL
        tManual();
        break; // Fin estado MANUAL
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de AUTOMATICO a MENU
        tMenu();
        break; // Fin estado MENU
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" de AUTOMATICO a SEL
        tSetAutomatico();
        break; // Fin Accion
      }
      break;      // *** FIN ESTADO S_AUTO ****
      // **************************************

    case S_MANUAL:       /*** INICIO ESTADO S_MANUAL ***/
      Serial.print("Estado S_MANUAL: ");
      Serial.println(S_MANUAL);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de MANUAL a INFO
        tInfo();
        break; // Fin estado INFO
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de MANUAL a MENU
        tMenu();
        break; // Fin menu
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" de MANUAL a SEL
        tSetManual();
        break; // Fin Seleccion
      }
      break;      // *** FIN ESTADO S_MANUAL ***
      // ***************************************

    case S_INFO:       /*** INICIO ESTADO S_INFO ***/
      Serial.print("Estado S_INFO: ");
      Serial.println(S_INFO);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de INFO a CONFIG
        tConfig();
        break;
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de INFO a MENU
        tMenu();
        break;
      }
      break;      // *** FIN ESTADO S_INFO ****
      // **************************************

    case S_CONFIG:     /*** INICIO ESTADO S_CONFIG ***/
      Serial.print("Estado S_CONFIG: ");
      Serial.println(S_CONFIG);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de CONFIG a AUTO
        tAutomatico();
        break;
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de CONFIG a MENU
        tMenu();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" de CONFIG a BOX ******ANO
        tBox();
        break;
      }
      break;      // *** FIN ESTADO S_CONFIG ****
      // ****************************************


      /*  **********************************************
       *  ********** ESTADOS DE CONFIGURACION **********
       *  **********************************************
       */
    case S_BOX:     /*** INICIO ESTADO S_BOX ***/  
      Serial.print("Estado S_BOX: "); 
      Serial.println(S_BOX);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de BOX a TPES ******ANO
        tTpes();
        break;    
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de BOX a CONFIG
        tConfig();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos BOX
        tIncBox();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos BOX
        tDecBox();
        break;
      }        
      break;      // *** FIN ESTADO S_BOX  ****
      // **************************************

    case S_TPES:     /*** INICIO ESTADO S_TPES ***/ 
      Serial.print("Estado S_TPES: ");
      Serial.println(S_TPES);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de TPES a DT
        tDt();
        break;    
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de TPES a CONFIG
        tConfig();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos TPES
        tIncTpes();
        actualizaTpes();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos TPES
        tDecTpes();
        actualizaTpes();
        break;
      }        
      break;      // *** FIN ESTADO S_TPES ****
      // **************************************      

    case S_DT:     /*** INICIO ESTADO S_DT ***/
      Serial.print("Estado S_DT: ");
      Serial.println(S_DT);
      
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de DT a BOX
        tBox();
        break;
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de DT a CONFIG
        tConfig();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" de DT a ANO
        tAno();
        break;
      }
      break;      // *** FIN ESTADO S_DT ****
      // ****************************************

      /*
       *  ***************************************
       *  ******* Configuración del Reloj *******
       *  ***************************************
       */

    case S_ANO:     /*** INICIO ESTADO S_ANO ***/
      Serial.print("Estado S_ANO: ");
      Serial.println(S_ANO);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de ANO a MES
        tMes();
        break;
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de ANO a CHECK
        tCheck();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos AÑO
        tIncAno();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos AÑO
        tDecAno();
        break;
      }
      break;      // *** FIN ESTADO S_ANO  ****
      // **************************************

    case S_MES:     /*** INICIO ESTADO S_MES ***/  
      Serial.print("Estado S_MES: "); 
      Serial.println(S_MES);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de MES a DIA
        tDia();
        break;    
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de MES a CHECK
        tCheck();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos MES
        tIncMes();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos MES
        tDecMes();
        break;
      }     
      break;      // *** FIN ESTADO S_MES  ****
      // **************************************
      
    case S_DIA:     /*** INICIO ESTADO S_DIA ***/
      Serial.print("Estado S_DIA: ");
      Serial.println(S_DIA);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de DIA a HORA
        tHora();
        break;    
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de DIA a CHECK
        tCheck();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos DIA
        tIncDia();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos DIA
        tDecDia();
        break;
      }      
      break;        //*** FIN ESTADO S_DIA ****
      // **************************************

    case S_HORA:     /*** INICIO ESTADO S_HORA ***/  
      Serial.print("Estado S_HORA: "); 
      Serial.println(S_HORA);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de HORA a MIN
        tMin();
        break;    
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de HORA a CHECK
        tCheck();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos HORA
        tIncHora();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos HORA
        tDecHora();
        break;
      }        
      break;      // *** FIN ESTADO S_HORA ****
      // **************************************

    case S_MIN:     /*** INICIO ESTADO S_MIN ***/  
      Serial.print("Estado S_MIN: "); 
      Serial.println(S_MIN);
      if (presionaBoton(BTN_MENU)) { // Transición BTN_MENU "B2" de MIN a ANO
        tAno();
        break;    
      }
      if (presionaBoton(BTN_EXIT)) { // Transición BTN_EXIT "B1" de MIN a CHECK
        tCheck();
        break;
      }
      if (presionaBoton(BTN_SEL))  { // Transición BTN_SEL  "B3" incrementamos MIN
        tIncMin();
        break;
      }
      if (presionaBoton(BTN_SET))  { // Transición BTN_SET  "B4" decrementamos MIN
        tDecMin();
        break;
      }        
      break;      // *** FIN ESTADO S_MIN  ****
      // **************************************

      // Confirma y actualiza el RTC
    case S_CHECK:     /*** INICIO ESTADO S_CHECK ***/ 
      Serial.print("Estado S_CHECK: ");
      Serial.println(S_CHECK);
      if (presionaBoton(BTN_EXIT))  { // Transición BTN_EXIT  "B1" CHECK a DT
        tCheckDt();
        break;
      }
      if (presionaBoton(BTN_SEL))   { // Transición BTN_SEL  "B3" CONFIRMA DESCONFIRMA
        printCheck(!confirma);
        break;
      }        
      break;      // *** FIN ESTADO S_MIN  ****
      // **************************************
      
  } // Fin switch
} // Fin loop()
