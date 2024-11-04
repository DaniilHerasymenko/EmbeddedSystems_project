#include <SPI.h> 
#include <MFRC522.h> 
#include <WiFi.h>
#include <vector>

#define SS_PIN 5 
#define RST_PIN 22 
MFRC522 mfrc522(SS_PIN, RST_PIN);   // Створюємо екземпляр MFRC522

// Налаштування WiFi
const char* ssid = "Redmi Note 9 Pro";
const char* password = "12345678";

// Структура для зберігання UID, імені студента та часу
struct StudentRecord { 
  String uid; 
  unsigned long time; // Час, коли зчитано мітку
}; 

// Динамічний буфер для зберігання зчитаних міток з поточним часом
std::vector<StudentRecord> buffer;
 
void setup()  
{ 
  Serial.begin(9600);   // Початок послідовного зв'язку
  
  SPI.begin();          // Початок роботи SPI
  mfrc522.PCD_Init();   // Ініціалізація MFRC522

  WiFi.begin(ssid, password);

  // Підключення до WiFi
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Підключення до WiFi...");
  }
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  delay(3000);
  digitalWrite(2, LOW);

  Serial.println("Підключено до WiFi");
  Serial.println("Прикладіть картку до рідера..."); 
  Serial.println(); 
}  
 
void loop()  
{ 
  // Перевірка наявності нової картки
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }
  bool isReaded = false;

  // Зчитування UID мітки 
  String uidStr = ""; 
  for (byte i = 0; i < mfrc522.uid.size; i++) { 
    uidStr.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? "0" : "")); 
    uidStr.concat(String(mfrc522.uid.uidByte[i], HEX)); 
    isReaded = true;
  } 
  uidStr.toUpperCase(); 

  if (isReaded){
    analogWrite(2, 255);
  }
  else{
    analogWrite(2, 40);
  }
  delay(1500);
  analogWrite(2, 0);

  // Збереження зчитаного UID з поточним часом у динамічний буфер
  StudentRecord record;
  record.uid = uidStr;
  record.time = millis(); // Отримуємо час з моменту старту програми

  buffer.push_back(record); // Додаємо запис до буфера

  // Виведення результатів
  Serial.print("UID мітки: "); 
  Serial.println(record.uid); 
  Serial.print("Час зчитування (мс): "); 
  Serial.println(record.time);

  // Виведення всього буфера
  Serial.println("\nBUFFER:");
  for (int i = 0; i < buffer.size(); i++) {
    Serial.print("UID: "); 
    Serial.print(buffer[i].uid); 
    Serial.print(", Time: "); 
    Serial.println(buffer[i].time);
  }
  Serial.println("-------------------------");

  delay(3000); // Затримка перед наступним зчитуванням
}

