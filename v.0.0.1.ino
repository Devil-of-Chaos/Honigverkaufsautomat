#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Preferences.h>
#include <U8g2lib.h>
#include <MFRC522.h>
#include <EEPROM.h>
#include "ESP32_MailClient.h"

#define WLAN_SSID             "SSID"
#define WLAN_PASSWORD         "123456"
#define WLAN_MAX_RETRIES      5

#define EMAIL_SERVER          "mail.example.com"
#define EMAIL_PORT            465
#define EMAIL_USERNAME        "mail@example.com"
#define EMAIL_PASSWORD        "123456"
#define EMAIL_SENDER          "mail@example.com"
#define EMAIL_RECIPIENT       "mail@example.com"
#define EMAIL_SUBJECT         "Honigverkauf"


#define isDebug               1


#define VERSION               "0.0.1"
#define MODE_RUN              0
#define MODE_SETTINGS         1

// Menü anzahl der Punkte
#define MAIN_MENU_MAX         4
#define SUBJECT_MENU_MAX      3
#define RFID_MENU_MAX         3

// LEVEL invertiert wegen relais
#define BUTTON_SELECT_LEVEL   HIGH
#define LOCK_OPEN_LEVEL       LOW
#define LOCK_CLOSE_LEVEL      HIGH
#define LED_ON_LEVEL          LOW
#define LED_OFF_LEVEL         HIGH
#define COINAGE_ON_LEVEL      LOW
#define COINAGE_OFF_LEVEL     HIGH

// Taster
#define BUTTON_SUBJECT_1      34
#define BUTTON_SUBJECT_2      35
#define BUTTON_SUBJECT_3      36

// LED Beleuchtung Taster/Fach
#define LED_SUBJECT_1         12
#define LED_SUBJECT_2         13
#define LED_SUBJECT_3         14

// Schloss öffner
#define OPEN_LOCK_1           25
#define OPEN_LOCK_2           26
#define OPEN_LOCK_3           27

// Schloss Kontakt
#define CLOSE_LOCK_1          37
#define CLOSE_LOCK_2          38
#define CLOSE_LOCK_3          39

// Münzzähler
#define COINAGE_COUNTER       32
#define COINAGE_LOCK          33

// RFID
#define RFID_SCK              18
#define RFID_MISO             19
#define RFID_SDA              21
#define RFID_RST              22
#define RFID_MOSI             23


// OLED fuer Heltec WiFi Kit 32 (ESP32 onboard OLED)
U8G2_SSD1306_128X64_NONAME_F_SW_I2C u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 4, /* reset=*/ 16);
// RFID
MFRC522 mfrc522(RFID_SDA, RFID_RST);
MFRC522::MIFARE_Key key;

Preferences preferences;
SMTPData smtpData;

void sendCallback(SendStatus info);


int i_main;
int i_sub;
long checksum;
int menuPosition = 0;
int menuPositionMax = MAIN_MENU_MAX;
int currentMode = MODE_RUN;

int subject = 0;
int payed = 0;
int price = 0;
int retries = 0;

int currentImpuls = 0;

float priceSubject1;
float priceSubject2;
float priceSubject3;

int quantitySubject1;
int quantitySubject2;
int quantitySubject3;

int mastercardSet;
byte readcard[4];
byte mastercard[4];
byte card1[4];
byte card2[4];
byte card3[4];

uint8_t successRead;

void displayBoot(void) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB18_tf);
  u8g2.setCursor(25, 27);
  u8g2.print("BOOT");
  u8g2.setFont(u8g2_font_courB08_tf);
  u8g2.setCursor(25, 64);
  u8g2.print(VERSION);
  u8g2.sendBuffer();
}

void displayAllEmpty(void) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.setCursor(10, 10);
  u8g2.print("Alle");
  u8g2.setCursor(10, 23);
  u8g2.print("Fächer");
  u8g2.setCursor(10, 36);
  u8g2.print("leer");
  u8g2.sendBuffer();
}

void displayCloseSubject(int __subject) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.setCursor(10, 27);
  u8g2.print("Fach ");
  u8g2.print(__subject);
  u8g2.setCursor(10, 64);
  u8g2.print("schließen");
  u8g2.sendBuffer();
}

void displayChooseSubject(void) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB14_tf);
  u8g2.setCursor(10, 27);
  u8g2.print("Bitte Fach");
  u8g2.setCursor(10, 64);
  u8g2.print("wählen");
  u8g2.sendBuffer();
}

void displayOK(void) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.setCursor(10, 10);
  u8g2.print("OK");
  u8g2.sendBuffer();
}

void displayEmptySubject(void) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.setCursor(10, 10);
  u8g2.print("Fach ");
  u8g2.print(subject);
  u8g2.print(" leer");
  u8g2.setCursor(10, 23);
  u8g2.print("andere");
  u8g2.setCursor(10, 36);
  u8g2.print("Auswahl");
  u8g2.sendBuffer();
}

void displayBuyed(void) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.setCursor(10, 10);
  u8g2.print("Vielen");
  u8g2.setCursor(10, 23);
  u8g2.print("Dank");
  u8g2.setCursor(10, 36);
  u8g2.print("Guten");
  u8g2.setCursor(10, 49);
  u8g2.print("Appetit");
  u8g2.sendBuffer();
}

void displayPayment(int __price, int __payed) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.setCursor(10, 10);
  u8g2.print("Preis: ");
  u8g2.print((float) __price / 100);
  u8g2.print("€");
  u8g2.setCursor(10, 23);
  u8g2.print("noch");
  u8g2.setCursor(10, 36);
  u8g2.print((float) (__price - __payed) / 100);
  u8g2.setCursor(10, 49);
  u8g2.print("einwerfen");
  u8g2.sendBuffer();
}

void displaySetupInt(int __display) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB18_tf);
  u8g2.setCursor(25, 27);
  u8g2.print(__display);
  u8g2.sendBuffer();
}

void displaySetupFloat(float __display) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB18_tf);
  u8g2.setCursor(25, 27);
  u8g2.print(__display);
  u8g2.sendBuffer();
}

void displayMainMenu(int __display) {
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.clearBuffer();
  u8g2.setCursor(10, 10);
  u8g2.print("Fach 1");
  u8g2.setCursor(10, 23);
  u8g2.print("Fach 2");
  u8g2.setCursor(10, 36);
  u8g2.print("Fach 3");
  u8g2.setCursor(10, 49);
  u8g2.print("RFID");
  u8g2.setCursor(10, 62);
  u8g2.print("Beenden");
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.setCursor(0, 10 + __display * 13);
  u8g2.print("*");
  u8g2.sendBuffer();
}

void displaySubjectMenu(int __display) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.setCursor(10, 10);
  u8g2.print("Preis");
  u8g2.setCursor(10, 23);
  u8g2.print("Bestand");
  u8g2.setCursor(10, 36);
  u8g2.print("Öffnen");
  u8g2.setCursor(10, 49);
  u8g2.print("Beenden");
  u8g2.setCursor(0, 10 + __display * 13);
  u8g2.print("*");
  u8g2.sendBuffer();
}

void displayRFIDMenu(int __display) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.setCursor(10, 10);
  u8g2.print("Karte 1");
  u8g2.setCursor(10, 23);
  u8g2.print("Karte 2");
  u8g2.setCursor(10, 36);
  u8g2.print("Karte 3");
  u8g2.setCursor(10, 49);
  u8g2.print("Beenden");

  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.setCursor(0, 10 + __display * 13);
  u8g2.print("*");
  u8g2.sendBuffer();
}

void displayShowRFID(void) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB10_tf);
  u8g2.setCursor(25, 27);
  u8g2.print("Karte");
  u8g2.setCursor(25, 64);
  u8g2.print("vorhalten");
  u8g2.sendBuffer();
}

void displayRFIDMaster(void) {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_courB08_tf);
  u8g2.setCursor(10, 10);
  u8g2.print("Ersteinrichtung");
  u8g2.setCursor(10, 36);
  u8g2.print("Mastercard");
  u8g2.setCursor(10, 49);
  u8g2.print("vorhalten");
  u8g2.sendBuffer();
}

void processRun(void) {

  payed = 0;
  subject = 0;

  if (quantitySubject1 <= 0 && quantitySubject2 <= 0 && quantitySubject3 <= 0) {
    displayAllEmpty();
    digitalWrite(LED_SUBJECT_1, LED_OFF_LEVEL);
    digitalWrite(LED_SUBJECT_2, LED_OFF_LEVEL);
    digitalWrite(LED_SUBJECT_3, LED_OFF_LEVEL);
    digitalWrite(OPEN_LOCK_1, LOCK_CLOSE_LEVEL);
    digitalWrite(OPEN_LOCK_2, LOCK_CLOSE_LEVEL);
    digitalWrite(OPEN_LOCK_3, LOCK_CLOSE_LEVEL);
    subject = 0;
    do {
      successRead = getID();
    }
    while (!successRead);
    if (readcard[4] == mastercard[4] || readcard[4] == card1[4] || readcard[4] == card2[4] || readcard[4] == card3[4]) {
      currentMode = MODE_SETTINGS;
    }
    delay(1000);
    successRead = 0;

  } else {
    if (digitalRead(CLOSE_LOCK_1) == LOCK_OPEN_LEVEL) {
      displayCloseSubject(1);
      subject = 0;
    } else if (digitalRead(CLOSE_LOCK_2) == LOCK_OPEN_LEVEL) {
      displayCloseSubject(2);
      subject = 0;
    } else if (digitalRead(CLOSE_LOCK_3) == LOCK_OPEN_LEVEL) {
      displayCloseSubject(3);
      subject = 0;
    } else {
      subject = 0;
      displayChooseSubject();
      if (digitalRead(BUTTON_SUBJECT_1) == BUTTON_SELECT_LEVEL) {
        delay(250);
        while (digitalRead(BUTTON_SUBJECT_1) == BUTTON_SELECT_LEVEL) {
        }
        subject = 1;
#ifdef isDebug
        Serial.println("Fach: 1");
#endif
      }

      if (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
        delay(250);
        while (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
        }
        subject = 2;
#ifdef isDebug
        Serial.println("Fach: 2");
#endif
      }

      if (digitalRead(BUTTON_SUBJECT_3) == BUTTON_SELECT_LEVEL) {
        delay(250);
        while (digitalRead(BUTTON_SUBJECT_3) == BUTTON_SELECT_LEVEL) {
        }
        subject = 3;
#ifdef isDebug
        Serial.println("Fach: 3");
#endif
      }

      if (subject != 0) {
        doPayment();
      }

    }
  }
}



void doPayment(void) {
  int quantity = 0;

  digitalWrite(LED_SUBJECT_1, LED_OFF_LEVEL);
  digitalWrite(LED_SUBJECT_2, LED_OFF_LEVEL);
  digitalWrite(LED_SUBJECT_3, LED_OFF_LEVEL);

  switch (subject) {
    case 1:
      quantity = quantitySubject1;
      price = priceSubject1 * 100;
      digitalWrite(LED_SUBJECT_1, LED_ON_LEVEL);
      break;
    case 2:
      quantity = quantitySubject2;
      price = priceSubject2 * 100;
      digitalWrite(LED_SUBJECT_2, LED_ON_LEVEL);
      break;
    case 3:
      quantity = quantitySubject3;
      price = priceSubject3 * 100;
      digitalWrite(LED_SUBJECT_3, LED_ON_LEVEL);
      break;
  }

  if (quantity <= 0) {
    displayEmptySubject();
    subject = 0;
    delay(2000);
  } else {
    if (digitalRead(COINAGE_LOCK) == COINAGE_OFF_LEVEL){
      digitalWrite(COINAGE_LOCK, COINAGE_ON_LEVEL);
      delay(30);
    }
    while (payed < price) {
      if (digitalRead(COINAGE_COUNTER) == HIGH){
        if (!currentImpuls){
          payed = payed + 50;
          currentImpuls = 1;
        }
      } else {
        currentImpuls = 0;
      }
    
      displayPayment(price, payed);

      if (digitalRead(BUTTON_SUBJECT_1) == BUTTON_SELECT_LEVEL) {
        delay(250);
        while (digitalRead(BUTTON_SUBJECT_1) == BUTTON_SELECT_LEVEL) {
        }
        subject = 1;
        price = priceSubject1 * 100;
        digitalWrite(LED_SUBJECT_1, LED_OFF_LEVEL);
        digitalWrite(LED_SUBJECT_2, LED_OFF_LEVEL);
        digitalWrite(LED_SUBJECT_3, LED_OFF_LEVEL);
        digitalWrite(LED_SUBJECT_1, LED_ON_LEVEL);
      }

      if (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
        delay(250);
        while (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
        }
        subject = 2;
        price = priceSubject2 * 100;
        digitalWrite(LED_SUBJECT_1, LED_OFF_LEVEL);
        digitalWrite(LED_SUBJECT_2, LED_OFF_LEVEL);
        digitalWrite(LED_SUBJECT_3, LED_OFF_LEVEL);
        digitalWrite(LED_SUBJECT_2, LED_ON_LEVEL);
      }

      if (digitalRead(BUTTON_SUBJECT_3) == BUTTON_SELECT_LEVEL) {
        delay(250);
        while (digitalRead(BUTTON_SUBJECT_3) == BUTTON_SELECT_LEVEL) {
        }
        subject = 3;
        price = priceSubject3 * 100;
        digitalWrite(LED_SUBJECT_1, LED_OFF_LEVEL);
        digitalWrite(LED_SUBJECT_2, LED_OFF_LEVEL);
        digitalWrite(LED_SUBJECT_3, LED_OFF_LEVEL);
        digitalWrite(LED_SUBJECT_3, LED_ON_LEVEL);
      }
    }

    if (payed >= price) {
      digitalWrite(COINAGE_LOCK, COINAGE_OFF_LEVEL);

      switch (subject) {
        case 1:
          digitalWrite(OPEN_LOCK_1, LOCK_OPEN_LEVEL);
          delay(200);
          digitalWrite(OPEN_LOCK_1, LOCK_CLOSE_LEVEL);
          digitalWrite(LED_SUBJECT_1, LED_OFF_LEVEL);
          quantitySubject1--;
          setPreferences();
          sendEmail();
          break;
        case 2:
          digitalWrite(OPEN_LOCK_2, LOCK_OPEN_LEVEL);
          delay(200);
          digitalWrite(OPEN_LOCK_2, LOCK_CLOSE_LEVEL);
          digitalWrite(LED_SUBJECT_2, LED_OFF_LEVEL);
          quantitySubject2--;
          setPreferences();
          sendEmail();
          break;
        case 3:
          digitalWrite(OPEN_LOCK_3, LOCK_OPEN_LEVEL);
          delay(200);
          digitalWrite(OPEN_LOCK_3, LOCK_CLOSE_LEVEL);
          digitalWrite(LED_SUBJECT_3, LED_OFF_LEVEL);
          quantitySubject3--;
          setPreferences();
          sendEmail();
          break;
      }
      displayBuyed();
      delay(3000);
      payed = 0;
      subject = 0;
    }
  }
  if (subject == 0) exit;
}
void setupRFID(void) {
  menuPositionMax = RFID_MENU_MAX;
  i_sub = 1;
  while (i_sub > 0) {
    getMenuPosition();

    if (mastercardSet != 143) {
      displayRFIDMaster();
      do {
        successRead = getID();
      }
      while (!successRead);
      for (uint8_t j = 0; j < 4; j++) {
        mastercard[j] = readcard[j];
      }
      mastercardSet = 143;
      successRead = 0;
      setPreferences();
    } else {
      displayRFIDMenu(menuPosition);
      if (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
        delay(250);
        while (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
        }
#ifdef isDebug
        Serial.print("Setup Position:");
        Serial.println(menuPosition);
#endif

        if (menuPosition != 3) {
          displayShowRFID();
        }

        switch (menuPosition) {
          case 0:
            do {
              successRead = getID();
            }
            while (!successRead);
            for (uint8_t j = 0; j < 4; j++) {
              card1[j] = readcard[j];
            }
            successRead = 0;
            break;
          case 1:
            do {
              successRead = getID();
            }
            while (!successRead);
            for (uint8_t j = 0; j < 4; j++) {
              card2[j] = readcard[j];
            }
            successRead = 0;
            break;
          case 2:
            do {
              successRead = getID();
            }
            while (!successRead);
            for (uint8_t j = 0; j < 4; j++) {
              card3[j] = readcard[j];
            }
            successRead = 0;
            break;
          case 3:
            menuPosition = 0;
            menuPositionMax = MAIN_MENU_MAX;
            subject = 0;
            setPreferences();
            displayOK();
            delay(1000);
            i_sub = 0;
            break;
        }
      }
    }
  }
}

uint8_t getID() {
  if (!mfrc522.PICC_IsNewCardPresent()) return 0;
  if (!mfrc522.PICC_ReadCardSerial()) return 0;
#ifdef isDebug
  Serial.println(F("Scanned PICC's UID:"));
#endif
  for (uint8_t j = 0; j < 4; j++) {
    readcard[j] = mfrc522.uid.uidByte[j];
#ifdef isDebug
    Serial.print(readcard[j], HEX);
#endif
  }
#ifdef isDebug
  Serial.println("");
#endif
  mfrc522.PICC_HaltA();
  return 1;
}

void processSetup(void) {
  if (currentMode != MODE_SETTINGS) {
    currentMode = MODE_SETTINGS;
    menuPosition = 0;
    menuPositionMax = MAIN_MENU_MAX;
    subject = 0;
  }

  digitalWrite(LED_SUBJECT_1, LED_OFF_LEVEL);
  digitalWrite(LED_SUBJECT_2, LED_OFF_LEVEL);
  digitalWrite(LED_SUBJECT_3, LED_OFF_LEVEL);

  getMenuPosition();
  displayMainMenu(menuPosition);

  if (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
    delay(250);
    while (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
    }
#ifdef isDebug
    Serial.print("Setup Position:");
    Serial.println(menuPosition);
#endif

    if (menuPosition == 3) {
      getPreferences();
      menuPosition = 0;
      menuPositionMax = MAIN_MENU_MAX;
      subject = 0;
      setupRFID();
    } else if (menuPosition < menuPositionMax) {
      subject = menuPosition + 1;
      menuPosition = 0;
      menuPositionMax = SUBJECT_MENU_MAX;
      setupSubject();
    } else {
      getPreferences();
      currentMode = MODE_RUN;
      menuPosition = 0;
      menuPositionMax = MAIN_MENU_MAX;
      subject = 0;
    }
  }
}

void setupSubject() {

  menuPositionMax = SUBJECT_MENU_MAX;
  switch (subject) {
    case 1:
      digitalWrite(LED_SUBJECT_1, LED_ON_LEVEL);
      break;
    case 2:
      digitalWrite(LED_SUBJECT_2, LED_ON_LEVEL);
      break;
    case 3:
      digitalWrite(LED_SUBJECT_3, LED_ON_LEVEL);
      break;
  }

  i_main = 1;
  while (i_main > 0) {

    getMenuPosition();
    displaySubjectMenu(menuPosition);

    if (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
      delay(250);
      while (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
      }
#ifdef isDebug
      Serial.print("Setup Position:");
      Serial.println(menuPosition);
#endif

      if (menuPosition == 0) {
        setupPrice();
      }
      if (menuPosition == 1) {
        setupQuantity();
      }
      if (menuPosition == 2) {
        switch (subject) {
          case 1:
            digitalWrite(LED_SUBJECT_1, LED_ON_LEVEL);
            digitalWrite(OPEN_LOCK_1, LOCK_OPEN_LEVEL);
            delay(500);
            digitalWrite(LED_SUBJECT_1, LED_OFF_LEVEL);
            digitalWrite(OPEN_LOCK_1, LOCK_CLOSE_LEVEL);
            break;
          case 2:
            digitalWrite(LED_SUBJECT_2, LED_ON_LEVEL);
            digitalWrite(OPEN_LOCK_2, LOCK_OPEN_LEVEL);
            delay(500);
            digitalWrite(LED_SUBJECT_2, LED_OFF_LEVEL);
            digitalWrite(OPEN_LOCK_2, LOCK_CLOSE_LEVEL);
            break;
          case 3:
            digitalWrite(LED_SUBJECT_3, LED_ON_LEVEL);
            digitalWrite(OPEN_LOCK_3, LOCK_OPEN_LEVEL);
            delay(500);
            digitalWrite(LED_SUBJECT_3, LED_OFF_LEVEL);
            digitalWrite(OPEN_LOCK_3, LOCK_CLOSE_LEVEL);
            break;
        }
      }
      if (menuPosition == 3) {
        menuPosition = 0;
        menuPositionMax = MAIN_MENU_MAX;
        subject = 0;
        displayOK();
        delay(1000);
        i_main = 0;
      }
    }

  }
}

void setupPrice(void) {
  float displayPosition = 0.0;
  menuPositionMax = 999;
  switch (subject) {
    case 1:
      menuPosition = priceSubject1 * 10;
      break;
    case 2:
      menuPosition = priceSubject2 * 10;
      break;
    case 3:
      menuPosition = priceSubject3 * 10;
      break;
  }

  i_sub = 1;
  while (i_sub > 0) {

    getMenuPosition();

    displayPosition = (float) menuPosition / 10;
    displaySetupFloat(displayPosition);

    if (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
      delay(250);
      while (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
      }
#ifdef isDebug
      Serial.print("Setup Position:");
      Serial.println(menuPosition);
#endif

      switch (subject) {
        case 1:
          priceSubject1 = (float) menuPosition / 10;
          break;
        case 2:
          priceSubject2 = (float) menuPosition / 10;
          break;
        case 3:
          priceSubject3 = (float) menuPosition / 10;
          break;
      }
      setPreferences();
      menuPosition = 0;
      menuPositionMax = SUBJECT_MENU_MAX;
      displayOK();
      delay(1000);
      i_sub = 0;
    }
  }
}

void setupQuantity(void) {
  menuPositionMax = 99;
  switch (subject) {
    case 1:
      menuPosition = quantitySubject1;
      break;
    case 2:
      menuPosition = quantitySubject2;
      break;
    case 3:
      menuPosition = quantitySubject3;
      break;
  }

  i_sub = 1;
  while (i_sub > 0) {

    getMenuPosition();
    displaySetupInt(menuPosition);

    if (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
      delay(250);
      while (digitalRead(BUTTON_SUBJECT_2) == BUTTON_SELECT_LEVEL) {
      }
#ifdef isDebug
      Serial.print("Setup Position:");
      Serial.println(menuPosition);
#endif

      switch (subject) {
        case 1:
          quantitySubject1 = menuPosition;
          break;
        case 2:
          quantitySubject2 = menuPosition;
          break;
        case 3:
          quantitySubject3 = menuPosition;
          break;
      }
      setPreferences();
      menuPosition = 0;
      menuPositionMax = SUBJECT_MENU_MAX;
      displayOK();
      delay(1000);
      i_sub = 0;
    }
  }
}

void setup() {

  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial) {
  }

#ifdef isDebug
  Serial.println("Start");
#endif

  SPI.begin();
  // Init MFRC522
  mfrc522.PCD_Init();

  // interne pull downs Taser
  pinMode(BUTTON_SUBJECT_1, INPUT);
  pinMode(BUTTON_SUBJECT_2, INPUT);
  pinMode(BUTTON_SUBJECT_3, INPUT);

  // LED output
  pinMode(LED_SUBJECT_1, OUTPUT);
  pinMode(LED_SUBJECT_2, OUTPUT);
  pinMode(LED_SUBJECT_3, OUTPUT);
  digitalWrite(LED_SUBJECT_1, LED_OFF_LEVEL);
  digitalWrite(LED_SUBJECT_2, LED_OFF_LEVEL);
  digitalWrite(LED_SUBJECT_3, LED_OFF_LEVEL);

  // open lock output
  pinMode(OPEN_LOCK_1, OUTPUT);
  pinMode(OPEN_LOCK_2, OUTPUT);
  pinMode(OPEN_LOCK_3, OUTPUT);
  digitalWrite(OPEN_LOCK_1, LOCK_CLOSE_LEVEL);
  digitalWrite(OPEN_LOCK_2, LOCK_CLOSE_LEVEL);
  digitalWrite(OPEN_LOCK_3, LOCK_CLOSE_LEVEL);

  // interne pull downs close lock
  pinMode(CLOSE_LOCK_1, INPUT);
  pinMode(CLOSE_LOCK_2, INPUT);
  pinMode(CLOSE_LOCK_3, INPUT);

  // Münzzähler
  pinMode(COINAGE_COUNTER, INPUT);
  pinMode(COINAGE_LOCK, OUTPUT);
  digitalWrite(COINAGE_LOCK, COINAGE_OFF_LEVEL);

  delay(100);

  // LOAD SETTINGS
  getPreferences();

  WiFi.begin(WLAN_SSID, WLAN_PASSWORD);
  while (WiFi.status() != WL_CONNECTED && WLAN_MAX_RETRIES > retries) {
    Serial.print(".");
    retries++;
    delay(200);
  }

  // Boot Screen
  u8g2.begin();
  u8g2.enableUTF8Print();

  displayBoot();
  delay(2000);
}

void loop() {
  if (mastercardSet != 143) {
    currentMode = MODE_SETTINGS;
  }
  if (currentMode == MODE_SETTINGS) {
    processSetup();
  }
  if (currentMode == MODE_RUN) {
    processRun();
  }
}



void getMenuPosition(void) {

  if (digitalRead(BUTTON_SUBJECT_1) == BUTTON_SELECT_LEVEL) {
    delay(250);
    while (digitalRead(BUTTON_SUBJECT_1) == BUTTON_SELECT_LEVEL) {
    }
    if (menuPosition == 1) menuPosition = menuPositionMax;
    else if (menuPosition > 1) menuPosition--;
#ifdef isDebug
    Serial.print("Menu Position:");
    Serial.println(menuPosition);
#endif
  }

  if (digitalRead(BUTTON_SUBJECT_3) == BUTTON_SELECT_LEVEL) {
    delay(250);
    while (digitalRead(BUTTON_SUBJECT_3) == BUTTON_SELECT_LEVEL) {
    }
    if (menuPosition == menuPositionMax) menuPosition = 0;
    else if (menuPosition < menuPositionMax) menuPosition++;
#ifdef isDebug
    Serial.print("Menu Position:");
    Serial.println(menuPosition);
#endif
  }
}

void getPreferences(void) {
  preferences.begin("EEPROM", false);

  priceSubject1     = preferences.getFloat("priceSubject1", 0.0);
  priceSubject2     = preferences.getFloat("priceSubject2", 0.0);
  priceSubject3     = preferences.getFloat("priceSubject3", 0.0);
  quantitySubject1  = preferences.getInt("quantity1", 0);
  quantitySubject2  = preferences.getInt("quantity2", 0);
  quantitySubject3  = preferences.getInt("quantity3", 0);
  mastercardSet     = preferences.getInt("mastercardSet", 0);
  for (uint8_t j = 0; j < 4; j++) {
    mastercard[j]   = preferences.getInt("mastercard_" + j, 0);
    card1[j]        = preferences.getInt("card1_" + j, 0);
    card2[j]        = preferences.getInt("card2_+j", 0);
    card3[j]        = preferences.getInt("card3_+j", 0);
  }

  checksum = priceSubject1 + priceSubject2 + priceSubject3 + quantitySubject1 + quantitySubject2 + quantitySubject3 + mastercardSet;

  preferences.end();

#ifdef isDebug
  Serial.println("Read Preferences:");
  Serial.print("Price Subject 1:");
  Serial.print(priceSubject1);
  Serial.println("Price Subject 2:");
  Serial.print(priceSubject2);
  Serial.println("Price Subject 3:");
  Serial.print(priceSubject3);
  Serial.println("Quantity Subject 1:");
  Serial.print(quantitySubject1);
  Serial.println("Quantity Subject 2:");
  Serial.print(quantitySubject2);
  Serial.println("Quantity Subject 3:");
  Serial.print(quantitySubject3);
  Serial.println("Mastercard:");
  Serial.print(mastercardSet);
#endif
}

void setPreferences(void) {
  long checksum_check;
  checksum_check = priceSubject1 + priceSubject2 + priceSubject3 + quantitySubject1 + quantitySubject2 + quantitySubject3 + mastercardSet;

  if (checksum == checksum_check) {
#ifdef isDebug
    Serial.println("Preferences unverändert");
#endif
    return;
  }

  checksum = checksum_check;
  preferences.begin("EEPROM", false);

  preferences.putFloat("price1", priceSubject1);
  preferences.putFloat("price2", priceSubject2);
  preferences.putFloat("price3", priceSubject3);
  preferences.putInt("quantity1", quantitySubject1);
  preferences.putInt("quantity2", quantitySubject2);
  preferences.putInt("quantity3", quantitySubject3);

  preferences.putInt("mastercardSet", mastercardSet);
  for (uint8_t j = 0; j < 4; j++) {
    preferences.putInt("mastercard_" + j, mastercard[j]);
    preferences.putInt("card1_" + j, card1[j]);
    preferences.putInt("card2_" + j, card2[j]);
    preferences.putInt("card3_" + j, card3[j]);
  }

  preferences.end();

#ifdef isDebug
  Serial.println("Save Preferences:");
  Serial.println("Price Subject 1:");
  Serial.print(priceSubject1);
  Serial.println("Price Subject 2:");
  Serial.print(priceSubject2);
  Serial.println("Price Subject 3:");
  Serial.print(priceSubject3);
  Serial.println("Quantity Subject 1:");
  Serial.print(quantitySubject1);
  Serial.println("Quantity Subject 2:");
  Serial.print(quantitySubject2);
  Serial.println("Quantity Subject 3:");
  Serial.print(quantitySubject3);
  Serial.println("Mastercard: ");
  Serial.print(mastercardSet);
#endif
}

void sendEmail(void) {
  if (WiFi.status() != WL_CONNECTED) {
    retries = 0;
    WiFi.begin(WLAN_SSID, WLAN_PASSWORD);
    while (WiFi.status() != WL_CONNECTED && WLAN_MAX_RETRIES > retries) {
      Serial.print(".");
      retries++;
      delay(200);
    }
    if (WiFi.status() == WL_CONNECTED) {
      smtpData.setLogin(EMAIL_SERVER, EMAIL_PORT, EMAIL_USERNAME, EMAIL_PASSWORD);
      smtpData.setSTARTTLS(true);
      smtpData.setSender("Honigautomat", EMAIL_SENDER);
      smtpData.setPriority("High");
      smtpData.setSubject(EMAIL_SUBJECT);
      smtpData.setMessage("Honigglasverkauf", false);
      smtpData.addRecipient(EMAIL_RECIPIENT);
      if (!MailClient.sendMail(smtpData)) Serial.println("Error sending Email, " + MailClient.smtpErrorReason());
      smtpData.empty();
    }
  }
}
