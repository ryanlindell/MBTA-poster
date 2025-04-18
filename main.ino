#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <FastLED.h>
#include <math.h>
using namespace std;

#define LINE1 16
#define LINE2 15
#define LINE3 14
#define LINE4 13
CRGB leds1[16];
CRGB leds2[13];
CRGB leds3[9];
CRGB leds4[12];

const char* ssid = "BU Guest (unencrypted)";
const char* password = "";

struct Station {
    const char* name;
    const char* id;
    float latitude;
    float longitude;
    int index;
};

const Station line1[16] = {
  {"Blandford Street", "70149", 42.349276, -71.100213, 0},  
  {"Boston University East", "70147", 42.349811, -71.104657, 1},  
  {"Boston University Central", "70145", 42.350148, -71.107455, 2},  
  {"Amory Street", "170140", 42.350901, -71.114318, 3},  
  {"Babcock Street", "170136", 42.351538, -71.119553, 4},  
  {"Packard's Corner", "70135", 42.351651, -71.12551, 5},  
  {"Harvard Avenue", "70131", 42.350602, -71.130727, 6},  
  {"Griggs Street", "70129", 42.348919, -71.134305, 7},  
  {"Allston Street", "70127", 42.348546, -71.137362, 8},  
  {"Warren Street", "70125", 42.348819, -71.140051, 9},  
  {"Washington Street", "70121", 42.343974, -71.142731, 10},  
  {"Sutherland Road", "70117", 42.341577, -71.146607, 11},  
  {"Chiswick Road", "70115", 42.34054, -71.15114, 12},  
  {"Chestnut Hill Avenue", "70113", 42.33829, -71.15302, 13},  
  {"South Street", "70111", 42.339581, -71.157499, 14},  
  {"Boston College", "70107", 42.339581, -71.157499, 15}  
};

const Station line2[13] = {
  {"Cleveland Circle", "70237", 42.336216, -71.149201, 0},
  {"Englewood Avenue", "70235", 42.336964, -71.145867, 1},
  {"Dean Road", "70233", 42.337807, -71.141753, 2},
  {"Tappan Street", "70231", 42.338498, -71.138731, 3},
  {"Washington Square", "70229", 42.339471, -71.135139, 4},
  {"Fairbanks", "70227", 42.33969, -71.131228, 5},
  {"Brandon Hall", "70225", 42.340053, -71.128869, 6},
  {"Summit Avenue", "70223", 42.34112, -71.125652, 7},
  {"Coolidge Corner", "70219", 42.342274, -71.120915, 8},
  {"Saint Paul Street", "70217", 42.34334, -71.116927, 9},
  {"Kent Street", "70215", 42.344117, -71.114097, 10},
  {"Hawes Street", "70213", 42.344758, -71.111761, 11},
  {"Saint Marys Street", "70211", 42.345884, -71.107697, 12}
};

const Station line3[9] = {
  {"Kenmore", "71151", 42.348949, -71.095169, 0},  
  {"Hynes Convention Center", "70153", 42.347888, -71.087903, 1},  
  {"Copley", "70155", 42.350126, -71.077376, 2},  
  {"Arlington", "70156", 42.351902, -71.070893, 3},  
  {"Boylston", "70158", 42.352531, -71.064682, 4},  
  {"Park Street", "70200", 42.356395, -71.062424, 5},  
  {"Government Center", "70202", 42.359705, -71.059215, 6},  
  {"Haymarket", "70204", 42.363021, -71.05829, 7},  
  {"North Station", "70206", 42.36528, -71.060205, 8}  
};

const Station line4[12] = {
  {"Fenway", "70186", 42.345328, -71.104269, 0},  
  {"Longwood", "70183", 42.341571, -71.110147, 1},  
  {"Brookline Village", "70181", 42.33257, -71.117041, 2},  
  {"Beaconsfield", "70179", 42.335041, -71.141306, 3},  
  {"Reservoir", "70175", 42.335163, -71.148601, 4},  
  {"Chestnut Hill", "70173", 42.326782, -71.16478, 5},  
  {"Newton Centre", "70171", 42.3294, -71.192622, 6},  
  {"Newton Highlands", "70169", 42.32253, -71.205421, 7},  
  {"Eliot", "70167", 42.319214, -71.216949, 8},  
  {"Waban", "70165", 42.325967, -71.230714, 9},  
  {"Woodland", "70163", 42.333094, -71.243659, 10},  
  {"Riverside", "70161", 42.337348, -71.252236, 11}  
};

// Define line sizes as constants
const int LINE1_SIZE = 16;
const int LINE2_SIZE = 13;
const int LINE3_SIZE = 9;
const int LINE4_SIZE = 12;

const char* apiKey = "f7837ff2895941aba12294b92629dd9a";

// Structure to store search results
struct SearchResult {
    int lineNumber;  // 1, 2, 3, or 4 representing which line array
    int stationIndex; // Index within that line array
    float distance;   // Distance to the station
};

// Function to calculate Haversine distance
float haversine(float lat1, float lon1, float lat2, float lon2) {
    const float R = 6371.0; // Radius of Earth in km
    float dLat = radians(lat2 - lat1);
    float dLon = radians(lon2 - lon1);
    float a = sin(dLat / 2) * sin(dLat / 2) +
              cos(radians(lat1)) * cos(radians(lat2)) *
              sin(dLon / 2) * sin(dLon / 2);
    float c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c * 1000; // Convert to meters
}

// Function to find the closest station across all lines
SearchResult findClosestStation(float trainLat, float trainLon) {
    SearchResult result = {0, -1, 99999.0};
    float distance;
    
    // Check line1
    for (int i = 0; i < LINE1_SIZE; i++) {
        distance = haversine(trainLat, trainLon, line1[i].latitude, line1[i].longitude);
        if (distance < result.distance) {
            result.lineNumber = 1;
            result.stationIndex = i;
            result.distance = distance;
        }
    }
    
    // Check line2
    for (int i = 0; i < LINE2_SIZE; i++) {
        distance = haversine(trainLat, trainLon, line2[i].latitude, line2[i].longitude);
        if (distance < result.distance) {
            result.lineNumber = 2;
            result.stationIndex = i;
            result.distance = distance;
        }
    }
    
    // Check line3
    for (int i = 0; i < LINE3_SIZE; i++) {
        distance = haversine(trainLat, trainLon, line3[i].latitude, line3[i].longitude);
        if (distance < result.distance) {
            result.lineNumber = 3;
            result.stationIndex = i;
            result.distance = distance;
        }
    }
    
    // Check line4
    for (int i = 0; i < LINE4_SIZE; i++) {
        distance = haversine(trainLat, trainLon, line4[i].latitude, line4[i].longitude);
        if (distance < result.distance) {
            result.lineNumber = 4;
            result.stationIndex = i;
            result.distance = distance;
        }
    }
    
    return result;
}

// Function to update the LED at a specific station
void updateLED(int lineNumber, int stationIndex, CRGB color) {
    switch (lineNumber) {
        case 1:
            if (stationIndex >= 0 && stationIndex < LINE1_SIZE) {
                leds1[stationIndex] = color;
            }
            break;
        case 2:
            if (stationIndex >= 0 && stationIndex < LINE2_SIZE) {
                leds2[stationIndex] = color;
            }
            break;
        case 3:
            if (stationIndex >= 0 && stationIndex < LINE3_SIZE) {
                leds3[stationIndex] = color;
            }
            break;
        case 4:
            if (stationIndex >= 0 && stationIndex < LINE4_SIZE) {
                leds4[stationIndex] = color;
            }
            break;
    }
}

void getTrainLocationsAndNearestStations() {
    HTTPClient http;
    String url = "https://api-v3.mbta.com/vehicles?filter[route]=Green-B,Green-C,Green-D,Green-E";

    http.begin(url);
    http.addHeader("accept", "application/json");
    http.addHeader("x-api-key", apiKey);

    int httpCode = http.GET();
    if (httpCode > 0) {
        String payload = http.getString();
        StaticJsonDocument<4096> doc;
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            Serial.println("Error parsing JSON");
            http.end();
            return;
        }

        // Reset LEDs before updating
        for (int i = 0; i < LINE1_SIZE; i++) leds1[i] = CRGB::Black;
        for (int i = 0; i < LINE2_SIZE; i++) leds2[i] = CRGB::Black;
        for (int i = 0; i < LINE3_SIZE; i++) leds3[i] = CRGB::Black;
        for (int i = 0; i < LINE4_SIZE; i++) leds4[i] = CRGB::Black;

        JsonArray vehicles = doc["data"].as<JsonArray>();
        for (JsonObject vehicle : vehicles) {
            // Extract train data
            float trainLat = vehicle["attributes"]["latitude"];
            float trainLon = vehicle["attributes"]["longitude"];
            
            // Find closest station
            SearchResult closest = findClosestStation(trainLat, trainLon);
            
            // If we found a station within a reasonable distance
            if (closest.stationIndex != -1) {
                // Optionally: Only consider stations within a certain distance (e.g., 200 meters)
                // if (closest.distance <= 200.0) {
                updateLED(closest.lineNumber, closest.stationIndex, CRGB(0, 255, 0));
                // Debug print
                Serial.print("Train at line ");
                Serial.print(closest.lineNumber);
                Serial.print(", station index ");
                Serial.print(closest.stationIndex);
                Serial.print(", distance: ");
                Serial.println(closest.distance);
                // }
            }
        }

        // Apply LED changes
        FastLED.show();
        Serial.println("Updated!");
    } 
    else {
        Serial.println("HTTP request failed");
    }

    http.end();
}

void setup() {
    // Start serial communication
    Serial.begin(115200);

    // Connect to Wi-Fi
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.println("Connected to WiFi");

    FastLED.addLeds<PL9823, LINE1, RGB>(leds1, LINE1_SIZE);
    FastLED.addLeds<PL9823, LINE2, RGB>(leds2, LINE2_SIZE);
    FastLED.addLeds<PL9823, LINE3, RGB>(leds3, LINE3_SIZE);
    FastLED.addLeds<PL9823, LINE4, RGB>(leds4, LINE4_SIZE);
    
    // Initialize all LEDs to off
    for (int i = 0; i < LINE1_SIZE; i++) leds1[i] = CRGB::Black;
    for (int i = 0; i < LINE2_SIZE; i++) leds2[i] = CRGB::Black;
    for (int i = 0; i < LINE3_SIZE; i++) leds3[i] = CRGB::Black;
    for (int i = 0; i < LINE4_SIZE; i++) leds4[i] = CRGB::Black;
    FastLED.show();
}

void loop() {
    getTrainLocationsAndNearestStations();
    delay(500);
}