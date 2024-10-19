#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#include <DHT.h>

// Thông tin kết nối WiFi
const char* ssid = "Huong Tran_2.4g";         
const char* password = "tughidi9";   

// Thông tin Firebase
#define FIREBASE_HOST "teamdha-12864-default-rtdb.firebaseio.com"
#define FIREBASE_AUTH "s2zVUa91xFf3de5FaXQK2Mx9gg6hjnWEEHu4i42k"

// Khai báo chân relay
#define RELAY1 D1  
#define RELAY2 D2  
#define RELAY3 D5  

// Khai báo chân nút nhấn
#define BUTTON1 3   
#define BUTTON2 D7  
#define BUTTON3 D3  

// Khai báo chân cảm biến mưa
#define RAIN_SENSOR_PIN D0  

// Thiết lập cảm biến DHT11
#define DHTPIN D4    
#define DHTTYPE DHT11  
DHT dht(DHTPIN, DHTTYPE);

// Biến lưu trạng thái nút nhấn và relay
bool lastButton1State = HIGH;
bool lastButton2State = HIGH;
bool lastButton3State = HIGH;
int relay1State = HIGH;  
int relay2State = HIGH;
int relay3State = HIGH;

// Biến lưu trạng thái mưa
int rainStatus = HIGH; 

// Thời gian debounce
unsigned long lastDebounceTime1 = 0;
unsigned long lastDebounceTime2 = 0;
unsigned long lastDebounceTime3 = 0;
unsigned long debounceDelay = 50;

// Khởi tạo Firebase và WiFiClient
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

void setup() {
  Serial.begin(115200);

  // Kết nối WiFi với thời gian chờ
  WiFi.begin(ssid, password);
  Serial.print("Đang kết nối WiFi...");
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 30000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Không thể kết nối WiFi.");
    return;
  }

  Serial.println("\nĐã kết nối WiFi!");
  Serial.print("Địa chỉ IP: ");
  Serial.println(WiFi.localIP());

  // Cấu hình Firebase
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  // Thiết lập chân relay là OUTPUT
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(RELAY3, OUTPUT);

  // Thiết lập trạng thái ban đầu của relay
  digitalWrite(RELAY1, relay1State);
  digitalWrite(RELAY2, relay2State);
  digitalWrite(RELAY3, relay3State);

  // Thiết lập chân nút nhấn là INPUT_PULLUP
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);

  // Thiết lập chân cảm biến mưa
  pinMode(RAIN_SENSOR_PIN, INPUT);

  // Khởi động DHT11
  dht.begin();
}

void loop() {
  // Kiểm tra và điều khiển relay từ nút nhấn
  checkButtonPress();

  // Đọc cảm biến DHT và gửi dữ liệu lên Firebase
  DHT_to_firebase();

  // Đọc trạng thái mưa và gửi dữ liệu lên Firebase
  rainSensorToFirebase();

  // Đọc trạng thái relay từ Firebase
  updateRelayFromFirebase();

  // Gửi trạng thái relay hiện tại lên Firebase
  updateRelayToFirebase();
}

// Hàm kiểm tra trạng thái nút nhấn và cập nhật relay
void checkButtonPress() {
  unsigned long currentTime = millis();

  // Kiểm tra nút nhấn 1
  bool reading1 = digitalRead(BUTTON1);
  if (reading1 != lastButton1State && currentTime - lastDebounceTime1 > debounceDelay) {
    lastDebounceTime1 = currentTime;
    if (reading1 == LOW) {
      relay1State = !relay1State;  // Đổi trạng thái relay 1
      digitalWrite(RELAY1, relay1State);
      delay(10);  // Thêm delay để tránh việc nhấn nhiều lần
      Serial.println("Relay 1 đã thay đổi trạng thái từ nút nhấn");

      // Cập nhật trạng thái lên Firebase
      String relay1StateStr = (relay1State == HIGH) ? "1" : "0";  
      if (!Firebase.setString(firebaseData, "/Relay1", relay1StateStr)) {
        Serial.println("Lỗi khi cập nhật Relay 1 lên Firebase: " + firebaseData.errorReason());
      }
    }
  }
  lastButton1State = reading1;

  // Tương tự cho nút nhấn 2 và nút nhấn 3
  bool reading2 = digitalRead(BUTTON2);
  if (reading2 != lastButton2State && currentTime - lastDebounceTime2 > debounceDelay) {
    lastDebounceTime2 = currentTime;
    if (reading2 == LOW) {
      relay2State = !relay2State;
      digitalWrite(RELAY2, relay2State);
      delay(10);
      Serial.println("Relay 2 đã thay đổi trạng thái từ nút nhấn");

      String relay2StateStr = (relay2State == HIGH) ? "1" : "0";  
      if (!Firebase.setString(firebaseData, "/Relay2", relay2StateStr)) {
        Serial.println("Lỗi khi cập nhật Relay 2 lên Firebase: " + firebaseData.errorReason());
      }
    }
  }
  lastButton2State = reading2;

  bool reading3 = digitalRead(BUTTON3);
  if (reading3 != lastButton3State && currentTime - lastDebounceTime3 > debounceDelay) {
    lastDebounceTime3 = currentTime;
    if (reading3 == LOW) {
      relay3State = !relay3State;
      digitalWrite(RELAY3, relay3State);
      delay(10);
      Serial.println("Relay 3 đã thay đổi trạng thái từ nút nhấn");

      String relay3StateStr = (relay3State == HIGH) ? "1" : "0";  
      if (!Firebase.setString(firebaseData, "/Relay3", relay3StateStr)) {
        Serial.println("Lỗi khi cập nhật Relay 3 lên Firebase: " + firebaseData.errorReason());
      }
    }
  }
  lastButton3State = reading3;
}

// Đọc và gửi dữ liệu từ DHT11 lên Firebase
void DHT_to_firebase() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Lỗi đọc dữ liệu từ DHT11!");
    return;
  }

  Serial.print("Nhiệt độ: ");
  Serial.print(temperature);
  Serial.print("°C | Độ ẩm: ");
  Serial.print(humidity);
  Serial.println("%");

  if (!Firebase.setFloat(firebaseData, "Temperature", temperature)) {
    Serial.println("Lỗi khi cập nhật nhiệt độ: " + firebaseData.errorReason());
  }

  if (!Firebase.setFloat(firebaseData, "Humidity", humidity)) {
    Serial.println("Lỗi khi cập nhật độ ẩm: " + firebaseData.errorReason());
  }
}

// Đọc trạng thái cảm biến mưa và gửi lên Firebase
void rainSensorToFirebase() {
  int currentRainStatus = digitalRead(RAIN_SENSOR_PIN);

  if (currentRainStatus != rainStatus) {
    rainStatus = currentRainStatus;  
    String rainMessage = (rainStatus == LOW) ? "0" : "1";
    Serial.println("Trạng thái mưa: " + rainMessage);

    if (!Firebase.setString(firebaseData, "/RainSensor", rainMessage)) {
      Serial.println("Lỗi khi cập nhật trạng thái mưa lên Firebase: " + firebaseData.errorReason());
    }
  }
}

// Đọc trạng thái relay từ Firebase để điều khiển
void updateRelayFromFirebase() {
  if (Firebase.getString(firebaseData, "/Relay1")) {
    String firebaseRelay1State = firebaseData.stringData();
    int relay1StateInt = (firebaseRelay1State == "1") ? HIGH : LOW;  
    if (relay1StateInt != relay1State) {
      relay1State = relay1StateInt;
      digitalWrite(RELAY1, relay1State);
      Serial.println("Relay 1 đã thay đổi trạng thái từ Firebase");
    }
  } else {
    Serial.println("Lỗi khi đọc Relay 1 từ Firebase: " + firebaseData.errorReason());
  }

  // Tương tự cho relay 2 và 3
  if (Firebase.getString(firebaseData, "/Relay2")) {
    String firebaseRelay2State = firebaseData.stringData();
    int relay2StateInt = (firebaseRelay2State == "1") ? HIGH : LOW;  
    if (relay2StateInt != relay2State) {
      relay2State = relay2StateInt;
      digitalWrite(RELAY2, relay2State);
      Serial.println("Relay 2 đã thay đổi trạng thái từ Firebase");
    }
  } else {
    Serial.println("Lỗi khi đọc Relay 2 từ Firebase: " + firebaseData.errorReason());
  }

  if (Firebase.getString(firebaseData, "/Relay3")) {
    String firebaseRelay3State = firebaseData.stringData();
    int relay3StateInt = (firebaseRelay3State == "1") ? HIGH : LOW;  
    if (relay3StateInt != relay3State) {
      relay3State = relay3StateInt;
      digitalWrite(RELAY3, relay3State);
      Serial.println("Relay 3 đã thay đổi trạng thái từ Firebase");
    }
  } else {
    Serial.println("Lỗi khi đọc Relay 3 từ Firebase: " + firebaseData.errorReason());
  }
}

// Cập nhật trạng thái relay lên Firebase
void updateRelayToFirebase() {
  String relay1StateStr = (relay1State == HIGH) ? "1" : "0";
  if (!Firebase.setString(firebaseData, "/Relay1", relay1StateStr)) {
    Serial.println("Lỗi khi cập nhật Relay 1: " + firebaseData.errorReason());
  }

  String relay2StateStr = (relay2State == HIGH) ? "1" : "0";
  if (!Firebase.setString(firebaseData, "/Relay2", relay2StateStr)) {
    Serial.println("Lỗi khi cập nhật Relay 2: " + firebaseData.errorReason());
  }

  String relay3StateStr = (relay3State == HIGH) ? "1" : "0";
  if (!Firebase.setString(firebaseData, "/Relay3", relay3StateStr)) {
    Serial.println("Lỗi khi cập nhật Relay 3: " + firebaseData.errorReason());
  }
}
