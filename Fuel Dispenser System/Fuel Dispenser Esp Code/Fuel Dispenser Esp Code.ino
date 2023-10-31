#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FirebaseESP8266.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <LittleFS.h>
#include "index.h"

#define FIREBASE_HOST "https://fuel-dispenser-system-default-rtdb.firebaseio.com/"
#define FIREBASE_PROJECT_ID "fuel-dispenser-system"

#define ssidPath "/ssid.txt"
#define ssidPassPath "/ssidPass.txt"
#define brandNamePath "/brandName.txt"
#define locationPath "/location.txt"
#define passwordPath "/password.txt"
#define pricePath "/price.txt"

#define led D7

String ssid = "Fuel Dispenser By Nodex";
String ssidPass = "";

String brand;
String location;
String password;
String price;

bool fromFile = false;
bool wentOff = false;
String fileName = "";

ESP8266WebServer server(80);

FirebaseData firebaseData;
FirebaseJson json;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

String readFile(String path) {
  String a = "";
  File file = LittleFS.open(path, "r");
  if (!file) {
    return "";
  }
  while (file.available()) { a += file.readString(); }
  file.close();
  a.trim();
  return a;
}

void writeFile(String path, String message) {
  File file = LittleFS.open(path, "w");
  if (!file) {
    return;
  }
  if (file.print(message)) {
    Serial.println("Y");
  } else {
    Serial.println("F");
  }
  delay(200);
  file.close();
  return;
}

bool appendFile(String path, const char *message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = LittleFS.open(path, "a");
  if (!file) {
    Serial.println("Failed to open file for appending");
    return false;
  }
  if (file.print(message)) {
    Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
  return true;
}

void deleteFile(String path) {
  if (LittleFS.remove(path)) {
    Serial.println("D");
    return;
  } else {
    Serial.println("F");
    return;
  }
}

void sendPW() {
  Serial.println(password);
}
void viewDetails() {
  Serial.println("Brand: " + brand + "| Location: " + location + "| Price: " + price + "       ");
}
void pricePerLitre() {
  Serial.println(price);
}
void savePW(String data) {
  password = data.substring(14, data.length());
  writeFile(passwordPath, password);
}
void changePrice(String data) {
  price = data.substring(6, data.length());
  writeFile(pricePath, price);
}

void uploadData(String data) {

  if (WiFi.status() == WL_CONNECTED) {

    json.set("brand", brand);
    json.set("location", location);
    json.set("quantity", data.substring(1, data.length()));
    json.set("price", price);
    json.set("time", timeClient.getEpochTime());

    if (Firebase.push(firebaseData, "/data", json)) {
      // delete if sent
      if (fromFile) {
        deleteFile(fileName);
        fromFile = false;
      }

    } else if (!fromFile) {
      wentOff = true;
      for (byte i = 0; i < 50; i++) {
        if (!LittleFS.exists("/" + String(i) + ".txt")) {
          writeFile("/" + String(i) + ".txt", data);
          break;
        }
      }
    }
  }
}

void checkForLogs() {
  for (byte i = 0; i < 50; i++) {
    if (LittleFS.exists("/" + String(i) + ".txt")) {
      fileName = "/" + String(i) + ".txt";
      fromFile = true;
      uploadData(readFile(fileName));
    }
  }
}

void interpret(String data) {
  // this function will check the kind of data pass to it
  data.trim();
  // Serial.println(data);

  if (data.equals("PWDETAILS")) {
    sendPW();
  } else if (data.equals("GET_PRICE")) {
    pricePerLitre();
  } else if (data.startsWith("SAVEPWDETAILS:")) {
    savePW(data);
  } else if (data.startsWith("LITRE:")) {
    changePrice(data);
  } else if (data.startsWith(":")) {
    uploadData(data);
  } else if (data.equals("VIEW")) {
    viewDetails();
  }
}

void getData() {
  brand = readFile(brandNamePath);
  if (brand.equals("")) {
    writeFile(brandNamePath, "NODEX");
    brand = "NODEX";
  }

  location = readFile(location);
  if (location.equals("")) {
    writeFile(location, "WARRI");
    location = "WARRI";
  }

  password = readFile(passwordPath);
  if (password.equals("")) {
    writeFile(passwordPath, "0000");
    password = "0000";
  }

  price = readFile(pricePath);
  if (price.equals("")) {
    writeFile(pricePath, "500");
    price = "500";
  }

  if (LittleFS.exists(ssidPath)) {
    ssid = readFile(ssidPath);
    ssid.trim();
  }
  if (LittleFS.exists(ssidPassPath)) {
    ssidPass = readFile(ssidPassPath);
    ssidPass.trim();
  }
  //Serial.println("START");
}

void setup() {
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  LittleFS.begin();

  String r = "";

  while (1) {
    if (Serial.available()) {
      r = Serial.readStringUntil('\n');
      r.trim();
      break;
    }
  }

  if (r.equals("1")) {
    WiFi.softAP(ssid, ssidPass);

    server.on("/", handleRoot);
    server.on("/on", HTTP_POST, handleForm);
    server.onNotFound(handleNotFound);

    server.begin();
    while (true) {
      server.handleClient();
    }
  } else {
    getData();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, ssidPass);

    while (WiFi.status() != WL_CONNECTED) {
      delay(200);
    }

    digitalWrite(led, HIGH);
  }

  Firebase.begin(FIREBASE_HOST, FIREBASE_PROJECT_ID);
  Firebase.reconnectWiFi(true);

  timeClient.begin();
  timeClient.setTimeOffset(0);
}

void loop() {

  timeClient.update();
  if (WiFi.status() != WL_CONNECTED) {
    wentOff = true;
    digitalWrite(led, LOW);
  } else {
    digitalWrite(led, HIGH);
    if (wentOff) {
      checkForLogs();
      wentOff = false;
    }
  }



  if (Serial.available()) {
    interpret(Serial.readStringUntil('\n'));
  }
  delay(500);
}


void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}
void handleForm() {

  brand = server.arg("brandName");
  location = server.arg("location");
  ssid = server.arg("ssid");
  ssidPass = server.arg("password");

  brand.toUpperCase();
  location.toUpperCase();

  writeFile(ssidPath, ssid);
  writeFile(ssidPassPath, ssidPass);
  writeFile(brandNamePath, brand);
  writeFile(locationPath, location);

  server.send(200, "text/html", "Data received successfully.");
}

void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}