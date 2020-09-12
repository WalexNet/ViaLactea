/* Funciones de Transiciones
 *  y de utilidad gral
 */

// Asume la existencia de un arreglo button
// Verificamos el si se presiono algun boton
uint8_t presionaBoton(int btn) {
  uint8_t valor_nuevo = digitalRead(button[btn]);
  uint8_t result = (button_estate[btn] != valor_nuevo) && valor_nuevo == 1;
  button_estate[btn] = valor_nuevo;
  // Serial.println(button[btn]);
  return result;
}

/***************  Configuracion de Inicio y el RTC *********************/
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

  // por defecto, estado OFF
  tOff();
  
}

/********************** Funciones de Transiciones **********************/
void tMenu(){
  // Transición a MENU
  estado = S_MENU;
  digitalWrite(LED_OFF, LOW);
  digitalWrite(LED_ON, HIGH);   
  printMenu();
}

void tAutomatico(){
  // Pasamos al estado AUTOMATICO
  estado = S_AUTO;
  printAutomatico(automatico);  
}

void tOff(){
  // Pasamos a estado OFF
  estado = S_OFF;
  manual      = false;
  automatico  = false;
  digitalWrite(LED_OFF, HIGH);
  digitalWrite(LED_ON,   LOW);
  electroValvulas34.Off();
  printOff();
}

void tManual(){
  // Pasamos al estado MANUAL
  estado = S_MANUAL;
  printManual(manual);  
}

void tSetAutomatico(){
  // Seguimos en AUTOMATICO solo seleccionamos la accion
  estado = S_AUTO;
  if (!manual) printAutomatico(!automatico); // Si esta manual no podemos poner automatico
  else printError("Modo Manu ACTIVO");  
}

void tInfo(){
  // Pasamos a estado INFO
  estado = S_INFO;
  printInfo("T: 00:00", "   00.0L");  
}

void tSetManual(){
  // Seleccionamos Estado MANUAL 
  estado = S_MANUAL;
  if (!automatico) printManual(!manual);     // Si esta automatico no podemos poner manual
  else printError("Modo auto ACTIVO");  
}

void tConfig(){
  // Pasamos a estado CONFIG
  estado = S_CONFIG;
  printConfig();
}

void tBox(){
  // Pasamos estado BOX
  estado = S_BOX;
  printSetBox();
}

void tTpes(){
  // Pasamos a TPES
  estado = S_TPES;
  printSetTiempoPes();
}

void tIncBox(){
  // Incrementa box
  box++;
  printSetBox();
}

void tDecBox(){
  // Decrementa box
  if (box>0) {
    box--;
  } else {
    box = 0;
  }
  printSetBox();  
}

void tDt(){
  // Pasa a estado DT
  estado = S_DT;
  printDT();  
}

void tIncTpes(){
  // Incrementa tpes en 50 miliseg
  tpes += 50;
  printSetTiempoPes();  
}

void tDecTpes(){
  // Decrementa tpes en 50 miliseg
  if (tpes>0) {
    tpes -= 50;
  } else {
    tpes = 0;
  }
  printSetTiempoPes();  
}

void tAno(){
  // Pasamos a ANO
  estado = S_ANO;        
  printSetFechaHora();
}

void tMes(){
  // Pasamos a MES
  estado = S_MES;
  printSetFechaHora();  
}

void tCheck(){
  // Pasamos a CHECK
  estado = S_CHECK;
  printCheck(confirma);
}

void tIncAno(){
  // Incrementa aaaa (año)
  aaaa++;
  printSetFechaHora();  
}

void tDecAno(){
  // Decrementa aaaa (año)
  aaaa--;
  printSetFechaHora();  
}

void tDia(){
  // Pasa a DIA
  estado = S_DIA;
  printSetFechaHora();  
}

void tIncMes(){
  // Incrementa el mm (mes)
  if (mm<12) {
    mm++;
  } else {
    mm = 1;
  }
  printSetFechaHora();
}

void tDecMes(){
  // Decrementa mm (mes)
  if (mm>1) {
    mm--;
  } else {
    mm = 12;
  }
  printSetFechaHora();  
}

void tHora(){
  // Pasa a HORA
  estado = S_HORA;
  printSetFechaHora();
}

void tIncDia(){
  // Incrementa dd (dia)
  if (dd<31) {
    dd++;
  } else {
    dd = 1;
  }
  printSetFechaHora();
}

void tDecDia(){
  // Decrementa dd (dia)
  if (dd>1) {
    dd--;
  } else {
    dd = 31;
  }
  printSetFechaHora();  
}

void tMin(){
  // Pasa a MIN
  estado = S_MIN;
  printSetFechaHora();
}

void tIncHora(){
  // Incrementa hh (hora)
  if (hh<23) {
    hh++;
  } else {
    hh = 0;
  }
  printSetFechaHora();
}

void tDecHora(){
  // Decrementa hh (hora)
  if (hh>0) {
    hh--;
  } else {
    hh = 23;
  }
  printSetFechaHora();
}

void tIncMin(){
  // Incrementa ii (minutos)
  if (ii<59) {
    ii++;
  } else {
    ii = 0;
  }
  printSetFechaHora();
}

void tDecMin(){
  // Decrementa ii (minutos)
  if (ii>0) {
    ii--;
  } else {
    ii = 59;
  }
  printSetFechaHora();
}

void tCheckDt(){
  // Si cinfirmo los cambios, guardar los datos modificados
  if (confirma) rtc.adjust(DateTime(aaaa,mm,dd,hh,ii,0));        
  estado = S_DT;
  printDT();
}

void actualizaTpes(){
  electroValvulas34.SetOnTime(tpes);
  electroValvulas34.SetOffTime(tpes);
}
