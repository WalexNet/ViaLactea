/*
 * Funciones de Control e impresión en el LCD
 */

/************************** Despliegue en Pantalla ****************************/
void printOff() {
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("<< VIA LACTEA >>");
  lcd.setCursor(0, 1);
  lcd.print(" OFF - STAND BY");
  delay(2000);
  lcd.noBacklight();      // Apagamos, la retroilumincion
  
}

void printMenu() {
  lcd.backlight();        // Encendemos, la retroilumincion
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("[OPCIONES]");
  lcd.setCursor(0, 1);
  lcd.print("2 para cambiar");
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
