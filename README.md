# DiscoveryBoardGame
A+ Assignment

This assignment is concerned with building a simple memory and co-ordination toy using the STM32F3 Discovery board
The board should do the following:
(01) When powered on, the 8 LEDs should spin in a circle until the blue button is pressed.
(02) When the button is pressed, a random sequence of LEDs will light up. Only LD3,LD7,LD10, and LD6 (PE9, PE11,PE13 and PE15) can be in the sequence and each LED in the sequence must be different to the previous one. Each LED must be lit for 500ms. The sequence cannot be the same each time the program is run.
(03) After the sequence has been played, the user has to tip the board to recreate it. Initially all the LEDS will be off. One of the 4 LEDs will light up when the board is tipped towards it (use a threshold of 200 in the x and y direction from the accelerometer with a 2G full scale sensitivity). When the accelerometer detects that the board has been tipped towards an led for more than 500ms it will go off. If it is the correct LED for the sequence, the next LED in the sequence will be detected until they are all correct. If an incorrect LED is detected, the correct LED will be lit for 500ms and the toy will return to the initial spinning loop.
(04) The initial sequence will show 3 LEDs, if the sequence is recreated correctly, a sequence of 4 LEDs will be shown, then 5, then 6 and finally 7. If the user recreates a 7 LED sequence, all the LEDs will flash repeatedly on and off for 200ms until the button is pressed.
