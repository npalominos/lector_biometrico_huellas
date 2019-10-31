//IMPLEMENTACION DE PROTOCOLO DE LECTURA BIOMETRICA DE HUELLAS
//NESTOR PALOMINOS 2018

//PARA CONECTAR LA SD SHIELD AL ARDUINO MEGA:
//13>53 12>50 11>51 10>8(CS)
//PARA QUE FUNCIONE EL RTC:
//A4>SDA20 A5>SCL21 


#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include "RTClib.h"
#include <Tone.h>

Tone tone1;
char x[10];
byte y[150];
String parametro[5];

int indices[150];
int cont=0,cont2=0;

int flag_c0=0;

RTC_DS1307 RTC;

String FechaHora="";


//*******************************************
//*************************TRAMAS FINGERPRINT
//*******************************************

  byte DetectaDedo0[]     = {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x03, 0xc0};  //10
  byte DetectaDedo1[]     = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x03, 0xc0};  //12 <-esta es la oficial segun el data
  
  byte GetImage[]         = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x04, 0xc0};
  
  
  byte GenTempletA[]      = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x01, 0x00,0x06, 0xc0};
  byte GenTempletB[]      = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x02, 0x00,0x07, 0xc0};
  
  byte MergeTwoTemplate[] = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x06, 0x00, 0x08, 0xc0};
  byte StoreTemplate[]    = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x07, 0x01, 0x00, 0x01, 0x00,0x0e, 0xc0};//15
  
  //                                                                          Cmd  bufId    StrPg       PgNum    
  //byte Search[]         = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x05, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x0f, 0xc0};
  byte Search[]           = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x06, 0x05, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x0f, 0xc0};//17
 
  
  byte DetectaDedo2[]     = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, 0x04, 0xc0};  //12
  byte DetectaDedo3[]     = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x02, 0x00, 0x08, 0xc0};
  byte Final[]            = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x03, 0x02, 0x00, 0x08, 0xc0};
  
  byte LuzDoble[]         = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x16, 0x12, 0x07, 0x03, 0x03, 0xc0}; //14
  byte LuzRoja1[]         = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x16, 0x01, 0x05, 0x03, 0x03, 0xc0}; //14
    
  //                                                                           tipo de luz   duracion
  //byte LuzDoble[]         = {0xc0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x16, <0x12>, <0x07>, 0x03, 0x03, 0xc0}; //14
  
  
  
  byte Reset[]            = {0xc0, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x15, 0x00, 0x17, 0xc0}; //12
  
  

//*******************************************
//*********************************SETUP
//*******************************************

void setup()
{
   Serial.begin(57600);
   Serial1.begin(57600);

   Wire.begin();
   RTC.begin();

   tone1.begin(3);
   
   if (! RTC.isrunning()) {
     Serial.println("RTC is NOT running!");
     RTC.adjust(DateTime(__DATE__, __TIME__));
   }
   
  Serial.println("ARDUINO OS...");
  Serial.println("=============");
  pinMode(SS, OUTPUT);

  if (!SD.begin(10,11,12,13)) {
    Serial.println("ERROR");
    return;
  }

  Serial.println("OK.... HELP para ver la ayuda");
  Serial.println(">");
  
  //desactiva temporalmente la conexion gprs
  //mySerial.println("AT$AREG=0");
  
}

//*******************************************
//*********************************LOOP
//*******************************************


void loop()
{


  if( Serial.available()){
        char dato_in =Serial.read();
        Serial.print(dato_in); 
        
        x[cont]=dato_in;
        cont++; 
        
        if(dato_in==0x0A || dato_in==0x0D){
         ProcesaTrama();
         cont=0;
         VaciarArreglos(); 
        }
    }
     
    //***si se recibe algo del modulo conectado a serial1
    if( Serial1.available()){
        byte dato1=Serial1.read();
        MuestraHexa(dato1);
        
        y[cont2]=dato1;
        cont2++; 
      
      
        if(dato1==0xC0 && flag_c0==1){
          Serial.print("-> largo=");
          Serial.println(cont2);
          cont2=0;
          ProcesaTrama2(); 
          VaciarArreglos2();
        }
      
        if(dato1==0xC0){
          flag_c0=!flag_c0;
        }
        

    }
    //***
  
}

void VaciarArreglos(){
 for(int i=0;i<10;i++){
   x[i]=' ';
 } 
 Serial.println(" ");
 Serial.print(">");
}

void VaciarArreglos2(){
   for (int i=0;i<150;i++){       
     y[i]=' ';
     indices[i]=0;
  }  
}

 void EnviarArray(byte x[],int n){
  int largo=n;
  
  for(int i=0;i<largo;i++){
    Serial1.write(x[i]);
    MuestraHexa(x[i]);
    delay(100);
  } 
 }
 
 
//*******************************************
//********FUNCIONES DE DESPLIEGUE HEXADECIMAL
//******************************************* 
 
void MuestraHexa(char x){
        Serial.print("0x"); 
        Serial.print(htoaH(x)); 
        Serial.print(htoaL(x)); 
        Serial.print(" ");
}

  
char htoaH(byte dato){
     char hex;
     hex=dato>>4;
     if(hex>0x09){
        return hex+'A'-10;
     }
     else{
        return hex+'0';
     }
}

char htoaL(byte dato){
     char hex;
     hex=dato&=0x0F; 
     if(hex>0x09){
        return hex+'A'-10;
     }
     else{
        return hex+'0';
     }
}


//*******************************************
//******FUNCIONES GENERICAS LECTURA/ESCRITURA
//*******************************************


void Grabar(String archivo,String dato){
  
  //mySerial.end();
  
  File dataFile = SD.open("archivo.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.println(dato);
    dataFile.close();
    delay(1500);
    Serial.println(dato);
    Serial.println(archivo +" ..grabado ok");
  }  
  else {
    Serial.println("error");
  } 
  Leer("archivo.txt");
}

void Leer(String archivo){

  char charBuf[archivo.length()+1];
  //***
  /*
  for(int i=0;i<archivo.length()+1;i++){
    Serial.println(charBuf[i]);
  }
  */
  //***
  File dataFile = SD.open("archivo.txt");
  delay(500);
  if (dataFile) {
    Serial.println("archivo.txt:");
    while (dataFile.available()) {
    	Serial.write(dataFile.read());
        delay(100);
    }
    dataFile.close();
  } else {
    Serial.println("error opening datalogger.txt");
  }


}

//*******************************************
//******FUNCIONES GENERICAS DE RTC y ANALOGOS
//*******************************************

void MuestraHora(){
   DateTime now = RTC.now();
   FechaHora=(String)now.year()+"/"+(String)now.month()+"/"+(String)now.day()+";"+(String)now.hour()+":"+(String)now.minute()+":"+(String)now.second();  
   Serial.println(FechaHora);
}

void MuestraAnalogo(){
 
  String dataString = "";

  float sensor=analogRead(0);
  float tempC = (5.0 * sensor * 100.0)/1024.0; 

  String strSensor=String(tempC,2);
  dataString = FechaHora + ";" +strSensor;

  Serial.println(sensor);
}


//************************************************
//*****SEPARAR STRINGS POR ESPACIOS (PARAMETRIZAR)
//************************************************


void Parametrizar(String str){
  
  int indice=0;
  for(int i=0;i<5;i++){
   parametro[i]="";
  } 
  Serial.println("");
  //Serial.println(str.length());
  for(int k=0;k<str.length();k++){
    String dato=str.substring(k,k+1);
    if(dato==" "){
      indice++;
      //Serial.println("n:"+indice);
    }
    else{
      parametro[indice]=parametro[indice]+dato;
      //Serial.println(parametro[indice]);
    }
  }
  /*
  Serial.println("parametro 0: " +parametro[0]);
  Serial.println("parametro 1: " +parametro[1]);
  Serial.println("parametro 2: " +parametro[2]);
  Serial.println("parametro 3: " +parametro[3]);
  Serial.println("parametro 4: " +parametro[4]);
  */ 
}


//*******************************************
//***********************PROCESATRAMA COMANDO
//*******************************************


void ProcesaTrama(){
 String trama="";
 for(int i=0;i<cont-1;i++){
   trama=trama+String(x[i]);
 }
 Parametrizar(trama);
 String str=parametro[0]; 
 
 //***
 //String cmd=parametro[0];
 //Serial.println("par0: "+cmd);
 //***
 str.toUpperCase();
 Serial.println(">"+str);
 
 if(str=="DATE"){
   MuestraHora();
 }
 else if(str=="ANALOG"){
   MuestraAnalogo();
 }
 else if(str=="HELP"){
   Help();
 }
 else if(str=="DEDO0"){
   EnviarArray(DetectaDedo0,10);
 }
 else if(str=="DETECT"){
   EnviarArray(DetectaDedo1,12);
 }
 else if(str=="GETIMAGE"){
   EnviarArray(GetImage,12);
 }
 else if(str=="DEDO2"){
   EnviarArray(DetectaDedo2,12);
 }
 else if(str=="RESET"){
   EnviarArray(Reset,12);
 }
 else if(str=="GENTEMPLETA"){
   EnviarArray(GenTempletA,13);
 }
 else if(str=="GENTEMPLETB"){
   EnviarArray(GenTempletB,13);
 }
 else if(str=="MERGE"){
   EnviarArray(MergeTwoTemplate,12);
 } 
 else if(str=="STORE"){
   EnviarArray(StoreTemplate,15);
 }
 else if(str=="SEARCH"){
   Serial.println("Detect->GetImage->GenTempletA->Search");
   EnviarArray(Search,17);
 }
 else if(str=="START"){
   EnviarArray(DetectaDedo0,10);Serial.println(" * ");
   delay(1000);
   EnviarArray(GetImage,12);Serial.println(" * ");
   delay(1000);
   EnviarArray(GenTempletA,12);Serial.println(" * ");
   delay(1000);
   EnviarArray(LuzDoble,14);Serial.println(" * ");
   delay(1000);
   EnviarArray(DetectaDedo0,10);Serial.println(" * ");
   delay(1000);
   EnviarArray(GetImage,12);Serial.println(" * ");
   delay(1000);
   EnviarArray(GenTempletB,12);Serial.println(" * ");
   delay(1000);
   EnviarArray(LuzDoble,14);Serial.println(" * ");
   delay(1000);
   EnviarArray(MergeTwoTemplate,12);Serial.println(" * ");
   delay(1000);
   EnviarArray(GenTempletB,12);Serial.println(" * ");
   delay(1000);
 }
 
 else if(str=="LUZ1"){
   EnviarArray(LuzRoja1,14);
 }
 else if(str=="LUZ2"){
   EnviarArray(LuzDoble,14);
 }
 
 else if(str=="TONO"){
   
   if(parametro[2]!=""){
     int frec=parametro[1].toInt();   
     int duracion=parametro[2].toInt();
     Serial.print(frec); 
     Tono(frec,duracion); 
   }
   else{
     Serial.println("forma de uso: tono <frec [hz]> <duracion [ms]>");
   }
   
 }
 else if(str=="SD"){
     if (!SD.begin(4)) {
        Serial.println("initialization failed!");
     }
     else{
        Serial.println("initialization done.");
     }
 }
 else if(str=="ENVIAR"){
   if(parametro[1]!=""){
     Serial.println("enviando byte por serial 1");
     Serial1.println(parametro[1]);
   }
   else{
    Serial.println("forma de uso: enviar <byte>");
    Serial.println("ejemplo: enviar 1"); 
   }
 }  
 else if(str=="READ"){
   Leer("archivo.txt");
 } 
 else if(str=="DEL"){
  if(parametro[1]!=""){
    Serial.println("Borrando "+parametro[1]);
    SD.remove("archivo.txt");
  }
  else{
    Serial.println("forma de uso: del <archivo>");
  }
 } 
 else if(str=="WRITE"){
   if(parametro[1]!=""){
     Grabar("archivo.txt",parametro[1]);
   }else{
     Serial.println("Faltan parametros!");
   }
 }  

}



void Help(){
  Serial.println("                AYUDA                 ");
  Serial.println("    ==============================    ");
  Serial.println(" DATE         : Muestra Fecha y hora  ");
  Serial.println(" ANALOG       : Muestra valor analogo ");
  Serial.println(" READ         : Lee archivo           ");
  Serial.println(" WRITE        : Escribe archivo       ");
  Serial.println(" DEL          : Borra archivo         ");
  Serial.println(" TONE         : Genera Tono           ");
  Serial.println(" DETECT       : Detecta dedo          ");
  Serial.println(" GETIMAGE     : Obtiene imagen        ");
  Serial.println(" GENTEMPLETA  : Genera template A     ");
  Serial.println(" GENTEMPLETB  : Genera template B     ");
  Serial.println(" MERGE        : Compara Templates     ");
  Serial.println(" STORE        : Guarda Huella         ");    
  Serial.println(" SEARCH       : Busca Huella          ");   
  Serial.println(" LUZ1         : Luz roja              ");   
  Serial.println(" LUZ2         : Luz verde             ");   
  
}

void Pos(){
 Serial1.println("AT$GPSRD=10"); 
 delay(1000);
}

String ReadFile(int Linea,char Ruta[]){
 int Lin=0;
 String Resultado;
 File myFile;
 byte Bin;
 myFile = SD.open(Ruta);;
 if (myFile) {
  while (myFile.available()) {
    Bin=myFile.read();
    if (Bin==13){
       Lin++;
       myFile.read();
    }
    else{
       if (Lin==Linea){
          Resultado=Resultado+(char(Bin));
       }
       if (Lin!=Linea){
          myFile.close();
          return Resultado;
       }
   }
  }
  myFile.close();
  return Resultado;
 }
}


  
void Tono(int frecuencia,int duracion)
{
  tone1.play(frecuencia,duracion);
  
  
}


//********************************************
//*********PROCESAMIENTO DE TRAMA DE RESPUESTA
//********************************************

void ProcesaTrama2(){
   
    //RESPUESTA A DETECT FINGER
    //0xC0 0x07 0x00 0x00 0x00 0x00 0x00 0x01 <0x00> 0x00 0x08 0xC0
    //                                      confirm code
    
    if(y[0]==0xC0 && y[1]==0x07 && y[2]==0x00 &&y[3]==0x00 && y[4]==0x00 && y[5]==0x00 && y[6]==0x00 && y[7]==0x01){
      
      if(y[8]==0x00){
        Serial.println("Detecto dedo");
      }
      else if(y[8]==0x02){
        Serial.println("No hay dedo");
      }

    }  
  }
  
  
/*
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

//TODAS LAS TRAMAS COMIENZAN Y TERMINAN CON 0xC0

//PAQUETE DE COMANDO
//1 BYTE: PACKET FLAG      0x01
//4 BYTES: MODULE ADDRESS  0x00 0x00 0x00 0x00
//2 BYTES:PACKET LENGTH    0x00 0x01
//1 BYTE: COMMAND CODE     0x01   //0x01:detectfinger 0x02:getimage
//2 BYTES: CHECKSUM        0x00 0x03   //suma de bytes desde packet flag hasta antes de checksum
                                       //en este caso, 0x01+0x01+0x01=0x03   [MSB] [LSB]


//////////////////////////////////////////////////////////////////////////////////////
//SEGUN EL ALGORITMO, PRIMERO SE DEBE ESPERAR A QUE DETECTFINGER DE POSITIVO
//LUEGO SE EJECUTA GET IMAGE, EL CUAL TAMBIEN DEBE DAR POSITIVO
//POSTERIORMENTE GENERAR EL TEMPLATE A CON LA PRIMERA TOMA DE LA HUELLA
//Y EN CASO DE QUE SEA POSITIVO, EJECUTA NUEVAMENTE EL ALGORITMO, PERO
//AHORA GUARDA UN SEGUNDO TEMPLATE "B" DONDE FINALMENTE SE HACE UN "MERGE" 
//COMPARANDO AMBOS TEMPLATE, VALIDANDO Y GUARDANDOLO EN MEMORIA

//Buffer A=0x01 , Buffer B=0x02

ARDUINO OS...
=============
OK.... HELP para ver la ayuda
>
detect
>DETECT
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x01> [0x00 0x03] [0xC0]
                                                 0x01: cmd detectfinger

>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x00> [0x00 0x08] [0xC0] -> largo=12
                                     confirm code 0x00: detecto dedo
Detecto dedo

>

>getimage
>GETIMAGE
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x02> [0x00 0x04] [0xC0]
                                                 0x02: cmd getimage
                                                 
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x06] <0x00> [0x62] [0x08] [0x08] [0x08] [0x08] [0x00 0x8F] [0xC0] -> largo=17
                                     confirm code 0x00: ok
                                                  0x01: packet error
                                                  0x03: enrollment error
gentempleta
>GENTEMPLETA
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x02] <0x03> <0x01> [0x00 0x06] [0xC0]
                                                 0x03:cmd get templet
                                                 0x01:buffer A
                                                 
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x00> [0x00 0x08] [0xC0] -> largo=12
                                                  0x00: generate minutae successful
                                                  0x01: packet receive error
//AHORA SE REPITE PARA EL TEMPLATE B

>detect
>DETECT
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x01] [0x01] [0x00 0x03] [0xC0]
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x01] [0x00] [0x00 0x08] [0xC0] -> largo=12
Detecto dedo

getimage
>GETIMAGE
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x02> [0x00 0x04] [0xC0]
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x06] <0x00> [0x62] [0x08] [0x08] [0x08] [0x08]  [0x00 0x8F] [0xC0] -> largo=17
 
gentempletb
>GENTEMPLETB
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x02] <0x03> <0x02> [0x00 0x07] [0xC0]
                                                 0x03:cmd get templet
                                                 0x02:buffer B
                                                 
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x00> [0x00 0x08] [0xC0] -> largo=12
                                                  0x00: generate minutae successful
                                                  0x01: packet receive error


>

>merge
>MERGE
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x01] [0x06] [0x00 0x08] [0xC0]
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x0A> [0x00 0x12] [0xC0] -> largo=12
                                                  0x00:merge ok
                                                  0x01:packet error
                                                  0x0A:merge error
MERGE FAIL!
>
/////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////

ARDUINO OS...
=============
OK.... HELP para ver la ayuda
>
detect
>DETECT

Command Packet
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x01> [0x00 0x03] [0xC0]
  0      1             2                3         4        5         6

  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x01)
  2: 4 bytes - Module Address (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x01)
  4: 1 byte  - Command Code   (0x01) 0x01: cmd detectfinger
  5: 2 bytes - Checksum       (0x00 0x03)
  6: 1 byte  - End            (0xC0)    

Response Packet
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x00> [0x00 0x08] [0xC0]
   0      1             2                3         4        5         6 
   
  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x07)
  2: 4 bytes - Resvd          (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x01)
  4: 1 byte  - Confirm Code   (0x00) ok
  5: 2 bytes - Checksum       (0x00 0x08)
  6: 1 byte  - End            (0xC0) 
  
>

>getimage
>GETIMAGE

Command Packet
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x02> [0x00 0x04] [0xC0]
  0      1             2                 3        4        5         6
                                            
  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x01)
  2: 4 bytes - Module Address (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x01)
  4: 1 byte  - Command Code   (0x02) 0x02: cmd getimage
  5: 1 byte  - Buffer ID      (0x01:Buf A , 0x02: Buf B) 
  6: 2 bytes - Checksum       (0x00 0x04)
  8: 1 byte  - End            (0xC0)         
  
Response Packet  
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x06] [0x00] [0x63] [0x08] [0x08] [0x08] [0x08] [0x00 0x90] [0xC0]
   0      1              2               3         4      5      6      7      8     9         10        11
   
  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x07)
  2: 4 bytes - Resvd          (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x06)
  4: 1 byte  - Confirm Code   (0x00) ok
  5: 1 byte  - Valid Area     (0x63)
  6: 1 byte  - Up Border TB   (0x08)
  7: 1 byte  - Down Border DB (0x08)
  8: 1 byte  - Left Border LB (0x08)
  9: 1 byte  - Right Border RB(0x08)
  10:2 bytes - Checksum       (0x00 0x90)
  11:1 byte  - End            (0xC0) 


>

>gentempleta
>GENTEMPLETA

Command Packet
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x02] [0x03] [0x01] [0x00 0x06] [0xC0]
  0      1            2                 3         4      5         6        7
  
  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x01)
  2: 4 bytes - Module Address (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x02)
  4: 1 byte  - Command Code   (0x03)  0x03:cmd get templet
  5: 1 byte  - Buffer ID      (0x01:Buf A , 0x02: Buf B) 
  6: 2 bytes - Checksum       (0x00 0x07)
  8: 1 byte  - End            (0xC0) 

Response Packet
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x01] [0x00] [0x00 0x08] [0xC0]
   0      1             2                3         4        5         6
   
                                Confirm Code:     0x00: generate minutae successful
                                                  0x01: packet receive error
   
  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x07)
  2: 4 bytes - Resvd          (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x01)
  4: 1 byte  - Confirm Code   (0x00)
  5: 2 bytes - Checksum       (0x00 0x08)
  6: 1 byte  - End            (0xC0) 

>
//////////////aca repite la toma de huella y la guarda en buffer b

>detect
>DETECT

Command Packet
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x01> [0x00 0x03] [0xC0]
  0      1             2                3         4        5         6

  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x01)
  2: 4 bytes - Module Address (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x01)
  4: 1 byte  - Command Code   (0x01) 0x01: cmd detectfinger
  5: 2 bytes - Checksum       (0x00 0x03)
  6: 1 byte  - End            (0xC0)    

Response Packet
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x00> [0x00 0x08] [0xC0]
   0      1             2                3         4        5         6 
   
  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x07)
  2: 4 bytes - Resvd          (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x01)
  4: 1 byte  - Confirm Code   (0x00) ok
  5: 2 bytes - Checksum       (0x00 0x08)
  6: 1 byte  - End            (0xC0) 



>getimage
>GETIMAGE

Command Packet
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x02> [0x00 0x04] [0xC0]
  0      1             2                 3        4        5         6
                                            
  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x01)
  2: 4 bytes - Module Address (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x01)
  4: 1 byte  - Command Code   (0x02) 0x02: cmd getimage
  5: 1 byte  - Buffer ID      (0x01:Buf A , 0x02: Buf B) 
  6: 2 bytes - Checksum       (0x00 0x04)
  8: 1 byte  - End            (0xC0)                                                 
                                                 

Response Packet  
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x06] [0x00] [0x63] [0x08] [0x08] [0x08] [0x08] [0x00 0x90] [0xC0]
   0      1              2               3         4      5      6      7      8     9         10        11
   
  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x07)
  2: 4 bytes - Resvd          (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x06)
  4: 1 byte  - Confirm Code   (0x00) ok
  5: 1 byte  - Valid Area     (0x63)
  6: 1 byte  - Up Border TB   (0x08)
  7: 1 byte  - Down Border DB (0x08)
  8: 1 byte  - Left Border LB (0x08)
  9: 1 byte  - Right Border RB(0x08)
  10:2 bytes - Checksum       (0x00 0x90)
  11:1 byte  - End            (0xC0) 


>gentempletb
>GENTEMPLETB

Command Packet
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x02] <0x03> <0x02> [0x00 0x07] [0xC0]
  0       1             2                3        4       5        6        7
  
  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x01)
  2: 4 bytes - Module Address (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x02)
  4: 1 byte  - Command Code   (0x03)  0x03:cmd get templet
  5: 1 byte  - Buffer ID      (0x01:Buf A , 0x02: Buf B) 
  6: 2 bytes - Checksum       (0x00 0x07)
  8: 1 byte  - End            (0xC0)   
                                        

Response Packet  
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x01] [0x00] [0x00 0x08] [0xC0] 
   0      1             2                3         4        5         6
   
                                Confirm Code:     0x00: generate minutae successful
                                                  0x01: packet receive error
   
  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x07)
  2: 4 bytes - Resvd          (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x01)
  4: 1 byte  - Confirm Code   (0x00)
  5: 2 bytes - Checksum       (0x00 0x08)
  6: 1 byte  - End            (0xC0) 

>

>merge
>MERGE

Command Packet
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x01] [0x06] [0x00 0x08] [0xC0]
  0      1            2                 3         4         5        6

  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x00)
  2: 4 bytes - Module Address (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x01)
  4: 1 byte  - Command Code   (0x06)
  7: 2 bytes - Checksum       (0x00 0x08)
  8: 1 byte  - End            (0xC0)  

Response Packet  
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x01] <0x00> [0x00 0x08] [0xC0]
   0       1            2                 3        4         5        6
   
                                Confirm Code:     0x00:merge ok
                                                  0x01:packet error
                                                  0x0A:merge error
   
  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x07)
  2: 4 bytes - Resvd          (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x01)
  4: 1 byte  - Confirm Code   (0x00)
  5: 2 bytes - Checksum       (0x00 0x08)
  6: 1 byte  - End            (0xC0)  
  
MERGE OK!!

>

>store
>STORE

Command Packet
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x04] [0x07] [0x01] [0x00 0x01] [0x00 0x0E] [0xC0]
  0      1              2               3         4      5         6          7         8
  
  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x00)
  2: 4 bytes - Module Address (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x04)
  4: 1 byte  - Command Code   (0x07)
  5: 1 byte  - Buffer ID      (0x01:Buf A , 0x02: Buf B)
  6: 2 bytes - Page Number    (0x00 0x01)
  7: 2 bytes - Checksum       (0x00 0x0E)
  8: 1 byte  - End            (0xC0)  
  
Response Packet  
>[0xC0] [0x07] [0x00 0x00 0x00 0x00] [0x00 0x01] [0x00] [0x00 0x08] [0xC0] 
   0      1              2               3          4       5         6
   
                                Confirm Code:     0x00: Storage successful
                                                  0x01: Packet receive error
                                                  0x0b: PageID exceeds fingerprint range

  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x07)
  2: 4 bytes - Resvd          (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x01)
  4: 1 byte  - Confirm Code   (0x00)
  5: 2 bytes - Checksum       (0x00 0x08)
  6: 1 byte  - End            (0xC0)  

>SEARCH
Detect->GetImage->GenTempletA->Search

Command Packet
[0xC0] [0x01] [0x00 0x00 0x00 0x00] [0x00 0x06] [0x05] [0x01] [0x00 0x01] [0x00 0x01] [0x00 0x0E] [0xC0]
  0      1              2                3        4      5         6          7           8        9

  0: 1 byte  - Start          (0xC0)
  1: 1 byte  - Packet Flag    (0x00)
  2: 4 bytes - Module Address (0x00 0x00 0x00 0x00)
  3: 2 bytes - Packet Length  (0x00 0x06)
  4: 1 byte  - Command Code   (0x05)
  5: 1 byte  - Buffer ID      (0x01:Buf A , 0x02: Buf B)
  6: 2 bytes - Start Page     (0x00 0x01)
  7: 2 bytes - Page Number    (0x00 0x01) 
  8: 2 bytes - Checksum       (0x00 0x0E)
  9: 1 byte  - End            (0xC0) 


>0xC0 0x07 0x00 0x00 0x00 0x00   0x00 0x23 0x00 0x00 0x01 0x00   0x00 0x00 0x00 0x00

 0x00 0x00 0x00 0x00 0x00 0x00   0x00 0x00 0x00 0x00 0x00 0x00   0x00 0x00 0x00 0x00
 
 0x00 0x00 0x00 0x00 0x00 0x00   0x00 0x00 0x00 0x00 0x00 0x00   0x2B 0xC0 
 
 -> largo=46
>
*/
   
