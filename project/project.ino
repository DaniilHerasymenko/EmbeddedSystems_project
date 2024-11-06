#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <vector>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define SS_PIN 5
#define RST_PIN 22
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Створюємо екземпляр MFRC522

// Налаштування WiFi
const char* ssid = "";//додати свої
const char* password = "";//додати свої
const char* serverUrl = "http://178.212.99.155:3000/endpoint"; // Заміни на адресу вашого сервера

// Піни для світлодіодів
const int GREEN_LED = 2;
const int RED_LED = 4;

// Структура для зберігання UID та часу
struct StudentRecord {
  String uid;
  unsigned long time; // Час, коли зчитано мітку
};

// Динамічний буфер для зберігання зчитаних міток з поточним часом
std::vector<StudentRecord> buffer;
String macAddress = "";

// Потік для зчитування RFID міток
void readRFID(void *pvParameters) {  // Додаємо параметр void*
  while (true) {
    // Перевірка наявності нової картки
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
      delay(50);
      continue;
    }

    // Зчитування UID мітки
    String uidStr = "";
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      uidStr.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : ""));
      uidStr.concat(String(mfrc522.uid.uidByte[i], HEX));
    }
    uidStr.toUpperCase();

    // Створюємо запис та зберігаємо його у буфер
    StudentRecord record;
    record.uid = uidStr;
    record.time = millis();
    buffer.push_back(record);

    // Виведення UID та часу
    Serial.print("UID мітки: ");
    Serial.println(record.uid);
    Serial.print("Час зчитування (мс): ");
    Serial.println(record.time);

    delay(3000); // Затримка перед наступним зчитуванням
  }
}

// Потік для відправки даних на сервер
void sendDataToServer(void *pvParameters) {  // Додаємо параметр void*
  while (true) {
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      http.begin(serverUrl); // Вказуємо URL сервера
      http.addHeader("Content-Type", "application/json");

      if (!buffer.empty()) {
        // Формуємо JSON для передачі
//        String jsonData = "{\"uid\":\"" + buffer.front().uid + "\",\"time\":" + String(buffer.front().time) + "}";
        String jsonData = "{\"mac\":\"" + macAddress + "\",\"uid\":\"" + buffer.front().uid + "\",\"time\":" + String(buffer.front().time) + "}";

        int httpResponseCode = http.POST(jsonData);

        if (httpResponseCode == 200) { // Якщо дані успішно відправлено
          Serial.println("Дані успішно відправлено");
          buffer.erase(buffer.begin()); // Видаляємо перший елемент з буфера
        } else {
          Serial.print("Помилка відправки, код відповіді: ");
          Serial.println(httpResponseCode);
        }

        http.end();
      }
    } else {
      Serial.println("WiFi не підключено, чекаємо на підключення...");
      WiFi.begin(ssid, password);
      delay(3000); // Затримка перед наступною спробою
    }

//      // Виведення всього буфера
//  Serial.println("\nBUFFER:");
//  for (int i = 0; i < buffer.size(); i++) {
//    Serial.print("UID: "); 
//    Serial.print(buffer[i].uid); 
//    Serial.print(", Time: "); 
//    Serial.println(buffer[i].time);
//  }
//  Serial.println("-------------------------");

  
  }
  
}

void setup() {
  Serial.begin(9600); // Початок послідовного зв'язку
  
  SPI.begin();         // Початок роботи SPI
  mfrc522.PCD_Init();  // Ініціалізація MFRC522

  WiFi.begin(ssid, password);

  

  // Підключення до WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Підключення до WiFi...");
  }

  macAddress = WiFi.macAddress();
  Serial.print("MAC-адреса ESP32: ");
  Serial.println(macAddress);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  delay(1000);
  digitalWrite(2, LOW);

  Serial.println("Підключено до WiFi");

  // Створення потоків
  xTaskCreate(readRFID, "readRFID", 4096, NULL, 1, NULL);   // Потік для зчитування RFID
  xTaskCreate(sendDataToServer, "sendDataToServer", 4096, NULL, 1, NULL); // Потік для відправки даних
}

void loop() {
  // Основний цикл може бути порожнім, оскільки всі задачі виконуються в потоках
}
