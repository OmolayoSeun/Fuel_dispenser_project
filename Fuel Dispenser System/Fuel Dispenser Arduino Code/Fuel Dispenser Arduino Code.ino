#include <Keypad.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <PN532_SWHSU.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

#define calib 5260
#define relay 4


PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

short perLit = 620;

char keys[4][4] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte pin_rows[4] = { 5, 6, 7, 8 };       //Row pins
byte pin_column[4] = { 9, 10, 11, 12 };  //Column pins

Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, 4, 4);
LiquidCrystal_I2C lcd(0x27, 16, 2);

int cursorColumn = 0;
int flowPin = 2;
double flowRate;
volatile long count;
int counter = 0;
const char* menu[] = {
  "1. Change Pw",
  "2. View details",
  "3. Set Brand",
  "4. set location",
  "5. Set litre",
};


char key;
String reader;

void setup() {

  Serial.begin(9600);
  

  //nfc.begin();

  pinMode(flowPin, INPUT);
  pinMode(relay, OUTPUT);
  pinMode(8, INPUT);

  Serial.println(digitalRead(8));
  if(digitalRead(8)){

    while(1);
  }

  lcd.init();
  lcd.backlight();

  attachInterrupt(0, Flow, RISING);

  display();

  Serial.println("GET_PRICE");
  while (1) {
    if (Serial.available()) {
      reader = Serial.readStringUntil("\n");
      reader.trim();

      perLit = reader.toInt();

      break;
    }
  }
}

void Flow() {
  count++;
}

void display() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Fuel Dispenser");
  lcd.setCursor(4, 1);
  lcd.print("By NodeX");
}

void changePw() {
  Serial.println("PWDETAILS");

  while (true) {
    if (Serial.available()) {
      reader = Serial.readStringUntil("\n");
      reader.trim();
      break;
    }
  }

  lcd.clear();
  lcd.print("Old PW: ");

  if (reader != getValue()) {
    lcd.clear();
    lcd.print("Wrong Password!");
    delay(1000);
    return;
  }

  lcd.clear();
  lcd.print("New PW: ");
  reader = getValue();

  lcd.clear();
  lcd.print("Con PW: ");
  if (reader != getValue()) {
    lcd.clear();
    lcd.print("Pw don't match!");
    delay(1000);
    return;
  }

  Serial.println("SAVEPWDETAILS:" + reader);

  while (true) {
    if (Serial.available()) {
      if (Serial.read() == 'Y') {
        lcd.clear();
        lcd.print("Successfull");
        delay(1000);
        return;
      } else {
        lcd.clear();
        lcd.print("Failed!");
        delay(1000);
        return;
      }
    }
  }
}
void viewDetails() {
  reader = "";
  Serial.println("VIEW");

  while (true) {
    if (Serial.available()) {
      reader = Serial.readStringUntil("\n");
      break;
    }
  }

  lcd.clear();
  lcd.print(reader);

  while (true) {
    key = keypad.getKey();

    switch (key) {
      case 'A':
        lcd.scrollDisplayRight();
        break;
      case 'B':
        lcd.scrollDisplayLeft();
        break;
      case 'C':
        return;
    }
  }
}
void setBrand() {

  lcd.clear();
  lcd.print("Brand:  ");
}
void setLocation() {}
void setLitre() {

  lcd.clear();
  lcd.print("Litre: #");
  reader = getValue();

  Serial.println("LITRE:" + reader);

  while (true) {
    if (Serial.available()) {
      if (Serial.read() == 'Y') {
        lcd.clear();
        lcd.print("Successfull");
        perLit = reader.toInt();
        delay(1000);
        return;
      } else {
        lcd.clear();
        lcd.print("Failed!");
        delay(1000);
        return;
      }
    }
  }
}

void security() {

  Serial.println("PWDETAILS");

  while (true) {
    if (Serial.available()) {
      reader = Serial.readStringUntil("\n");
      reader.trim();
      break;
    }
  }

  lcd.clear();
  lcd.print("Ent PW: ");
  if (reader != getValue()) {
    lcd.clear();
    lcd.print("Wrong Password!");
    delay(1000);
    display();
    return;
  }
  counter = 0;
  setting();
  display();
}
void setting() {

  lcd.clear();
  lcd.print(menu[counter]);

  lcd.setCursor(0, 1);
  lcd.print(menu[counter + 1]);
  while (true) {
    key = keypad.getKey();

    switch (key) {
      case 'A':
        if (counter != 0) {
          counter--;
          setting();
          return;
        }
        break;
      case 'B':
        if (counter != 4) {
          counter++;
          setting();
          return;
        }
        break;
      case 'C':
        display();
        return;
      case '1':
        changePw();
        setting();
        return;
      case '2':
        viewDetails();
        setting();
        return;
      case '3':
        setBrand();
        setting();
        return;
      case '4':
        setLocation();
        setting();
        return;
      case '5':
        setLitre();
        setting();
        return;
    }
  }
}

void keypadAction(char key) {

  switch (key) {
    case 'D':
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Litres");

      lcd.setCursor(8, 0);
      lcd.print("Amount");

      lcd.setCursor(2, 1);
      lcd.print("A");

      lcd.setCursor(11, 1);
      lcd.print("B");

      bool a = false;
      while (true) {
        key = keypad.getKey();

        switch (key) {
          case 'A':
            inLitres();
            a = true;
            break;
          case 'B':
            inAmount();
            a = true;
            break;
          case 'C':
            display();
            return;
        }

        if (a) break;
        delay(50);
      }
  }
}

void inLitres() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Litre:  ");

  lcd.setCursor(0, 1);
  lcd.print("1 Lit:   #");


  lcd.setCursor(9, 1);
  lcd.print(perLit);

  // from here the operation will comntune further to dispense the fuel

  float litres = atof(getValue().c_str());

  float amount = float(litres * perLit);
  pump(float(litres * calib));

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Amt:   #");
  lcd.setCursor(8, 0);
  lcd.print(amount);

  lcd.setCursor(0, 1);
  lcd.print("Lit:  ");
  lcd.setCursor(7, 1);
  lcd.print(litres);

  Serial.println(":" + String(litres));
}

void inAmount() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Amt:   #");

  lcd.setCursor(0, 1);
  lcd.print("1 Lit:   #");

  lcd.setCursor(9, 1);
  lcd.print(perLit);

  // from here the operation will comntune further to dispense the fuel
  float amount = atof(getValue().c_str());
  float litres = amount / perLit;
  pump(litres * calib);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Amt:   #");
  lcd.setCursor(8, 0);
  lcd.print(amount);

  lcd.setCursor(0, 1);
  lcd.print("Lit:  ");
  lcd.setCursor(7, 1);
  lcd.print(litres);

  Serial.println(":" + String(litres));
}


//float temp;
void pump(long pulse) {
  Serial.println(pulse);
  lcd.clear();
  digitalWrite(relay, HIGH);
  count = 0;
  interrupts();
  while (1) {
    delay(50);
    if (count >= pulse) break;

    //temp = (count / 5880.0) * 620.00;
    
    lcd.setCursor(0, 0);
    lcd.print("Amt:   #");
    lcd.setCursor(8, 0);
    lcd.print(((float)count / calib) * (float)perLit);

    lcd.setCursor(0, 1);
    lcd.print("Lit:  ");
    lcd.setCursor(7, 1);
    lcd.print((float)count / calib);

    // lcd.clear();
    // lcd.setCursor(0, 0);
    // lcd.print(count);
  }
  //noInterrupts();
  digitalWrite(relay, LOW);
  count = 0;
}
String getValue() {
  String value = "";
  byte pos = 8;

  while (true) {
    key = keypad.getKey();
    switch (key) {
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
      case '0':
        value += key;
        lcd.setCursor(pos, 0);
        lcd.print(key);
        pos++;
        break;
      case '*':
        value += '.';
        lcd.setCursor(pos, 0);
        lcd.print('.');
        pos++;
        break;
      case '#':
        if (pos > 8) {
          value = value.substring(0, value.length() - 1);
          pos--;
          lcd.setCursor(pos, 0);
          lcd.print(' ');
        }
        break;
      case 'D':
        return value;
    }
  }
}

void loop() {
  key = keypad.getKey();

  if (key) {

    keypadAction(key);
    if (key == '*') {
      counter++;
      if (counter == 3) {
        counter = 0;
        security();
      }
    }
  }
  // if (nfc.tagPresent()) {
  //   String UID = "";
  //   NfcTag tag = nfc.read();
  //   UID = tag.getUidString();
  //   Serial.println(UID);
  //   Serial.println("Dis");
  // }

  delay(100);
}