
//Define Serial1 Name
#define SIM7000 Serial1

//Define GSM Model
#define TINY_GSM_MODEM_SIM7000

// See all AT commands, if wanted
#define DUMP_AT_COMMANDS


#include <TinyGsmClient.h>
#include <SPI.h>
#include <SD.h>
#include <Ticker.h>

#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SIM7000, Serial);
TinyGsm modem(debugger);
#else
TinyGsm modem(SIM7000);
#endif


#define UART_BAUD           9600
#define PIN_DTR             25
#define PIN_TX              27
#define PIN_RX              26
#define PWR_PIN             4

#define SD_MISO             2
#define SD_MOSI             15
#define SD_SCLK             14
#define SD_CS               13
#define LED_PIN             12

//Variables
int gnss_run_status = 0;
int fix_status = 0;
int year = 0;
int month = 0;
int day = 0;
int hour = 0;
int minutes = 0;
float secondWithSS = 0;
float lat = 0;
float lon = 0;
float msl_alt = 0;
float speed_over_ground = 0;
float course_over_ground = 0;
bool reserved1 = 0;
int fix_mode = 0;
int   hdop = 0;
int   pdop = 0;
int vdop = 0;
bool reserved2       = 0;
int gnss_satellites_used       = 0;
int gps_satellites_used       = 0;
int glonass_satellites_used       = 0;
bool reserved3       = 0;
int c_n0_max       = 0;
float hpa       = 0;
float vpa       = 0;

void enableGPS(void)
{
  // Set SIM7000G GPIO4 LOW ,turn on GPS power
  // CMD:AT+SGPIO=0,4,1,1
  // Only in version 20200415 is there a function to control GPS power
  modem.sendAT("+SGPIO=0,4,1,1");
  if (modem.waitResponse(10000L) != 1) {
    DBG(" SGPIO=0,4,1,1 false ");
  }
  modem.enableGPS();


}

void disableGPS(void)
{
  // Set SIM7000G GPIO4 LOW ,turn off GPS power
  // CMD:AT+SGPIO=0,4,1,0
  // Only in version 20200415 is there a function to control GPS power
  modem.sendAT("+SGPIO=0,4,1,0");
  if (modem.waitResponse(10000L) != 1) {
    DBG(" SGPIO=0,4,1,0 false ");
  }
  modem.disableGPS();
}

void modemPowerOn()
{
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, LOW);
  delay(1000);    //Datasheet Ton mintues = 1S
  digitalWrite(PWR_PIN, HIGH);
}

void modemPowerOff()
{
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, LOW);
  delay(1500);    //Datasheet Ton mintues = 1.2S
  digitalWrite(PWR_PIN, HIGH);
}


void modemRestart()
{
  modemPowerOff();
  delay(1000);
  modemPowerOn();
}

void setup()
{
  // Set console baud rate
  Serial.begin(115200);

  delay(10);

  modemPowerOn();

  SIM7000.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

  Serial.println("/**********************************************************/");
  Serial.println("To initialize the network test, please make sure your GPS");
  Serial.println("antenna has been connected to the GPS port on the board.");
  Serial.println("/**********************************************************/\n\n");

  delay(10000);

  if (!modem.testAT()) {
    Serial.println("Failed to restart modem, attempting to continue without restarting");
    modemRestart();
    return;
  }

  Serial.println("Start positioning . Make sure to locate outdoors.");
  Serial.println("The blue indicator light flashes to indicate positioning.");

  enableGPS();

}

void loop()
{

  String gps_raw = modem.getGPSraw();

  if (gps_raw != "") {
    gnss_run_status = splitter(gps_raw, ',', 0).toInt();
    fix_status = splitter(gps_raw, ',', 1).toInt();
    String date = splitter(gps_raw, ',', 2); //yyyyMMddhhmm ss.sss
    year = date.substring(0, 4).toInt();
    month = date.substring(4, 6).toInt();
    day = date.substring(6, 8).toInt();
    hour = date.substring(8, 10).toInt();
    minutes = date.substring(10, 12).toInt();
    secondWithSS = date.substring(12, 18).toFloat();
    lat = splitter(gps_raw, ',', 3).toFloat();//±dd.dddddd
    lon = splitter(gps_raw, ',', 4).toFloat();//±ddd.dddddd
    msl_alt = splitter(gps_raw, ',', 5).toFloat();//meters
    speed_over_ground = splitter(gps_raw, ',', 6).toFloat();//Km/hour
    course_over_ground = splitter(gps_raw, ',', 7).toFloat();//degrees
    fix_mode = splitter(gps_raw, ',', 8).toInt();
    reserved1 = splitter(gps_raw, ',', 9).toInt();
    hdop = splitter(gps_raw, ',', 10).toInt();
    pdop = splitter(gps_raw, ',', 11).toInt();
    vdop = splitter(gps_raw, ',', 12).toInt();
    reserved2 = splitter(gps_raw, ',', 13).toInt();
    gnss_satellites_used = splitter(gps_raw, ',', 14).toInt();
    gps_satellites_used = splitter(gps_raw, ',', 15).toInt();
    glonass_satellites_used = splitter(gps_raw, ',', 16).toInt();
    reserved3 = splitter(gps_raw, ',', 17).toInt();
    c_n0_max = splitter(gps_raw, ',', 18).toInt();//dBHz
    hpa = splitter(gps_raw, ',', 19).toFloat();//meters
    vpa = splitter(gps_raw, ',', 20).toFloat();//meters

    Serial.println("----------------------------------");

    Serial.print("GNSS Run Status:"); Serial.println(gnss_run_status);
    Serial.print("Fix Status:"); Serial.println(gnss_run_status);
    Serial.print("Year:"); Serial.println(year);
    Serial.print("Month:"); Serial.println(month);
    Serial.print("Day:"); Serial.println(day);
    Serial.print("Hour:"); Serial.println(hour);
    Serial.print("Minute:"); Serial.println(minutes);
    Serial.print("Seconds:"); Serial.println(secondWithSS, 6);
    Serial.print("Latitude:"); Serial.println(lat, 6);
    Serial.print("Longitude:"); Serial.println(lon, 6);
    Serial.print("MSL Altitude:"); Serial.println(msl_alt, 6);
    Serial.print("Speed Over Ground:"); Serial.println(speed_over_ground, 6);
    Serial.print("Course Over Ground:"); Serial.println(course_over_ground, 6);
    Serial.print("Fix Mode:"); Serial.println(fix_mode);
    Serial.print("Reserved1:"); Serial.println(reserved1);
    Serial.print("HDOP:"); Serial.println(hdop);
    Serial.print("PDOP:"); Serial.println(pdop);
    Serial.print("VDOP:"); Serial.println(vdop);
    Serial.print("Reserved2:"); Serial.println(reserved2);
    Serial.print("GNSS Satellites in View:"); Serial.println(gnss_satellites_used);
    Serial.print("GPS Satellites Used:"); Serial.println(gps_satellites_used);
    Serial.print("GLONASS Satellites used:"); Serial.println(glonass_satellites_used);
    Serial.print("Reserved3:"); Serial.println(reserved3);
    Serial.print("C/N0 max:"); Serial.println(c_n0_max);
    Serial.print("HPA:"); Serial.println(hpa, 6);
    Serial.print("VPA:"); Serial.println(vpa, 6);

  }
}

String splitter(String data, char separator, int index)
{
  int stringData = 0;
  String dataPart = "";

  for (int i = 0; i < data.length(); i++)
  {

    if (data[i] == separator)
    {

      stringData++;
    }
    else if (stringData == index)
    {

      dataPart.concat(data[i]);
    }
    else if (stringData > index)
    {

      return dataPart;
      break;
    }
  }

  return dataPart;
}
