/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"
#include "tusb.h"

#include "usb_descriptors.h"
#include "key_matrix.h"

 // --- Global object for our keypad ---
KeyMatrix keyboard;

// --- Function Prototype ---
void hid_task(void);

/*------------- MAIN -------------*/
int main(void) {
    board_init();
    tusb_init();

    while (1) {
        tud_task(); // tinyusb device task
        hid_task();
    }

    return 0;
}

//--------------------------------------------------------------------+
// HID Task
//--------------------------------------------------------------------+

static void send_hid_report() {
    // skip if hid is not ready yet
    if (!tud_hid_ready())
        return;

    // Use the key_codes array from our global keyboard object
    tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keyboard.key_codes);
}

// Polls the keypad and sends a report if needed
void hid_task(void) {
    // Poll every 10ms
    const uint32_t interval_ms = 10;
    static uint32_t start_ms = 0;

    if (board_millis() - start_ms < interval_ms) {
        return; // not enough time
    }
    start_ms += interval_ms;

    // Scan the matrix to see if any keys are pressed
    bool const keys_pressed = keyboard.update();

    // We send a report if the state has changed (keys are pressed now, or were just released)
    static bool last_keys_pressed = false;
    if (keys_pressed != last_keys_pressed) {
        send_hid_report();
    }
    last_keys_pressed = keys_pressed;

    // Remote wakeup
    if (tud_suspended() && keys_pressed) {
        // Wake up host if we are in suspend mode
        tud_remote_wakeup();
    }
}

// --- TinyUSB Callbacks ---

// Invoked when received GET_REPORT control request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

// Invoked when received SET_REPORT control request (e.g., for Caps Lock)
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    // This function is still required by TinyUSB, but we can leave it empty.
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}

// Device mounted, unmounted, suspended, resumed callbacks
void tud_mount_cb(void) {}
void tud_umount_cb(void) {}
void tud_suspend_cb(bool remote_wakeup_en) { (void)remote_wakeup_en; }
void tud_resume_cb(void) {}
