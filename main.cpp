#include <M5Core2.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "EGR425_Phase1_weather_bitmap_images.h"
#include "WiFi.h"
#include <NTPClient.h>
#include <time.h>



////////////////////////////////////////////////////////////////////
// Variables
////////////////////////////////////////////////////////////////////
// TODO 3: Register for openweather account and get API key
String urlOpenWeather = "https://api.openweathermap.org/data/2.5/weather?";
String apiKey = "<API_KEY>";

// TODO 1: WiFi variables
String wifiNetworkName = "";
String wifiPassword = "";

// Change temperature vairables
bool isCelcius = false;
bool isDay = true;

// Zip code screen variables
signed int zipCode[5] = {9,2,5,4,3};
static unsigned long screenHeight = 240;
bool isZipCodeScreen = false;


static unsigned long topRow = 40;
static unsigned long buttonWidth = 50;
static unsigned long buttonHeight = 40;
static unsigned long buttonPadding = 10;
// -- TOP BUTTONS TOUCH ZONES FOR ZIP CODE
static HotZone button1_hz_top(buttonPadding,topRow, (buttonWidth + buttonPadding), (topRow + buttonHeight));
static HotZone button2_hz_top(buttonPadding*2 + buttonWidth,topRow, (buttonWidth*2 + buttonPadding*2), (topRow + buttonHeight));
static HotZone button3_hz_top(buttonPadding*3 + buttonWidth*2,topRow, (buttonWidth*3 + buttonPadding*3), (topRow + buttonHeight));
static HotZone button4_hz_top(buttonPadding*4 + buttonWidth*3,topRow, (buttonWidth*4 + buttonPadding*5), (topRow + buttonHeight));
static HotZone button5_hz_top(buttonPadding*5 + buttonWidth*4,topRow, (buttonWidth*5 + buttonPadding*5), (topRow + buttonHeight));
// -- BOTTOM BUTTONS TOUCH ZONES FOR ZIP CODE
static HotZone button1_hz_bottom(buttonPadding,topRow+120, (buttonWidth + buttonPadding), ((topRow+120) + buttonHeight));
static HotZone button2_hz_bottom(buttonPadding*2 + buttonWidth,topRow+120, (buttonWidth*2 + buttonPadding*2), ((topRow+120) + buttonHeight));
static HotZone button3_hz_bottom(buttonPadding*3 + buttonWidth*2,topRow+120, (buttonWidth*3 + buttonPadding*3), ((topRow+120) + buttonHeight));
static HotZone button4_hz_bottom(buttonPadding*4 + buttonWidth*3,topRow+120, (buttonWidth*4 + buttonPadding*5), ((topRow+120) + buttonHeight));
static HotZone button5_hz_bottom(buttonPadding*5 + buttonWidth*4,topRow+120, (buttonWidth*5 + buttonPadding*5), ((topRow+120) + buttonHeight));

// Time variables
unsigned long lastTime = 0;
unsigned long timerDelay = 5000;  // 5000; 5 minutes (300,000ms) or 5 seconds (5,000ms)
double Celcius(double farenhiet);

// LCD variables
int sWidth;
int sHeight;

// Time Client
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org", 3600, 60000);

////////////////////////////////////////////////////////////////////
// Method header declarations
////////////////////////////////////////////////////////////////////
String httpGETRequest(const char* serverName);
void drawWeatherImage(String iconId, int resizeMult);
// void createButtons(int x, int y, int w, int h);

///////////////////////////////////////////////////////////////
// Put your setup code here, to run once
///////////////////////////////////////////////////////////////
void setup() {
    // Initialize the device
    M5.begin();
    
    // Set screen orientation and get height/width 
    sWidth = M5.Lcd.width();
    sHeight = M5.Lcd.height();

    // TODO 2: Connect to WiFi
    WiFi.begin(wifiNetworkName.c_str(), wifiPassword.c_str());
    timeClient.begin();
    Serial.printf("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.print("\n\nConnected to WiFi network with IP address: ");
    Serial.println(WiFi.localIP());
}

///////////////////////////////////////////////////////////////
// Put your main code here, to run repeatedly
///////////////////////////////////////////////////////////////
void loop() {

    M5.update();

    if(M5.BtnB.wasPressed()){
        isZipCodeScreen = !isZipCodeScreen;
        lastTime = 0;
    } else if(M5.BtnA.wasPressed()){
        isCelcius = !isCelcius;
        lastTime = 0;
    }
    // Only execute every so often
    if (!isZipCodeScreen) {
        if((millis() - lastTime) > timerDelay){
            if (WiFi.status() == WL_CONNECTED) {

                //////////////////////////////////////////////////////////////////
                // TODO 4: Hardcode the specific city,state,country into the query
                // Examples: https://api.openweathermap.org/data/2.5/weather?q=riverside,ca,usa&units=imperial&appid=YOUR_API_KEY
                //////////////////////////////////////////////////////////////////
                String serverURL = urlOpenWeather + "zip=" + zipCode[0] + zipCode[1] + zipCode[2] +
                zipCode[3] + zipCode[4] + ",us&units=imperial&appid=" + apiKey;
                //Serial.println(serverURL); // Debug print

                //////////////////////////////////////////////////////////////////
                // TODO 5: Make GET request and store reponse
                //////////////////////////////////////////////////////////////////
                String response = httpGETRequest(serverURL.c_str());
                //Serial.print(response); // Debug print
                
                //////////////////////////////////////////////////////////////////
                // TODO 6: Import ArduinoJSON Library and then use arduinojson.org/v6/assistant to
                // compute the proper capacity (this is a weird library thing) and initialize
                // the json object
                //////////////////////////////////////////////////////////////////
                const size_t jsonCapacity = 768+250;
                DynamicJsonDocument objResponse(jsonCapacity);

                //////////////////////////////////////////////////////////////////
                // TODO 7: (uncomment) Deserialize the JSON document and test if parsing succeeded
                //////////////////////////////////////////////////////////////////
                DeserializationError error = deserializeJson(objResponse, response);
                if (error) {
                    Serial.print(F("deserializeJson() failed: "));
                    Serial.println(error.f_str());
                    return;
                }
                //serializeJsonPretty(objResponse, Serial); // Debug print

                //////////////////////////////////////////////////////////////////
                // TODO 8: Parse Response to get the weather description and icon
                //////////////////////////////////////////////////////////////////

                if(objResponse["cod"] == "404"){
                    M5.Lcd.setCursor(20, 20);
                    M5.Lcd.fillScreen(TFT_BLACK);
                    M5.Lcd.setTextColor(TFT_WHITE);
                    M5.Lcd.setTextSize(2);
                    M5.Lcd.print("Invalid Zip code. Please enter another zip code.");
                } else {
                    JsonArray arrWeather = objResponse["weather"];
                    JsonObject objWeather0 = arrWeather.getElement(0);
                    String strWeatherDesc = objWeather0["main"];
                    String strWeatherIcon = objWeather0["icon"];
                    String cityName = objResponse["name"];

                    // TODO 9: Parse response to get the temperatures
                    JsonObject objMain = objResponse["main"];
                    double tempNow = isCelcius ? Celcius(objMain["temp"]) : objMain["temp"];
                    double tempMin = isCelcius ? Celcius(objMain["temp_min"]) : objMain["temp_min"];
                    double tempMax = isCelcius ? Celcius(objMain["temp_max"]) : objMain["temp_max"];
                    
                    Serial.printf("NOW: %.1f F and %s\tMIN: %.1f F\tMax: %.1f F\n", tempNow, strWeatherDesc, tempMin, tempMax);

                    //////////////////////////////////////////////////////////////////
                    // TODO 10: We can download the image directly, but there is no easy
                    // way to work with a PNG file on the ESP32 (in Arduino) so we will
                    // take another route - see EGR425_Phase1_weather_bitmap_images.h
                    // for more details
                    //////////////////////////////////////////////////////////////////
                    // String imagePath = "http://openweathermap.org/img/wn/" + strWeatherIcon + "@2x.png";
                    // Serial.println(imagePath);
                    // response = httpGETRequest(imagePath.c_str());
                    // Serial.print(response);

                    //////////////////////////////////////////////////////////////////
                    // TODO 12: Draw background - light blue if day time and navy blue of night
                    //////////////////////////////////////////////////////////////////
                    uint16_t primaryTextColor;
                    if (strWeatherIcon.indexOf("d") >= 0) {
                        M5.Lcd.fillScreen(TFT_CYAN);
                        primaryTextColor = TFT_DARKGREY;
                        isDay = true;
                    } else {
                        M5.Lcd.fillScreen(TFT_LIGHTGREY);
                        primaryTextColor = TFT_WHITE;
                        isDay = false;
                    }
                    
                    //////////////////////////////////////////////////////////////////
                    // TODO 13: Draw the icon on the right side of the screen - the built in 
                    // drawBitmap method works, but we cannot scale up the image
                    // size well, so we'll call our own method
                    //////////////////////////////////////////////////////////////////
                    //M5.Lcd.drawBitmap(0, 0, 100, 100, myBitmap, TFT_BLACK);
                    drawWeatherImage(strWeatherIcon, 2);
                    
                    //////////////////////////////////////////////////////////////////
                    // This code will draw the temperature centered on the screen; left
                    // it out in favor of another formatting style
                    //////////////////////////////////////////////////////////////////
                    /*M5.Lcd.setTextColor(TFT_WHITE);
                    textSize = 10;
                    M5.Lcd.setTextSize(textSize);
                    int iTempNow = tempNow;
                    tWidth = textSize * String(iTempNow).length() * 5;
                    tHeight = textSize * 5;
                    M5.Lcd.setCursor(sWidth/2 - tWidth/2,sHeight/2 - tHeight/2);
                    M5.Lcd.print(iTempNow);*/

                    //////////////////////////////////////////////////////////////////
                    // TODO 14: Draw the temperatures and city name
                    //////////////////////////////////////////////////////////////////
                    int pad = 10;
                    M5.Lcd.setCursor(pad, pad);
                    if(!isDay){
                        M5.Lcd.setTextColor(TFT_CYAN);
                    } else if(isDay){
                        M5.Lcd.setTextColor(TFT_WHITE);
                    }
                    M5.Lcd.setTextSize(3);
                    if(!isCelcius) {
                        M5.Lcd.printf("LO:%0.fF\n", tempMin);
                    } else {
                        M5.Lcd.printf("LO:%0.fC\n", tempMin);
                    }
                    
                    M5.Lcd.setCursor(pad, M5.Lcd.getCursorY() + pad);
                    M5.Lcd.setTextColor(primaryTextColor);
                    M5.Lcd.setTextSize(10);
                    if(!isCelcius) {
                        M5.Lcd.printf("%0.fF\n", tempNow);
                    } else {
                        M5.Lcd.printf("%0.fC\n", tempNow);
                    }

                    M5.Lcd.setCursor(pad * 15, pad);
                    M5.Lcd.setTextColor(TFT_RED);
                    M5.Lcd.setTextSize(3);
                    if(!isCelcius) {
                        M5.Lcd.printf("HI:%0.fF\n", tempMax);
                    } else {
                        M5.Lcd.printf("HI:%0.fC\n", tempMax);
                    }

                    M5.Lcd.setCursor(pad, M5.Lcd.getCursorY() * 6);
                    M5.Lcd.setTextColor(TFT_BLACK);
                    M5.Lcd.printf("%s\n", cityName.c_str());

                    timeClient.update();
                    M5.Lcd.setCursor(pad, M5.Lcd.getCursorY()+2);
                    M5.Lcd.setTextSize(1);
                    M5.Lcd.setTextColor(TFT_BLACK);
                    M5.Lcd.print("Last sync time: " + timeClient.getFormattedTime());
                }
            } else {
                Serial.println("WiFi Disconnected");
            }
            // Update the last time to NOW
        lastTime = millis();
        }
    } else if(isZipCodeScreen){

        M5.Lcd.fillScreen(BLACK);

        // First Top Buton
        M5.Lcd.drawRect(buttonPadding, topRow, buttonWidth, buttonHeight, TFT_WHITE);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(buttonPadding + (buttonWidth/2), topRow + (buttonHeight / 2));
        M5.Lcd.print("^");

        // createButtons(buttonPadding, topRow, buttonWidth, buttonHeight);

        // Second Top Button
        M5.Lcd.drawRect(((buttonPadding*2)+buttonWidth), topRow, buttonWidth, buttonHeight, TFT_WHITE);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(((buttonPadding*2)+ buttonWidth + (buttonWidth / 2)), topRow + (buttonHeight / 2));
        M5.Lcd.print("^");

        // Third Top Button
        M5.Lcd.drawRect(((buttonPadding*3)+buttonWidth*2), topRow, buttonWidth, buttonHeight, TFT_WHITE);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(((buttonPadding*3)+(buttonWidth*2) + (buttonWidth / 2)), topRow + (buttonHeight / 2));
        M5.Lcd.print("^");
        
        // Fourth Top Button
        M5.Lcd.drawRect(((buttonPadding*4)+buttonWidth*3), topRow, buttonWidth, buttonHeight, TFT_WHITE);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(((buttonPadding*4)+(buttonWidth*3) + (buttonWidth / 2)), topRow + (buttonHeight / 2));
        M5.Lcd.print("^");
        
        // Fourth Top Button
        M5.Lcd.drawRect(((buttonPadding*5)+buttonWidth*4), topRow, buttonWidth, buttonHeight, TFT_WHITE);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(((buttonPadding*5)+(buttonWidth*4) + (buttonWidth / 2)), topRow + (buttonHeight / 2));
        M5.Lcd.print("^");


        // ----------- BOTTOM BUTTONS SECTION --------------//

        // First Bottom Buton
        M5.Lcd.drawRect(buttonPadding, topRow + 120, buttonWidth, buttonHeight, TFT_WHITE);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(buttonPadding + (buttonWidth/2), (topRow + 120) + (buttonHeight / 2));
        M5.Lcd.print("v");

        // Second Bottom Button
        M5.Lcd.drawRect(((buttonPadding*2)+buttonWidth), topRow + 120, buttonWidth, buttonHeight, TFT_WHITE);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(((buttonPadding*2)+ buttonWidth + (buttonWidth / 2)), (topRow + 120) + (buttonHeight / 2));
        M5.Lcd.print("v");

        // Third Bottom Button
        M5.Lcd.drawRect(((buttonPadding*3)+buttonWidth*2), topRow + 120, buttonWidth, buttonHeight, TFT_WHITE);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(((buttonPadding*3)+(buttonWidth*2) + (buttonWidth / 2)), (topRow + 120) + (buttonHeight / 2));
        M5.Lcd.print("v");
        
        // Fourth Bottom Button
        M5.Lcd.drawRect(((buttonPadding*4)+buttonWidth*3), topRow + 120, buttonWidth, buttonHeight, TFT_WHITE);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(((buttonPadding*4)+(buttonWidth*3) + (buttonWidth / 2)), (topRow + 120) + (buttonHeight / 2));
        M5.Lcd.print("v");
        
        // Fourth Bottom Button
        M5.Lcd.drawRect(((buttonPadding*5)+buttonWidth*4), topRow + 120, buttonWidth, buttonHeight, TFT_WHITE);
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(((buttonPadding*5)+(buttonWidth*4) + (buttonWidth / 2)), (topRow + 120) + (buttonHeight / 2));
        M5.Lcd.print("v");

        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(1);
        M5.Lcd.setCursor(buttonPadding, screenHeight - 20);
        M5.Lcd.print("Press middle button to save and return to screen.");



        // First Zip Code Number
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(buttonPadding - 10 + (buttonWidth/2), (topRow + buttonHeight + 10) + (buttonHeight / 2));
        M5.Lcd.print(zipCode[0]);

        // Second Zip Code Number
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(((buttonPadding*2) - 10 + buttonWidth + (buttonWidth / 2)), (topRow + buttonHeight + 10) + (buttonHeight / 2));
        M5.Lcd.print(zipCode[1]);

        // Third Zip Code Number
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(((buttonPadding*3) - 10 + (buttonWidth*2) + (buttonWidth / 2)), (topRow + buttonHeight + 10) + (buttonHeight / 2));
        M5.Lcd.print(zipCode[2]);

        // Fourth Zip Code Number
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(((buttonPadding*4) - 10 + (buttonWidth*3) + (buttonWidth / 2)), (topRow + buttonHeight + 10) + (buttonHeight / 2));
        M5.Lcd.print(zipCode[3]);

        // Fifth Zip Code Number
        M5.Lcd.setTextColor(TFT_WHITE);
        M5.Lcd.setTextSize(3);
        M5.Lcd.setCursor(((buttonPadding*5) - 10 + (buttonWidth*4) + (buttonWidth / 2)), (topRow + buttonHeight + 10) + (buttonHeight / 2));
        M5.Lcd.print(zipCode[4]);


        
        if(M5.Touch.ispressed()){
                TouchPoint tp = M5.Touch.getPressPoint();
                if(button1_hz_top.inHotZone(tp)){
                    if(zipCode[0] + 1 > 9){
                        zipCode[0] = 0;
                    } else {
                        zipCode[0] = zipCode[0] + 1;
                    }
                } else if(button2_hz_top.inHotZone(tp)){
                    if(zipCode[1] + 1 > 9){
                        zipCode[1] = 0;
                    } else {
                        zipCode[1] = zipCode[1] + 1;
                    }
                } else if(button3_hz_top.inHotZone(tp)){
                    if(zipCode[2] + 1 > 9){
                        zipCode[2] = 0;
                    } else {
                        zipCode[2] = zipCode[2] + 1;
                    }
                } else if(button4_hz_top.inHotZone(tp)){
                    if(zipCode[3] + 1 > 9){
                        zipCode[3] = 0;
                    } else {
                        zipCode[3] = zipCode[3] + 1;
                    }
                } else if(button5_hz_top.inHotZone(tp)){
                    if(zipCode[4] + 1 > 9){
                        zipCode[4] = 0;
                    } else {
                        zipCode[4] = zipCode[4] + 1;
                    }
                } else if(button1_hz_bottom.inHotZone(tp)){
                    if(zipCode[0] - 1 < 0){
                        zipCode[0] = 9;
                    } else {
                        zipCode[0] = zipCode[0] - 1;
                    }
                } else if(button2_hz_bottom.inHotZone(tp)){
                    if(zipCode[1] - 1 < 0){
                        zipCode[1] = 9;
                    } else {
                        zipCode[1] = zipCode[1] - 1;
                    }
                } else if(button3_hz_bottom.inHotZone(tp)){
                    if(zipCode[2] - 1 < 0){
                        zipCode[2] = 9;
                    } else {
                        zipCode[2] = zipCode[2] - 1;
                    }
                } else if(button4_hz_bottom.inHotZone(tp)){
                    if(zipCode[3] - 1 < 0){
                        zipCode[3] = 9;
                    } else {
                        zipCode[3] = zipCode[3] - 1;
                    }
                } else if(button5_hz_bottom.inHotZone(tp)){
                    if(zipCode[4] - 1 < 0){
                        zipCode[4] = 9;
                    } else {
                        zipCode[4] = zipCode[4] - 1;
                    }
                }
        }
        // lastTime = millis();

        delay(100);
        
     }
}

// void createButtons(int x, int y, int w, int h){
//     M5.update();
//         M5.Lcd.drawRect(x, y, w, h, TFT_WHITE);
//         M5.Lcd.setTextColor(TFT_WHITE);
//         M5.Lcd.setTextSize(1);
//         M5.Lcd.setCursor(x + (w/2), y + (h / 2));
//         M5.Lcd.print("^");
// }

/////////////////////////////////////////////////////////////////
// This method takes in a URL and makes a GET request to the
// URL, returning the response.
/////////////////////////////////////////////////////////////////
String httpGETRequest(const char* serverURL) {
    
    // Initialize client
    HTTPClient http;
    http.begin(serverURL);

    // Send HTTP GET request and obtain response
    int httpResponseCode = http.GET();
    String response = http.getString();

    // Check if got an error
    if (httpResponseCode > 0)
        Serial.printf("HTTP Response code: %d\n", httpResponseCode);
    else {
        Serial.printf("HTTP Response ERROR code: %d\n", httpResponseCode);
        Serial.printf("Server Response: %s\n", response);
    }

    // Free resources and return response
    http.end();
    return response;
}

// Method to convert F to C
double Celcius(double farenhiet) {
    return (farenhiet - 32) * 5/9;
}

/////////////////////////////////////////////////////////////////
// This method takes in an image icon string (from API) and a 
// resize multiple and draws the corresponding image (bitmap byte
// arrays found in EGR425_Phase1_weather_bitmap_images.h) to scale (for 
// example, if resizeMult==2, will draw the image as 200x200 instead
// of the native 100x100 pixels) on the right-hand side of the
// screen (centered vertically). 
/////////////////////////////////////////////////////////////////
void drawWeatherImage(String iconId, int resizeMult) {

    // Get the corresponding byte array
    const uint16_t * weatherBitmap = getWeatherBitmap(iconId);

    // Compute offsets so that the image is centered vertically and is
    // right-aligned
    int yOffset = -(resizeMult * imgSqDim - M5.Lcd.height()) / 2;
    int xOffset = sWidth - (imgSqDim*resizeMult*.8); // Right align (image doesn't take up entire array)
    //int xOffset = (M5.Lcd.width() / 2) - (imgSqDim * resizeMult / 2); // center horizontally
    
    // Iterate through each pixel of the imgSqDim x imgSqDim (100 x 100) array
    for (int y = 0; y < imgSqDim; y++) {
        for (int x = 0; x < imgSqDim; x++) {
            // Compute the linear index in the array and get pixel value
            int pixNum = (y * imgSqDim) + x;
            uint16_t pixel = weatherBitmap[pixNum];

            // If the pixel is black, do NOT draw (treat it as transparent);
            // otherwise, draw the value
            if (pixel != 0) {
                // 16-bit RBG565 values give the high 5 pixels to red, the middle
                // 6 pixels to green and the low 5 pixels to blue as described
                // here: http://www.barth-dev.de/online/rgb565-color-picker/
                byte red = (pixel >> 11) & 0b0000000000011111;
                red = red << 3;
                byte green = (pixel >> 5) & 0b0000000000111111;
                green = green << 2;
                byte blue = pixel & 0b0000000000011111;
                blue = blue << 3;

                // Scale image; for example, if resizeMult == 2, draw a 2x2
                // filled square for each original pixel
                for (int i = 0; i < resizeMult; i++) {
                    for (int j = 0; j < resizeMult; j++) {
                        int xDraw = x * resizeMult + i + xOffset;
                        int yDraw = y * resizeMult + j + yOffset;
                        M5.Lcd.drawPixel(xDraw, yDraw, M5.Lcd.color565(red, green, blue));
                    }
                }
            }
        }
    }
}
//////////////////////////////////////////////////////////////////////////////////
// For more documentation see the following links:
// https://github.com/m5stack/m5-docs/blob/master/docs/en/api/
// https://docs.m5stack.com/en/api/core2/lcd_api
//////////////////////////////////////////////////////////////////////////////////