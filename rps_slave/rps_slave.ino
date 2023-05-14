#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <esp_now.h>
#include <WiFi.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define BUTTON_PIN0 27
#define BUTTON_PIN1 26
#define BUTTON_PIN2 25

// MAC Address of responder - edit as require};
uint8_t broadcastAddress[] = {0x0C,0xB8,0x15,0xC4,0x9A,0xA4};

// Define a data structure
typedef struct struct_message_masterout {
  bool c;
  int b;
} struct_message_masterout;
 
// Create a structured object
struct_message_masterout myData_masterout;
 
// Define a data structure
typedef struct struct_message_masterin {
  int b;
} struct_message_masterin;
 
// Create a structured object
struct_message_masterin myData_masterin;
 

// Peer info
esp_now_peer_info_t peerInfo;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

int lastState0 = HIGH; // the previous state from the input pin
int currentState0;     // the current reading from the input pin

int lastState1 = HIGH; // the previous state from the input pin
int currentState1;     // the current reading from the input pin

int lastState2 = HIGH; // the previous state from the input pin
int currentState2;     // the current reading from the input pin



int player2move = -1;

int result = -1;
bool iftitle = false;


// Define pins for OLED display
#define OLED_MOSI   23
#define OLED_CLK    18
#define OLED_DC     16
#define OLED_CS     5
#define OLED_RESET  17

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
 OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);



void r_p_s_result(int result){
  if (result == 0) {
      display.clearDisplay();
      display.setCursor(50,20);
      display.setTextColor(WHITE);
      display.setTextSize(2);
      display.println("TIE");
      display.display();
      delay(5000);
  } else if (result == 1){
      display.clearDisplay();
      display.setCursor(50,20);
      display.setTextColor(WHITE);
      display.setTextSize(2);
      display.println("you win");
      display.display();
      delay(5000);
  }   else if (result == 2){
      display.clearDisplay();
      display.setCursor(50,20);
      display.setTextColor(WHITE);
      display.setTextSize(2);
      display.println("you lose");
      display.display();
      delay(5000);
  } 
  int player2move = -1;
}

void title(){
  display.clearDisplay();
      display.setCursor(15,10);
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.println("make your moves");
      display.setCursor(15,40);
      display.setTextSize(3);
      display.println("x");
      display.fillRect(40, 40, 20, 50, SSD1306_WHITE);
      display.fillCircle(80, 50, 10, SSD1306_WHITE);
      display.display();
}

void button0(){
  currentState0 = digitalRead(BUTTON_PIN0);

  if(lastState0 == LOW && currentState0 == HIGH){  
    player2move = 0;
    // send 0
  }
  // save the last state
  lastState0 = currentState0;
}

void button1(){
  currentState1 = digitalRead(BUTTON_PIN1);

  if(lastState1 == LOW && currentState1 == HIGH){  
    player2move = 1;
    // send 1
  }
  // save the last state
  lastState1 = currentState1;
}

void button2(){
  currentState2 = digitalRead(BUTTON_PIN2);

  if(lastState2 == LOW && currentState2 == HIGH){  
    player2move = 2;
    // send 2
  }
  // save the last state
  lastState2 = currentState2;
}

// Callback function executed when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&myData_masterout, incomingData, sizeof(myData_masterout));
  Serial.print("Data received: ");
  Serial.println(len);
  Serial.print("title: ");
  iftitle = myData_masterout.c;
  Serial.print("result: ");
  result = myData_masterout.b;
  Serial.println(myData_masterout.b);
  Serial.println();
}

void setup()
{
  Serial.begin(9600);
    if(!display.begin(SSD1306_SWITCHCAPVCC)){
      Serial.println("SSD1306 allocation failed");
      for(;;);
  }
  
  pinMode(BUTTON_PIN0, INPUT_PULLUP);
  pinMode(BUTTON_PIN1, INPUT_PULLUP);
  pinMode(BUTTON_PIN2, INPUT_PULLUP);


  // Set ESP32 as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Initilize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Register callback function
  esp_now_register_recv_cb(OnDataRecv);

  // Register the send callback
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  pinMode(33, OUTPUT);
  digitalWrite(33, HIGH);
  pinMode(32, OUTPUT);
  digitalWrite(33, HIGH);
  pinMode(14, OUTPUT);
  digitalWrite(33, HIGH);

  Serial.println("setup done");

}

void loop(){
  // recieve if title
  if(iftitle == true){
  title();
  iftitle = false;
  } 
  myData_masterin.b = player2move;
  // Send message via ESP-NOW
  esp_err_t send_result = esp_now_send(broadcastAddress, (uint8_t *) &myData_masterin, sizeof(myData_masterin));
   
  if (send_result == ESP_OK) {
    Serial.println("Sending confirmed");
  }
  else {
    Serial.print("Sending error; ");
    Serial.println(send_result);

  } if (player2move == -1){
    button0();
    button1();
    button2();
  } else if (player2move != -1){
    // recieve result 
    if (result != -1){
        r_p_s_result(result);
        player2move = -1;
        result = -1;
    }
  }
  
}
