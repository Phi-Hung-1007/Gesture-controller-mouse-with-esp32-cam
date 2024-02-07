#include <WebServer.h>
#include <WiFi.h>
#include <esp32cam.h>

static const char* WIFI_SSID = "TrungBis";
static const char* WIFI_PASS = "31052003";

WebServer server(80);

static auto loRes = esp32cam::Resolution::find(320, 240);
static auto midRes = esp32cam::Resolution::find(350, 530);
static auto hiRes = esp32cam::Resolution::find(800, 600);

void jpg(){
  auto frame = esp32cam::capture();
  if (frame == nullptr) {
    Serial.println("FOTO GAGAL");
    server.send(503, "", "");
    return;
  }
  Serial.printf("FOTO %dx%d %db\n", frame->getWidth(), frame->getHeight(),
                static_cast<int>(frame->size()));
  server.setContentLength(frame->size());
  server.send(200, "image/jpeg");
  WiFiClient client = server.client();
  frame->writeTo(client);
  
}
void Response(){
  
}
void jpgLow(){
  if (!esp32cam::Camera.changeResolution(loRes)) {
    Serial.println("gagal");
  }
  jpg();
}

void jpgHi(){
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("gagal");
  }
  jpg();
}

void jpgMid(){
  if (!esp32cam::Camera.changeResolution(midRes)) {
    Serial.println("gagal");
  }
  jpg();
}
void
handleMjpeg()
{
  if (!esp32cam::Camera.changeResolution(hiRes)) {
    Serial.println("SET-HI-RES FAIL");
  }

  Serial.println("STREAM BEGIN");
  WiFiClient client = server.client();
  auto startTime = millis();
  int res = esp32cam::Camera.streamMjpeg(client);
  if (res <= 0) {
    Serial.printf("STREAM ERROR %d\n", res);
    return;
  }
  auto duration = millis() - startTime;
  Serial.printf("STREAM END %dfrm %0.2ffps\n", res, 1000.0 * res / duration);
}

void  setup(){
  Serial.begin(115200);
  //pinMode(4, OUTPUT);
  Serial.println();
  {
    using namespace esp32cam;
    Config cfg;
    cfg.setPins(pins::AiThinker);
    cfg.setResolution(hiRes);
    cfg.setBufferCount(2);
    cfg.setJpeg(80);
    bool ok = Camera.begin(cfg);
    Serial.println(ok ? "KAMERA OKE SIH" : "KAMERA, YAK GAGAL");
  }
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("WiFi failure");
    delay(500);
    ESP.restart();
  }
  Serial.println("WiFi connected");

  Serial.print("Resolusi rendah : http://");
  Serial.print(WiFi.localIP());
  Serial.println("/low.jpg");
  Serial.print("Resolusi tinggi : http://");
  Serial.print(WiFi.localIP());
  Serial.println("/hi.jpg");
  Serial.print("Resolusi menengah : http://");
  Serial.print(WiFi.localIP());
  Serial.println("/mid.jpg");
  Serial.print("Resolusi menengah : http://");
  Serial.print(WiFi.localIP());
  Serial.println("/Mjpeg");
  server.on("/low.jpg", jpgLow);
  server.on("/hi.jpg", jpgHi);
  server.on("/mid.jpg", jpgMid);
  server.on("/Mjpeg", handleMjpeg);
  server.begin();
}

void loop(){
  server.handleClient();
  //digitalWrite(4, HIGH);
}
