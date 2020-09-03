/*
 * ValoresSensados.cpp - Librería para detectar tanques de gas vacíos y mesas ocupadas peso usando velostat.
 * Created by Rosa M. Pincay, July 8, 2020.
 * Released into the ESPOL domain.
 */

#include "SensorInteligente.h"
#include "Arduino.h"

/** Crea un objeto de tipo SensorInteligente.
 * Sirve para medir el voltaje del velostat.
 * 
 * @param pinA0: pin análogo al que se conecta el velostat.
 */
SensorInteligente::SensorInteligente(int pinA0, float r1, float r2, float vin)
{
  _pinA1 = pinA0;
  _r1 = r1;
  _r2 = r2; 
  _vin = vin;
}

/*
 * Inicializa los puertos para establecer la comunicación serial UART
 */
SensorInteligente::SensorInteligente(int pinA0, int pinA1)
{
  _pinA0 = pinA0;
  _pinA1 = pinA1;
}

void SensorInteligente::inicializar()
{
  voltajeBateria = 0.0;
  _sensorMax = 0;
  _tiempoAnterior = 0.0;
  bateriaEnviar = 100.0;
  porcentajeBateria = 0.0;
  bateria = 0.0;
  voltajeMedido = 0.0;
}

float SensorInteligente::leerVoltajeBateria()
{
  bateria = analogRead(_pinA1);   
  voltajeBateria = (((float) bateria) * 5.0) / 1023.0;
  return voltajeBateria;
}

/*
 * Realiza la lectura del voltaje de la batería durante los primeros 2 segundos 
 * para establecer los valores de entrada mínimo y máximos.
 */
void SensorInteligente::calibrarBateria(float rBajo, float rArriba, float vIn){
  //rBajo = 1000; rArriba = 10000; vIn = 9;
  float vMax = (rBajo/(rBajo+rArriba))*vIn;
  _sensorMax = (int)(vMax*(1023/5));
  Serial.print("calibrado - Vout = ");
  Serial.println(_sensorMax);
}

/*
 * Toma los valores sensados por el velostat y de la batería. 
 * Se convierte el valor de la entrada analógica para el voltaje y,
 * se calibra el nivel de la batería para que esté en el rango 0 - 100.
 */
float SensorInteligente::leerVoltajeVelostat()
{
  voltajeAlfombra = analogRead(_pinA0);
  voltajeMedido = (((float) voltajeAlfombra) * 5.0) / 1023.0;
  Serial.print(voltajeMedido);
  return voltajeMedido;
}

/*
 * Obtiene el nivel de bateria más bajo.
 * @params porcentajeBateria
 */
float SensorInteligente::bateriaMenor(float porcentaje) 
{
  if (bateriaEnviar==100) {  
    bateriaEnviar = porcentaje; 
  }
  if ((porcentaje > bateriaEnviar-5)) {  
    bateriaEnviar = porcentaje; 
  }
  return bateriaEnviar;
}


/*
 * Función que envía el nivel de la batería a Sigfox en el intervalo asignado
 */
void SensorInteligente::enviarBateria(long intervalo,float porcentaje) 
{
  int bateria = (int)SensorInteligente::bateriaMenor(porcentaje); 
  Serial.print("Bateria  enviar: ");
  Serial.print(bateria);
  Serial.print(" - Porcentaje bateria: ");
  Serial.println(porcentaje); 
  if((int)bateria > 30){
    delay(intervalo-510);
    Serial.println(" <- Enviando...");
    SensorInteligente::enviarPorcentajeBateria((int)bateriaEnviar);
    Serial.println(" <- Enviado!!");
  }else{
    Serial.println("Bateria baja :(");
  }
}

void SensorInteligente::enviarPorcentajeBateria(int bateria){
  Serial.println("AT$RC");
  delay(500);
  Serial.print("AT$SF=");
  if ((int)bateria < 16)Serial.print("0");
  Serial.println((int)bateria, HEX);
}

float SensorInteligente::leerPorcentajeBateria(){ 
  bateria = analogRead(_pinA1);    
  delay(10);    
  porcentajeBateria = map((int)bateria, 0, _sensorMax, 0, 100);        
  return porcentajeBateria;
}
