#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <DHT.h>

// WiFi và Firebase
#define WIFI_SSID "92"
#define WIFI_PASSWORD "1234567891011"
#define API_KEY "AIzaSyDW8WZjl27_RNBq7qr4KZkxFcGkEKwVh1U"
#define DATABASE_URL "https://smarthouse-9a832-default-rtdb.asia-southeast1.firebasedatabase.app/"

// Firebase
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
bool signUp = false;

// Cảm biến và thiết bị
#define DHTPIN D2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define MQ2PIN A0

// LED hiển thị trạng thái
#define LED_QUAT D5
#define LED_DEN1 D6
#define LED_LOA D7

void capNhatFirebase(const String &path, int value)
{
  if (Firebase.RTDB.setInt(&fbdo, path, value))
  {
    Serial.printf("Cập nhật %s = %d\n", path.c_str(), value);
  }
  else
  {
    Serial.printf("Lỗi cập nhật %s: %s\n", path.c_str(), fbdo.errorReason().c_str());
  }
}

void hienTrangThaiThietBi(const String &path, int pin)
{
  if (Firebase.RTDB.getInt(&fbdo, path))
  {
    int trangThai = fbdo.intData();
    digitalWrite(pin, trangThai == 1 ? HIGH : LOW);
    Serial.printf("%s = %d\n", path.c_str(), trangThai);
  }
  else
  {
    Serial.printf("Lỗi đọc %s: %s\n", path.c_str(), fbdo.errorReason().c_str());
  }
}

void setup()
{
  Serial.begin(115200);
  dht.begin();

  pinMode(LED_QUAT, OUTPUT);
  pinMode(LED_DEN1, OUTPUT);
  pinMode(LED_LOA, OUTPUT);

  // Kết nối WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Đang kết nối WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nKết nối WiFi thành công");
  Serial.println(WiFi.localIP());

  // Cấu hình Firebase
  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("Đăng ký Firebase thành công");
    signUp = true;
  }
  else
  {
    Serial.printf("Lỗi đăng ký: %s\n", config.signer.signupError.message.c_str());
  }

  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void loop()
{
  if (Firebase.ready() && signUp)
  {
    // Đọc dữ liệu từ DHT11 và MQ2
    float doam = dht.readHumidity();
    float nhietdo = dht.readTemperature();
    int gas = analogRead(MQ2PIN); // MQ2 trả về giá trị từ 0 - 1023

    // Kiểm tra dữ liệu hợp lệ
    if (isnan(nhietdo) || isnan(doam))
    {
      Serial.println("Không đọc được dữ liệu từ DHT11");
      return;
    }

    // Gửi dữ liệu lên Firebase
    capNhatFirebase("/pkhach/Nhietdo", (int)nhietdo);
    capNhatFirebase("/pkhach/Doam", (int)doam);
    capNhatFirebase("/pkhach/Gas", gas);

    capNhatFirebase("/bep/Nhietdo", (int)nhietdo);
    capNhatFirebase("/bep/Doam", (int)doam);
    capNhatFirebase("/bep/Gas", gas);

    capNhatFirebase("/pngu1/Nhietdo", (int)nhietdo);
    capNhatFirebase("/pngu1/Doam", (int)doam);
    capNhatFirebase("/pngu1/Gas", gas);

    capNhatFirebase("/pngu2/Nhietdo", (int)nhietdo);
    capNhatFirebase("/pngu2/Doam", (int)doam);
    capNhatFirebase("/pngu2/Gas", gas);

    // Đọc trạng thái thiết bị và hiển thị bằng LED
    hienTrangThaiThietBi("/thietbi1/quat", LED_QUAT);
    hienTrangThaiThietBi("/thietbi2/den1", LED_DEN1);
    hienTrangThaiThietBi("/thietbi3/loa", LED_LOA);

    Serial.println("----------");
  }

  delay(1000); // 3 giây mỗi vòng lặp
}
