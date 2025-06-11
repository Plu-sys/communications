#include "sensors.h"
#include <libpynq.h>
#include <gpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> // Keep for other potential functions
#include <time.h>
#include <pinmap.h> // Make sure this correctly maps your IO_ARx pins
#include <string.h>
#include <iic.h>
#include "vl53l0x.h"

#define S0 IO_AR4
#define S1 IO_AR5
#define S2 IO_AR6
#define S3 IO_AR7 
#define SIGNAL IO_A0 // Digital Output, make sure that the pin can handle it

// Sampling settings
#define MEASUREMENT_TIME_MS 100 // Time over which to count pulses for frequency (e.g., 100 ms)
int i;
vl53x sensor;
uint32_t baselineDistance = 0;
uint32_t currentDistance;
uint8_t model, revision;

unsigned long red_frequency, green_frequency, blue_frequency, clear_frequency;

// --- Calibration Variables ---
// These will store the pulse counts for white and black surfaces.
unsigned long white_red_freq_cal = 0;
unsigned long white_green_freq_cal = 0;
unsigned long white_blue_freq_cal = 0;
// unsigned long white_clear_freq_cal = 0; // Clear can be useful for auto-brightness adjustment

unsigned long black_red_freq_cal = 0;
unsigned long black_green_freq_cal = 0;
unsigned long black_blue_freq_cal = 0;
// unsigned long black_clear_freq_cal = 0;

int distanceInit(){

    switchbox_set_pin(IO_AR_SCL, SWB_IIC0_SCL);
	switchbox_set_pin(IO_AR_SDA, SWB_IIC0_SDA);

	uint8_t addr = 0x29;
	i = tofPing(IIC0, addr);
	printf("Sensor Ping: ");
	if(i != 0) {
		printf("Fail\n");
		return 1;
	}
	printf("Succesful Ping.\n");

    i = tofInit(&sensor, IIC0, addr, 0); // Default range
	

    tofGetModel(&sensor, &model, &revision);
	printf("Model ID - %d\n", model);
	printf("Revision ID - %d\n", revision);

    // Measure baseline distance (no object)
	printf("Measuring baseline... Ensure no object in front of sensor.\n");
	sleep_msec(2000);
	baselineDistance = tofReadDistance(&sensor);
	printf("Baseline distance = %d mm\n", baselineDistance);

    if (i != 0) {
		return -1;
	} else {
        return 1;
    }

}

unsigned long getDistance(){

    currentDistance = tofReadDistance(&sensor);
	int delta = baselineDistance - currentDistance;

	printf("Distance = %d mm => ", currentDistance);
    return currentDistance;
}

void colorInit(){
    gpio_set_direction(S0, GPIO_DIR_OUTPUT);
    gpio_set_direction(S1, GPIO_DIR_OUTPUT);
    gpio_set_direction(S2, GPIO_DIR_OUTPUT);
    gpio_set_direction(S3, GPIO_DIR_OUTPUT);
    gpio_set_direction(SIGNAL, GPIO_DIR_INPUT);

    // Set frequency scaling:
    // S0=L, S1=H -> 2%
    // S0=H, S1=L -> 20%
    // S0=H, S1=H -> 100%
    gpio_set_level(S0, GPIO_LEVEL_HIGH); // 20% scaling
    gpio_set_level(S1, GPIO_LEVEL_LOW);

    iic_init(IIC0);
}

void colorCalibrate(){
    printf("TCS3200 Color Sensor Calibration and Reading\n");
    printf("Ensure consistent lighting and sensor distance for all steps.\n\n");

    printf("--- WHITE CALIBRATION ---\n");
    printf("Place a WHITE object in front of the sensor. Press Enter to continue...\n");
    getchar(); // Wait for user to press Enter

    printf("Calibrating for WHITE...\n");
    // set_filter('C'); // No need to measure clear for mapping, but can be informative
    // white_clear_freq_cal = measure_frequency(SIGNAL, MEASUREMENT_TIME_MS);
    // printf("  White Clear Frequency: %lu pulses\n", white_clear_freq_cal);

    set_filter('R');
    white_red_freq_cal = measure_frequency(SIGNAL, MEASUREMENT_TIME_MS);
    printf("  White Red Frequency: %lu pulses\n", white_red_freq_cal);

    set_filter('G');
    white_green_freq_cal = measure_frequency(SIGNAL, MEASUREMENT_TIME_MS);
    printf("  White Green Frequency: %lu pulses\n", white_green_freq_cal);

    set_filter('B');
    white_blue_freq_cal = measure_frequency(SIGNAL, MEASUREMENT_TIME_MS);
    printf("  White Blue Frequency: %lu pulses\n", white_blue_freq_cal);
    printf("White calibration complete.\n\n");

    printf("--- BLACK CALIBRATION ---\n");
    printf("Place a BLACK object in front of the sensor. Press Enter to continue...\n");
    getchar();

    printf("Calibrating for BLACK...\n");
    // set_filter('C');
    // black_clear_freq_cal = measure_frequency(SIGNAL, MEASUREMENT_TIME_MS);
    // printf("  Black Clear Frequency: %lu pulses\n", black_clear_freq_cal);

    set_filter('R');
    black_red_freq_cal = measure_frequency(SIGNAL, MEASUREMENT_TIME_MS);
    printf("  Black Red Frequency: %lu pulses\n", black_red_freq_cal);

    set_filter('G');
    black_green_freq_cal = measure_frequency(SIGNAL, MEASUREMENT_TIME_MS);
    printf("  Black Green Frequency: %lu pulses\n", black_green_freq_cal);

    set_filter('B');
    black_blue_freq_cal = measure_frequency(SIGNAL, MEASUREMENT_TIME_MS);
    printf("  Black Blue Frequency: %lu pulses\n", black_blue_freq_cal);
    printf("Black calibration complete.\n\n");

    printf("--- End Calibration ---\n");
    printf("Calibration values captured.\n");
    sleep_msec(2000); // Give user time to read message
};


char getColor(){
    set_filter('R');
    red_frequency = measure_frequency(SIGNAL, MEASUREMENT_TIME_MS);

    set_filter('G');
    green_frequency = measure_frequency(SIGNAL, MEASUREMENT_TIME_MS);

    set_filter('B');
    blue_frequency = measure_frequency(SIGNAL, MEASUREMENT_TIME_MS);

    // Optional: Read Clear frequency for intensity or ambient light info
    set_filter('C');
    clear_frequency = measure_frequency(SIGNAL, MEASUREMENT_TIME_MS);

    // ***** CRUCIAL FIX: Use the calibrated values for mapping *****
    // map_color(current_reading, black_cal_value, white_cal_value, output_min, output_max)
    unsigned long red_mapped   = map_color(red_frequency,   black_red_freq_cal,   white_red_freq_cal,   0, 255);
    unsigned long green_mapped = map_color(green_frequency, black_green_freq_cal, white_green_freq_cal, 0, 255);
    unsigned long blue_mapped  = map_color(blue_frequency,  black_blue_freq_cal,  white_blue_freq_cal,  0, 255);

    printf("Raw Freqs -> R: %5lu G: %5lu B: %5lu C: %5lu  |  Mapped RGB -> R: %3lu, G: %3lu, B: %3lu",
            red_frequency, green_frequency, blue_frequency, clear_frequency,
            red_mapped, green_mapped, blue_mapped);
    char color = determine_color(red_mapped, green_mapped, blue_mapped);
    return color;
};

// Get current time in microseconds
unsigned long get_time_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (unsigned long)(ts.tv_sec * 1000000 + ts.tv_nsec / 1000);
}

// Function to measure frequency by counting pulses over a time interval
unsigned long measure_frequency(io_t pin, unsigned long measurement_time_ms) {
    unsigned long pulse_count = 0;
    unsigned long start_time = get_time_us();
    unsigned long end_time = start_time + (measurement_time_ms * 1000); // Convert ms to us

    // Optional: Wait for the signal to go low before starting, for a potentially cleaner start.
    // Adjust timeout if necessary, or remove if causing issues with very low frequencies.
    unsigned long timeout_start = get_time_us();
    while (gpio_get_level(pin) == GPIO_LEVEL_HIGH && get_time_us() < end_time) {
        if (get_time_us() - timeout_start > (measurement_time_ms * 1000 / 2)) break; // Prevent getting stuck
    }


    while (get_time_us() < end_time) {
        // Wait for rising edge
        while (gpio_get_level(pin) == GPIO_LEVEL_LOW && get_time_us() < end_time);
        if (get_time_us() >= end_time) break;

        // Wait for falling edge
        while (gpio_get_level(pin) == GPIO_LEVEL_HIGH && get_time_us() < end_time);
        if (get_time_us() >= end_time) break;

        pulse_count++;
    }
    return pulse_count; // This is a pulse count, not Hz. Mapping uses this directly.
}


// Set filter for color sensor
void set_filter(char color) {
    switch (color) {
        case 'R': // Red filter
            gpio_set_level(S2, GPIO_LEVEL_LOW);
            gpio_set_level(S3, GPIO_LEVEL_LOW);
            break;
        case 'G': // Green filter
            gpio_set_level(S2, GPIO_LEVEL_HIGH);
            gpio_set_level(S3, GPIO_LEVEL_HIGH);
            break;
        case 'B': // Blue filter
            gpio_set_level(S2, GPIO_LEVEL_LOW);
            gpio_set_level(S3, GPIO_LEVEL_HIGH);
            break;
        case 'C': // Clear (no filter)
            gpio_set_level(S2, GPIO_LEVEL_HIGH);
            gpio_set_level(S3, GPIO_LEVEL_LOW);
            break;
        default:
            fprintf(stderr, "Warning: Unknown color filter '%c'\n", color);
            break;
    }
    sleep_msec(5); // Increased settle time slightly for more stability after filter change
}

// Map function for color values
unsigned long map_color(unsigned long x, unsigned long in_min, unsigned long in_max, unsigned long out_min, unsigned long out_max) {
    // Ensure in_min is less than in_max for sensible mapping.
    // If black reading is higher than white (unusual, but possible with noise/error), swap them.
    if (in_min > in_max) {
        unsigned long temp = in_min;
        in_min = in_max;
        in_max = temp;
    }
    
    if (x < in_min) x = in_min;
    if (x > in_max) x = in_max;

    if (in_max == in_min) return out_min; // Avoid division by zero, return min output

    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void control_leds(unsigned long r, unsigned long g, unsigned long b) {
    // These thresholds are placeholders. Adjust after successful calibration and testing.
    // Example: Turn on RED_LED if red is the dominant color and significantly high.
    // Consider relative intensities or more sophisticated logic.
    gpio_set_level(RED_LED,   (r > 200 && r > g && r > b) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
    gpio_set_level(GREEN_LED, (g > 200 && g > r && g > b) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
    gpio_set_level(BLUE_LED,  (b > 200 && b > r && b > g) ? GPIO_LEVEL_HIGH : GPIO_LEVEL_LOW);
}

char determine_color(unsigned long r, unsigned long g, unsigned long b) {
    char color;
        if (r < 50 && g < 50 && b < 50) {
            color = 'black';
            return color;
        }

        if (r > 200 && b > 200 && g > 200) {
            color = 'white';
            return color;
        }

        if (r > b && r > g + 25) {
            color = 'red';
            return color;
        }

        if (b > r && b > g) {
            color = 'blue';
            return color;
        }

        if (g > r && g > b) {
            color = 'green';
            return color;
        }
        else {
            color = 'err';
        }
        return color;
        
}
