#define ANTENNA_ID 1

#include <Time.h>
//try Serial.parseFloat();

double accel1, accel2, accel3;
double gyro[3];
String reply = "";
double cal_x = 0, cal_y = 0, cal_z = 0;
double threshold = 20;
double lat = 0.0, lng = 0.0;
boolean fix = false;
unsigned long time_interval = 30;
unsigned long start_time = 0;
double DELAY = 10;
void setup(){
  Serial.begin(57600);
  Serial1.begin(115200);        //XBee
  Serial2.begin(57600);        //IMU
  Serial3.begin(9600);        //GPS
  delay(500);
  readIMU();
  cal_x = accel1;
  cal_y = accel2;
  cal_z = accel3;
}

void loop(){
  //Serial.println("Loop");
  if(Serial1.available() > 2){
    char c1 = Serial1.read();
    if(c1 == '#'){
      char c2 = Serial1.read();
      if(c2 == 'c'){
        readIMU();             //calibrate
        cal_x = accel1;
        cal_y = accel2;
        cal_z = accel3;
      }
      else if(c2 == 't'){
        threshold = Serial1.parseFloat();
        Serial1.println(threshold);
      }
      else if(c2 == 'g'){    //get gps
        Serial1.println("G");
        readGPS();
      }
      else if(c2 == 'i'){
        readIMU();
        Serial1.print(accel1 - cal_x);
        Serial1.print(", ");
        Serial1.print(accel2 - cal_y);
        Serial1.print(", ");
        Serial1.print(accel3 - cal_z);
        Serial1.print(",");
        Serial1.print(gyro[0]);
        Serial1.print(",");
        Serial1.print(gyro[1]);
        Serial1.print(",");
        Serial1.print(gyro[2]);
        Serial1.println("");
      }
      else if(c2 == 'b'){
        float f = Serial1.parseFloat();
        time_interval = f; 
      }
      else if(c2 == 'd'){
        DELAY = Serial1.parseFloat(); 
      }
    }
  }
  readIMU();
  if(accel1 > cal_x+threshold || accel1 < cal_x-threshold || accel2 > cal_y+threshold || accel2 < cal_y-threshold || accel3 > cal_z+threshold || accel3 < cal_z-threshold){
    start_time = now();
    while(time_interval > (now() - start_time)){
      //Serial.println("Printing");
      readIMU();
      Serial1.print(accel1 - cal_x);
      Serial1.print(", ");
      Serial1.print(accel2 - cal_y);
      Serial1.print(", ");
      Serial1.print(accel3 - cal_z);
      Serial1.print(",");
      Serial1.print(gyro[0]);
      Serial1.print(",");
      Serial1.print(gyro[1]);
      Serial1.print(",");
      Serial1.print(gyro[2]);
      Serial1.println("");
      delay(DELAY);
      if(Serial1.available() > 1){
        Serial1.read();
        char c = Serial1.read();
        if(c == 'e')
          break; 
      }
    }
  }
}

bool validFix(String s)
{
  if (s[19] != ',')   // Check the GPRMC string for Fix
      return true;
  else
      return false;
}

void readGPS(){
  fix = false;
  while(!fix){
    Serial1.println("loop");
    readGPSData();
    Serial1.println(reply);
    if(reply.substring(0,5) == "GPRMC"){
      Serial1.println(reply);
      if(validFix(reply)){
        String latitude = reply.substring(18,27);
        String longitude = reply.substring(31,41);
      
        char latbuf[latitude.length()], lngbuf[longitude.length()];
        latitude.toCharArray(latbuf,latitude.length());
        longitude.toCharArray(lngbuf,longitude.length());
      
        convertLatLngToDecimal(latbuf, lngbuf);
        if (reply[29] == 'S')
         lat = (-1)*lat;
    
        if (reply[42] == 'W')
           lng = (-1)*lng;
        fix = true;
      }
    }
  }
  Serial1.println("FIX!!!!!");
  Serial1.print("Lat: ");
  Serial1.println(lat);
  Serial1.print("Long: ");
  Serial1.println(lng);
}

void convertLatLngToDecimal(char latbuf[], char lngbuf[])
{
  double d = atof(latbuf);
  lat = (int) (d/100);
  lat += (d-(lat*100))/60;
  
  d = atof(lngbuf);
  lng = (int) (d/100);
  lng += (d-(lng*100))/60;
}


void readIMU(){
  Serial2.print("#j");
  delayMicroseconds(10);
  accel1 = Serial2.parseFloat();
  accel2 = Serial2.parseFloat();
  accel3 = Serial2.parseFloat();
  gyro[0] = Serial2.parseFloat();
  gyro[1] = Serial2.parseFloat();
  gyro[2] = Serial2.parseFloat();
}

int convertAscii(char c){
  if(c >=48 && c <=57)
    return c-48;
  else{
   Serial.println("NOT ASCII!");
   Serial.println(c);
   return -1;
  }
}

void readGPSData(){
  char c[100] = "";
  if (Serial3.available())
  {
    while (Serial3.read() != '$') { }
    Serial3.readBytesUntil('\r',c,100);
  }
  reply = c;
}
