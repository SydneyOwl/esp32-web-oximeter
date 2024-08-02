#include "cal_BPM_SpO2.h"
#include <SPI.h>
#include "epd1in54.h"
#include "epdpaint.h"
#include "imagedata.h"

#define COLORED 0
#define UNCOLORED 1

MAX30105 particleSensor;

extern double eSpO2;
extern double Ebpm;
extern bool max30102_fail;

void updateDisplay_task(void *pvParameters);

unsigned char image[1024];
Epd epd;
Paint paint(image, 0, 0); // width should be the multiple of 8



void setup()
{
    // 初始化串口
    Serial.begin(115200);
    if (epd.Init(lut_partial_update) != 0)
    {
        Serial.print("e-Paper init failed");
        return;
    }

    epd.ClearFrameMemory(0xFF); // bit set = white, bit reset = black
    epd.DisplayFrame();
    epd.ClearFrameMemory(0xFF); // bit set = white, bit reset = black
    epd.DisplayFrame();

    // 开启 心率&血压 监测任务
    if (xTaskCreate(
            cal_BPM_SpO2_task,
            "cal_BPM_SpO2",
            4096, /* Stack depth - small microcontrollers will use much
            less stack than this. */
            NULL, /* This example does not use the task parameter. */
            1,    /* This task will run at priority 1. */
            NULL) /* This example does not use the task handle. */
        != pdPASS)
    {
        log_e("Couldn't create cal_BPM_SpO2 task\n");
    }
    // 开启 心率&血压 监测任务
    if (xTaskCreate(
            updateDisplay_task,
            "updateDisplay_task",
            4096, /* Stack depth - small microcontrollers will use much
            less stack than this. */
            NULL, /* This example does not use the task parameter. */
            1,    /* This task will run at priority 1. */
            NULL) /* This example does not use the task handle. */
        != pdPASS)
    {
        log_e("Couldn't create UpdateDisplay task\n");
    }
}

void loop()
{

}

void updateDisplay_task(void *pvParameters)
{
    if (max30102_fail)
    {
        paint.SetWidth(200);
        paint.SetHeight(20);
        paint.Clear(COLORED);
        paint.DrawStringAt(20, 2, "MAX30102 ERROR", &Font16, UNCOLORED);
        epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());
        epd.DisplayFrame();
        return;
    }
    while (1)
    {
        delay(500);
        if (abs(eSpO2 - MINIMUM_SPO2) < 0.1)
        {
            paint.DrawStringAt(0, 20, "---", &Font24, COLORED);
        }
        paint.SetWidth(200);
        paint.SetHeight(20);
        paint.Clear(COLORED);
        // paint.DrawStringAt(20, 2, ipaddr.c_str(), &Font16, UNCOLORED);
        epd.SetFrameMemory(paint.GetImage(), 0, 0, paint.GetWidth(), paint.GetHeight());

        paint.SetWidth(200);
        paint.SetHeight(40);
        paint.Clear(UNCOLORED);
        if (abs(eSpO2 - MINIMUM_SPO2) < 0.1)
        {
            paint.DrawStringAt(0, 20, "SpO2: ---", &Font24, COLORED);
        }
        else
        {
            std::string tmp = std::to_string(eSpO2);
            std::string result = tmp.substr(0, tmp.find(".") + 2);

            paint.DrawStringAt(2, 20, ("SpO2: " + result + "%").c_str(), &Font24, COLORED);
        }
        epd.SetFrameMemory(paint.GetImage(), 0, 30, paint.GetWidth(), paint.GetHeight());

        paint.SetWidth(200);
        paint.SetHeight(40);
        paint.Clear(UNCOLORED);
        if (abs(eSpO2 - MINIMUM_SPO2) < 0.1)
        {
            paint.DrawStringAt(0, 20, "BPM: ---", &Font24, COLORED);
        }
        else
        {
            std::string tmp = std::to_string(Ebpm);
            std::string result = tmp.substr(0, tmp.find("."));
            paint.DrawStringAt(2, 20, ("BPM: " + result).c_str(), &Font24, COLORED);
        }
        epd.SetFrameMemory(paint.GetImage(), 0, 80, paint.GetWidth(), paint.GetHeight());
        epd.DisplayFrame();
    }
}
