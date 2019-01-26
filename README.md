# TIGDrive
A simple welding positioner firmware for Arduino

Easy-to-implement stepper-based TIG welding rotary axis firmware!

Pinout:
11: STEP (output, stepper driver)
12: DIRECTION (output, stepper driver)
4-9: LCD SHIELD (operation)
10: LCD SHIELD (brightness)
A0: LCD SHIELD (buttons)

ADC values in readLcdButtons() are tuned experimentally, taken from my results with https://github.com/RobTC/SliderCam/tree/master/LCD_button_ADC_input
Other LCD shields may need different values for the divider, more info here - http://www.dfrobot.com/wiki/index.php/LCD_KeyPad_Shield_For_Arduino_SKU:_DFR0009 - on the Amazon SainSmart shield (it's identical).

The step calculations are designed for 2x microstepping and a 3:1 reduction ratio on the output; if you wish to change either of these, the adjustments will have to be made in the main actuation() function.

If a V.2 happens (only if I upgrade the hardware to NEMA 23/34 with a 6-8" chuck, most likely), it'll likely have rotary positioning with settable number of positions and a high-speed bidirectional mode for indicating/tapping in parts in the chuck

