
#include <Adafruit_SPIDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Adafruit_I2CDevice.h>
#include <Adafruit_BusIO_Register.h>
#include <Adafruit_ST77xx.h>
#include <Adafruit_ST7789.h>
#include <Adafruit_ST7735.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <TJpg_Decoder.h>
#include "esp_camera.h"
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15  // Chip select control pin
#define TFT_DC    2  // Data Command control pin
#define TFT_RST   12
#define Facedetection 4

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
    // Stop further decoding as image is running off bottom of screen
    if (y >= tft.height()) return 0;
    tft.drawRGBBitmap(x, y, bitmap, w, h);
    // Return 1 to decode next block
    return 1;
}

void init_camera() {
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
    if (psramFound()) {
        config.frame_size = FRAMESIZE_QQVGA;
        config.jpeg_quality = 10;
        config.fb_count = 1;
        Serial.println("PSRAM");
    }
    else {
        config.frame_size = FRAMESIZE_QQVGA;
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
    sensor_t* s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID) {
        Serial.println("PID");
        s->set_vflip(s, 1); // flip it back
        s->set_brightness(s, 2); // up the brightness just a bit
        s->set_saturation(s, 0);
    }

}
void setup() {
    Serial.begin(115200);
    Serial.println("ESP32-CAM Picture");
    tft.initR(INITR_BLACKTAB);
    tft.setRotation(1);
    tft.fillScreen(ST77XX_GREEN);
    init_camera();
    pinMode(Facedetection, INPUT);
    TJpgDec.setJpgScale(1);
    // The decoder must be given the exact name of the rendering function above
    TJpgDec.setCallback(tft_output);
}
void take_picture() {
    Serial.println("Taking picture..");
    camera_fb_t* fb = NULL;

    fb = esp_camera_fb_get();
    if (!fb) {
        Serial.println("Camera capture failed");
    }

    uint16_t w = 0, h = 0;
    TJpgDec.getJpgSize(&w, &h, fb->buf, fb->len);
    Serial.print("- Width = "); Serial.print(fb->width); Serial.print(", height = "); Serial.println(fb->height);

    Serial.print("Width = "); Serial.print(w); Serial.print(", height = "); Serial.println(h);
    // Draw the image, top left at 0,0
    TJpgDec.drawJpg(0, 0, fb->buf, fb->len);
}
void loop() {
    int state = digitalRead(Facedetection);
    if (state == HIGH) {
        Serial.println("Button pressed");
        tft.fillScreen(ST77XX_GREEN);
    }
    else {
        take_picture();
    }
   
        delay(100);
    
}