#include "esp_camera.h"
#include <WiFi.h>

//firebase  config -------------------------------------
// Include Firebase ESP32 library (this library)
#include <FirebaseESP32.h>

//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

//Define the DNS Name
#define HOSTNAME "camserver"

#include "camera_pins.h"

/* Put your SSID & Password */
const char* ssid = "RickAndMorty";  // Enter SSID here
const char* password = "princess@e104";  //Enter Password here

void startCameraServer();


//firebase vars


// Define the Firebase Data object
FirebaseData fbdo;

// Define the FirebaseAuth data for authentication data
FirebaseAuth auth;

// Define the FirebaseConfig data for config data
FirebaseConfig fConfig;
//-------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");



  // setup firebase 

// Assign the project host and api key (required)
fConfig.host = "hd-robot-default-rtdb.asia-southeast1.firebasedatabase.app";
fConfig.api_key = "AIzaSyCaNfprZxEN6trDFsVDgTFBpSGAe6xXLNk";

// Assign the user sign in credentials
auth.user.email = "rajendra@rajendra.rk";

auth.user.password = "rajendra@rajendra.rk";

// Initialize the library with the Firebase authen and config.
Firebase.begin(&fConfig, &auth);

// Optional, set AP reconnection in setup()
Firebase.reconnectWiFi(true);

// Optional, set number of error retry
Firebase.setMaxRetry(fbdo, 3);

// Optional, set number of error resumable queues
Firebase.setMaxErrorQueue(fbdo, 30);

// Optional, use classic HTTP GET and POST requests. 
// This option allows get and delete functions (PUT and DELETE HTTP requests) works for 
// device connected behind the Firewall that allows only GET and POST requests.   
Firebase.enableClassicRequest(fbdo, true);

// Optional, set the size of HTTP response buffer
// Prevent out of memory for large payload but data may be truncated and can't determine its type.
fbdo.setResponseSize(8192); // minimum size is 4096 bytes

//#define FIREBASE_HOST "hd-robot-default-rtdb.asia-southeast1.firebasedatabase.app" //Without http:// or https:// schemes
//#define FIREBASE_AUTH "gYaPu4ufQkaw9w6Mp4Da3bLEPZYnKNX1OHMkxJlg"
//------------------------------------------------------

//send IP to firebase ---------------------------------------------
  String ip = WiFi.localIP().toString();
  Firebase.setString(fbdo, "unit1/config/camera_ip",ip);

  
}

void loop() {
  // put your main code here, to run repeatedly:
  delay(10000);
}
