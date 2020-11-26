/*
    This creates two empty databases, populates values, and retrieves them back
    from the SPIFFS file 
*/
#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <SPI.h>
#include <FS.h>
#include "SPIFFS.h"

#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <sqlite3.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include "DHT.h"

const char *ssid = "wifi name";
const char *password = "your password";
/* You only need to format SPIFFS the first time you run a
   test or else use the SPIFFS plugin to create a partition
   https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true
#define INSERT_DATA_INTERVAL 50000
sqlite3 *db;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
// Variables to save date and time
String formattedDate;
String dayStamp;
String timeStamp;

//------------------DHT-------------------
#define DHTPIN 4 // Digital pin connected to the DHT sensor
// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type you're using!
#define DHTTYPE DHT11 // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
//<------------------DHT-------------------
DHT dht(DHTPIN, DHTTYPE);

WebServer server(80);
const char *data = "Callback function called";
static int callback(void *data, int argc, char **argv, char **azColName)
{
  int i;
  Serial.printf("%s: ", (const char *)data);
  for (i = 0; i < argc; i++)
  {
    Serial.printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  Serial.printf("\n");
  return 0;
}

int db_open(const char *filename, sqlite3 **db)
{
  int rc = sqlite3_open(filename, db);
  if (rc)
  {
    Serial.printf("Can't open database: %s\n", sqlite3_errmsg(*db));
    return rc;
  }
  else
  {
    Serial.printf("Opened database successfully\n");
  }
  return rc;
}

char *zErrMsg = 0;
int db_exec(sqlite3 *db, const char *sql)
{
  Serial.println(sql);
  long start = micros();
  int rc = sqlite3_exec(db, sql, callback, (void *)data, &zErrMsg);
  if (rc != SQLITE_OK)
  {
    Serial.printf("SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
  }
  else
  {
    Serial.printf("Operation done successfully\n");
  }
  Serial.print(F("Time taken:"));
  Serial.println(micros() - start);
  return rc;
}

bool openDb()
{

  int rc;

  if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
  {
    Serial.println("Failed to mount file system");
    return true;
  }

  // list SPIFFS contents
  File root = SPIFFS.open("/");
  if (!root)
  {
    Serial.println("- failed to open directory");
    return true;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - not a directory");
    return true;
  }
  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIR : ");
      Serial.println(file.name());
    }
    else
    {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("\tSIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }

  // remove existing file
  //SPIFFS.remove("/mydb.db");

  sqlite3_initialize();

  if (db_open("/spiffs/mydb.db", &db))
    return true;

  rc = db_exec(db, "CREATE TABLE IF NOT EXISTS test1 (Id  INTEGER  PRIMARY KEY,time INTEGER, temp Real, hum Real);");
  if (rc != SQLITE_OK)
  {
    sqlite3_close(db);
    Serial.println("create table error");
    while (true)
    {
      Serial.println("DB error it's not possible to continue");
    }

    return true;
  }
  return true;
}
sqlite3_stmt *res;
int rec_count = 0;
const char *tail;

int counter = 0;

void handleJsonData()
{
  int skip = 0;
  if (server.hasArg("skip"))
  {
    skip = atoi(server.arg("skip").c_str());
  }
  Serial.println(skip);

  char sql[100];
  sprintf(sql, "Select * from test1 order by time desc LIMIT %d,50", skip);
  char *json_response = (char *)malloc(8000);
  *json_response = '\0';
  Serial.println(json_response);
  //static char json_response[8024];

  int rc = sqlite3_prepare_v2(db, sql, 1000, &res, &tail);
  bool firstRow = true;

  const char beginHtml[] PROGMEM = R"rawliteral(<!DOCTYPE html>
<html>
<link href="https://stackpath.bootstrapcdn.com/bootstrap/4.4.1/css/bootstrap.min.css" rel="stylesheet" integrity="sha384-Vkoo8x4CGsO3+Hhxv8T/Q5PaXtkKtu6ug5TOeNV6gBiFeWPGFN9MuhOf23Q9Ifjh" crossorigin="anonymous">
<section class="container">
	<h2>any text</h2>
	<table class="order-table table">
		<thead>
			<tr>
				<th>Temperature</th>
				<th>Humidity</th>
				<th>Date</th>				
			</tr>
		</thead>
    <tbody id='tbody'>
		</tbody>
	</table>
</section>
<script> )rawliteral";
  char *p = json_response;
  //
  //p++;
  strcat(json_response, beginHtml);
  p += strlen(beginHtml);
  Serial.println(json_response);
  p += sprintf(p, " var counter=%d; var json=[", counter++);
  //*p++ = '[';
  while (sqlite3_step(res) == SQLITE_ROW)
  {
    if (!firstRow)
    {
      *p++ = ',';
    }
    else
      firstRow = false;
    p += sprintf(p, "[%s,%s,%s]", sqlite3_column_text(res, 2), sqlite3_column_text(res, 3), sqlite3_column_text(res, 1));

    rec_count++;
  }
  *p++ = ']';
  sqlite3_finalize(res);
  *p = '\0';
  const char endHtml[] PROGMEM = R"rawliteral(
window.onload=function(e){
var res='';
// creating all cells
for (var i = 0; i < json.length; i++) {
var row=json[i];
res+='<tr><td>'+row[0]+'</td><td>'+row[1]+'</td><td>'+(new Date(row[2]*1000)).toLocaleString()+'</td></tr>';}
document.getElementById("tbody").innerHTML = res;
var prev=parseInt((new URLSearchParams(location.search)).get("skip"));
prev=isNaN(prev)? 50:prev+50;
document.getElementById("page").href = '/?skip='+prev;
}
</script>
<a href='/'>first page</a>
<a  href='' id='page'>previous 50</div>
</body>
</html>)rawliteral";
  strcat(p, endHtml);
  p += strlen(endHtml);
  *p++ = '\0';
  server.send(200, "text/html", json_response);
  free(json_response);
}

void insertDataToDb()
{
  float hum = dht.readHumidity();
  if (isnan(hum))
  {
    hum = -99;
    Serial.println("Can't get Humidity");
  }
  // Read temperature as Celsius (the default)
  float temp = dht.readTemperature();
  if (isnan(temp))
  {
    temp = -99;
    Serial.println("Can't get Temperature");
  }
  // Read temperature as Fahrenheit (isFahrenheit = true)
  //  float f = dht.readTemperature(true);
  int rc;
  char sql[100];
  unsigned long time = timeClient.getEpochTime();
  if (time < 1606000000)
  {
    Serial.println("Time error, trying to fix it");
    timeClient.update();
  }

  sprintf(sql, "INSERT INTO test1 (time,temp,hum) VALUES (%Lu,%.1f,%.1f);", time, temp, hum);
  rc = db_exec(db, sql);
  if (rc != SQLITE_OK)
  {
    sqlite3_close(db);
    return;
  }
}

void setup()
{

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for connection
  Serial.print("Connecting to ");
  Serial.print(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();
  // Set offset time in seconds to adjust for your timezone, for example:
  // GMT +1 = 3600
  // GMT +8 = 28800
  // GMT -1 = -3600
  // GMT 0 = 0
  //timeClient.setTimeOffset(28800);
  timeClient.update();

  Serial.println(timeClient.getEpochTime());
  openDb();

  server.on("/", handleJsonData);
  server.begin();
  Serial.println("HTTP server started");
  int64_t t = millis();
  dht.begin();
  //insertDataToDb();

  Serial.println((int)(millis() - t));
}

uint64_t do_not_update_until = 5000;
void loop()
{
  server.handleClient();
  if (millis() > do_not_update_until)
  {
    insertDataToDb();
    do_not_update_until = millis() + INSERT_DATA_INTERVAL;
  }
}
