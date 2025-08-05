#ifndef KEY_MATRIX_H
#define KEY_MATRIX_H

#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "class/hid/hid.h"

class KeyMatrix
{
private:
	// --- Matrix Configuration ---
	static const int NUM_ROWS = 4;
	static const int NUM_COLS = 3;
	const uint ROW_PINS[NUM_ROWS] = {12, 11, 10, 9};
	const uint COL_PINS[NUM_COLS] = {15, 14, 13};

	// Maps the physical button layout to HID keycodes
	const uint8_t KEY_MAP[NUM_ROWS][NUM_COLS] = {
		{HID_KEY_1, HID_KEY_2, HID_KEY_3},
		{HID_KEY_4, HID_KEY_5, HID_KEY_6},
		{HID_KEY_7, HID_KEY_8, HID_KEY_9},
		{HID_KEY_ESCAPE, HID_KEY_0, HID_KEY_ENTER}};

public:
	// Public array to hold the keycodes of currently pressed keys.
	// The main loop will read from this. A standard HID report can hold 6 keys.
	uint8_t key_codes[6] = {0};

	// Constructor: Initializes all the GPIO pins for the matrix
	KeyMatrix()
	{
		// Initialize Row pins as outputs and set them low
		for (int i = 0; i < NUM_ROWS; ++i)
		{
			gpio_init(ROW_PINS[i]);
			gpio_set_dir(ROW_PINS[i], GPIO_OUT);
			gpio_put(ROW_PINS[i], 0);
		}

		// Initialize Column pins as inputs with pull-down resistors
		for (int i = 0; i < NUM_COLS; ++i)
		{
			gpio_init(COL_PINS[i]);
			gpio_set_dir(COL_PINS[i], GPIO_IN);
			gpio_pull_down(COL_PINS[i]);
		}
	}

	// Scans the matrix and updates the key_codes array.
	// Returns true if any key is currently pressed, false otherwise.
	bool update()
	{
		// Clear the previous key report
		for (int i = 0; i < 6; i++)
		{
			key_codes[i] = 0;
		}

		int key_count = 0;
		bool any_key_pressed = false;

		// Scan the matrix
		for (int row = 0; row < NUM_ROWS; ++row)
		{
			// Drive the current row HIGH
			gpio_put(ROW_PINS[row], 1);

			// A tiny delay for the signal to stabilize
			sleep_us(50);

			for (int col = 0; col < NUM_COLS; ++col)
			{
				// If a column pin is now HIGH, the button is pressed
				if (gpio_get(COL_PINS[col]))
				{
					any_key_pressed = true;
					// Add the keycode to our report if there's space
					if (key_count < 6)
					{
						key_codes[key_count++] = KEY_MAP[row][col];
					}
				}
			}

			// Drive the current row back LOW
			gpio_put(ROW_PINS[row], 0);
		}

		return any_key_pressed;
	}
};

#endif // KEY_MATRIX_H
