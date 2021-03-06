/** @file     SensorInteligente.cpp 
 *  @date     8 de julio de 2020
 *  @authors  Rosa Pincay, Javier Arce
 *  @brief    Librería para detección de pesos usando velostat y para realizar mediciones de batería. 
 *            Incluye el envío de datos encriptados a Sigfox Backend.    
 */
 
#include "SensorInteligente.h"
#include "Arduino.h"


/** 
 * @brief   Crea un objeto de tipo SensorInteligente, se utiliza para realizar mediciones de batería.
 *          Es necesario declarar el constructor de la función para poder utilizar sus métodos.
 * @param   pinA1: pin analógico al que se conecta la batería.
 */
SensorInteligente::SensorInteligente(int pinA1)
{
  _pinA1 = pinA1;
}


/**
 * @brief   Crea un objeto de tipo SensorInteligente, se utiliza para medir los valores de la batería y el velostat.
 *          Es necesario declarar el constructor de la función para poder utilizar sus métodos.
 * @param   pinA0: pin analógico al que se conecta el velostat.
 * @param   pinA1: pin analógico al que se conecta la fuente de alimentación.
 */
SensorInteligente::SensorInteligente(int pinA0, int pinA1)
{
  _pinA0 = pinA0;
  _pinA1 = pinA1;
}


/**
 * @brief   Inicializa las variables públicas y privadas que son utilizadas en la librería.
 */
void SensorInteligente::inicializar()
{
  _limiteMaximoBateria = 0;
  _tiempoAnterior = 0.0;
  nivelBateriaMayor = 100;
  voltajeBateria = 0.0;
  porcentajeBateria = 0;
  voltajeSensor = 0.0;
  bateria = 0.0;
}


/**
 * @brief   Función que realiza la lectura del pin analógico A1 y convierte ese valor a un voltaje.
 * @return  voltajeBateria: valor de tipo float en el rango de [0, 5].
 */
float SensorInteligente::leerVoltajeBateria()
{
  bateria = analogRead(_pinA1);   
  delay(2);
  voltajeBateria = (((float) bateria) * 5.0) / 1023.0;
  return voltajeBateria;
}


/** 
 * @brief   Función que se debe utilizar de forma obligatoria si se requiere obtener el porcentaje de batería.
 *          Es necesario que se implemente un divisor de voltaje. 
 *          Establece el voltaje máximo de entrada que entrega la batería en el rango de [0, 1023].
 *          Debe utilizarse en la función setup() luego de inicializar las variables de la librería.
 * @param   rBajo: valor de la resistencia en Ohmios (Ω) que se encuentra conectada al polo negativo.
 * @param   rArriba: valor de la resistencia en Ohmios Ohmios (Ω) que está conectada al polo positivo.
 * @param   VIn: voltaje de la batería, puede indicar un voltaje nominal o realizar mediciones con un multímetro.
 */
void SensorInteligente::calibrarBateria(float rBajo, float rArriba, float vIn){
  float vMax = (rBajo / (rBajo + rArriba)) * vIn;
  _limiteMaximoBateria = (int)(vMax * (1023 / 5));
}


/**
 * @brief   Función que se encarga de mapear el nivel de la batería en el rango de [0, 100]
 * @return  porcentajeBateria: valor de tipo int que indica el nivel actual de carga de la batería.
 */
int SensorInteligente::leerPorcentajeBateria(){ 
  bateria = analogRead(_pinA1);    
  delay(5);
  porcentajeBateria = map((int) bateria, 0, _limiteMaximoBateria, 0, 100);        
  return porcentajeBateria;
}


/**
 * @brief   Mide el voltaje del velostat el cual se encuentra conectado al pinA0.
 * @return  voltajeSensor: valor de tipo float del voltaje del velostat en el rango [0, 5].
 */
float SensorInteligente::leerVoltajeVelostat()
{
  int voltajeAlfombra = analogRead(_pinA0);
  voltajeSensor = (((float) voltajeAlfombra) * 5.0) / 1023.0;
  return voltajeSensor;
}


/**
 * @brief   Obtiene el nivel de bateria más bajo que ha sido censado.
 *          Si existe una caída del nivel de la batería mayor al 5% el porcentaje no cambiará.
 * @param   porcentajeBatería: valor que será leído continuamente en el programa.
 * @return  nivelBateriaMayor: devuelve el porcentaje de batería mayor que ha sido leído.
 */
int SensorInteligente::compararNivelBateria(int porcentajeBateria) 
{
  if ((porcentajeBateria <= nivelBateriaMayor)) {
    nivelBateriaMayor = porcentajeBateria;
  }
  if ((porcentajeBateria > nivelBateriaMayor - 5)) {  
    nivelBateriaMayor = porcentajeBateria; 
  }
  return nivelBateriaMayor;
}


/**
 * @brief   Función que determina el envío de la batería dado cierto intervalo de tiempo.
 *          La batería solo se envía cuando el nivel es mayor a 30%.
 * @param   intervalo: tiempo de espera para realizar el envío del valor de la batería.
 * @param   porcentaje: valor entre 0 y 100 que se transforma a un byte para ser enviado.
 */
void SensorInteligente::enviarBateria(long intervalo, int porcentaje) 
{
  int bateria = SensorInteligente::compararNivelBateria(porcentaje); 
  if (millis() - _tiempoAnterior > intervalo) {
    _tiempoAnterior = millis();
    if(bateria > 30){
      SensorInteligente::enviarPorcentajeBateria(bateria);
    }else{
      Serial.println(" -> Bateria baja");
    }
  }
}


/** 
 * @brief   Realiza el envío de la batería utilizando el algoritmo rot47 para que los datos lleguen encriptados al Backend de Sigfox.
 *          Utiliza comandos AT para enviar un único byte, no se necesita la librería Isigfox.
 * @param   bateria: valor entero del porcentaje de la batería [0, 100] que se desea enviar.
 */
void SensorInteligente::enviarPorcentajeBateria(int bateria){  
  char cadena[2];
  char cadenaEnviar[3];
  for (int i = 0; i < sizeof((uint8_t)bateria); i++) {
    char str[1];
    sprintf(str, "%02x", (uint8_t)bateria);
    strcat(cadena, str);
  }
  Serial.print("cadena: ");
  Serial.println(cadena);  
  char *enviar = rot47(cadena);
  uint8_t *c = (uint8_t *)cadena;
  const uint8_t payload = 2;
  uint8_t newBuffer[payload];
  newBuffer[0] = c[0];
  newBuffer[1] = c[1];
  for (int i = 0; i < sizeof(newBuffer); i++){
    char cad1[2];
    sprintf(cad1, "%02x", newBuffer[i]);
    strcat(cadenaEnviar, cad1);
  }
  Serial.print("rot47: ");Serial.println(enviar);
  Serial.println("AT$RC");
  delay(100);
  Serial.print("AT$SF=");
  if ((int)bateria < 16)Serial.print("0");
  //Serial.println(bateria, HEX);
  Serial.println(cadenaEnviar);  
  memset(enviar, '\0', strlen(enviar));
  memset(cadena, '\0', strlen(cadena));
  memset(cadena, '\0', strlen(cadenaEnviar));
  delay(3000);
}

/**
* @brief    Realiza la encriptacion a traves de ROT47 donde al leer una cadena de texto, cada
*           caracter se reemplaza por su ASCII 47 posiciones despues.
* @param    s: cadena texto cualquiera a la cual se aplicara el algoritmo
* @return   s: cadena de texto cifrada bajo ROT47 
*/
char *SensorInteligente::rot47(char *s)
{
  char *p = s;
  while(*p) {
    if(*p >= '!' && *p <= 'O'){
      *p = ((*p + 47) % 127);
    }else if(*p >= 'P' && *p <= '~'){
      *p = ((*p - 47) % 127);
    }
    p++;
  }
  return s;
}
