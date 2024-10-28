#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/uart.h"
#include "include/defaultEye.h"
#include "lib/Adafruit-GFX-Library-Pico/Adafruit_GFX.h"
#include "lib/Adafruit-SSD1351-library/Adafruit_SSD1351.h"

const uint16_t (*sclera)[SCLERA_WIDTH] = scleraDefault;
const uint8_t (*upper)[SCREEN_WIDTH] = upperDefault;
const uint8_t (*lower)[SCREEN_WIDTH] = lowerDefault;
const uint16_t (*polar)[80] = polarDefault;
const uint16_t (*iris)[IRIS_MAP_WIDTH] = irisDefault;
typedef Adafruit_SSD1351 displayType; // Using OLED display(s)

#define SPI_PORT spi0
#define DISPLAY_DC      16 // Data/command pin for BOTH displays
#define DISPLAY_RESET   5 // Reset pin for BOTH displays
#define SELECT_L_PIN    17 // LEFT eye chip select pin
#define SELECT_R_PIN    04 // RIGHT eye chip select pin
#define UART_RX_PIN     13 // Pin to receive UART commands from controller
#define MOSI_PIN  23
#define SCLK_PIN  18

#define TRACKING          // If enabled, eyelid tracks pupil
#define IRIS_SMOOTH       // If enabled, filter input from IRIS_PIN
#define IRIS_MIN      150 // Clip lower analogRead() range from IRIS_PIN (WAS: 120) - Reduced range so that it doesn't look to odd with multiple eye pairs
#define IRIS_MAX      400 // Clip upper "                                (WAS: 720) - Reduced range so that it doesn't look to odd with multiple eye pairs
#define AUTOBLINK        // If enabled, eyes blink autonomously

// Eye blinks are a tiny 3-state machine.  Per-eye allows winks + blinks.
#define NOBLINK 0     // Not currently engaged in a blink
#define ENBLINK 1     // Eyelid is currently closing
#define DEBLINK 2     // Eyelid is currently opening
typedef struct {
  uint8_t  state;     // NOBLINK/ENBLINK/DEBLINK
  uint32_t duration;  // Duration of blink state (micros)
  uint32_t startTime; // Time (micros) of last state change
} eyeBlink;

struct {
    displayType display;
    uint8_t cs;
    eyeBlink blink;
} eye[] ={
    Adafruit_SSD1351(128, 128, SCLK_PIN, MOSI_PIN, DISPLAY_DC, DISPLAY_RESET, SELECT_L_PIN),SELECT_L_PIN,{NOBLINK},
    Adafruit_SSD1351(128, 128, SCLK_PIN, MOSI_PIN, DISPLAY_DC, DISPLAY_RESET, SELECT_R_PIN), SELECT_L_PIN,{NOBLINK},

};
#define NUM_EYES (sizeof(eye) / sizeof(eye[0]));


// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart0
#define BAUD_RATE 115200

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 0
#define UART_RX_PIN 1



int main()
{
    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(SPI_PORT, 1000*1000);
    gpio_set_function(SELECT_L_PIN,   GPIO_FUNC_SIO);
    gpio_set_function(SELECT_R_PIN,   GPIO_FUNC_SIO);
    gpio_set_function(SCLK_PIN,  GPIO_FUNC_SPI);
    gpio_set_function(MOSI_PIN, GPIO_FUNC_SPI);
    gpio_set_dir(DISPLAY_RESET, GPIO_OUT);
    gpio_put(DISPLAY_RESET, 0);
    sleep_ms(1);
    gpio_put(DISPLAY_RESET, 1);
    sleep_ms(50);
    
    // Chip select is active-low, so we'll initialise it to a driven-high state
    gpio_set_dir(SELECT_L_PIN, GPIO_OUT);
    gpio_set_dir(SELECT_R_PIN, GPIO_OUT);
    gpio_put(SELECT_L_PIN, 1);
    gpio_put(SELECT_R_PIN, 1);

    gpio_put(SELECT_L_PIN, 0);
    eye[0].display.begin();
    gpio_put(SELECT_L_PIN, 1);
    gpio_put(SELECT_R_PIN, 0);
    eye[1].display.begin();
    gpio_put(SELECT_R_PIN, 1);

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);
    
    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART
    
    // Send out a string, with CR/LF conversions
    uart_puts(UART_ID, " Hello, UART!\n");
    
    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    while (true) {
        printf("Hello, world!\n");
        sleep_ms(1000);
    }
}
