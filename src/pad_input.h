/*
 * pad_input.h - BLiNX 1's pad layer: scripts, host pad slots and the
 * keyboard, behind the toolkit's emulated USB pad (usb_gamepad.h).
 */
#ifndef BLINX1_PAD_INPUT_H
#define BLINX1_PAD_INPUT_H

/* Read the script and the sources and install the layer as the USB pad's
 * source. Before xbox_OhciInit, so the controller thread starts with it and
 * the first movies' file opens are seen. Once; later calls do nothing. */
void blinx1_pad_init(void);

#endif
