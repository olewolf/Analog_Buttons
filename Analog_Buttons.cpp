/** @file Analog_Buttons.cpp
 *
 * Library to decode a number of digital buttons that are connected to 
 * a single analog input using a resistor ladder.  The buttons are
 * connected as follows:
 * 
 *    +5V ----+----
 *            |
 *           [ ] R
 *           [ ]
 *            |
 *    A0 -----+---- (Button5) ----> GND
 *            |
 *           [ ] R
 *           [ ]
 *            |
 *            +---- (Button1) ----> GND
 *            |
 *           [ ] R
 *           [ ]
 *            |
 *            +---- (Button2) ----> GND
 *            |
 *           [ ] R
 *           [ ]
 *            |
 *            +---- (Button3) ----> GND
 *            |
 *           [ ] R
 *           [ ]
 *            |
 *            +---- (Button4) ----> GND
 *            |
 *           [ ] R
 *           [ ]
 *            |
 *    GND ----+-----
 *
 * That is, the buttons short the resistor ladder at different positions,
 * causing the voltage at A0 to change according to which button is pressed.
 * Note that the last button is at the top, but otherwise the buttons are
 * placed in increasing order.
 * 
 * The voltage V at A0 is then given by the following equation, where N is 
 * the total number of buttons, n is  * the button number, and VCC is the
 * supply voltage:
 * 
 *     V = VCC * ( N - n )R / 6R                               (Eq. 1)
 *
 * (So, the resistor value doesn't matter as long as all resistors have the
 * same value.)
 * 
 * The library is initialized with the analog pin number (in the above
 * diagram, it is A0) and the number of buttons.
 * 
 * The buttons are debounced and may optionally be configured to send
 * repeated presses if they are held down.
 *
 *
 * (C) 2015 Ole Wolf <wolf@blazingangles.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <Arduino.h>
#include "Analog_Buttons.h"


/* Supply voltage at the top of the resistor ladder. */
static const float SUPPLY_VOLTAGE = 5.0;

/* Debounce time in milliseconds. */
static const int DEFAULT_DEBOUNCE_TIME  = 50;
/* Milliseconds before the button "repeats" if held down.  The value 0
   disables repeat. */
static const int DEFAULT_REPEAT_TIME    =  0;



/**
 * Constructor for the Analog Buttons class.  The constructor stores
 * the analog pin number and the number of buttons, and initializes the
 * button repeat time and the button debounce time to default values.
 * 
 * @param [in] pin_number The analog pin number; e.g., A0.
 * @param [in] number_of_buttons Number of buttons in the resistor ladder.
 */
Analog_Buttons::Analog_Buttons( int pin_number, int number_of_buttons )
  : pin( pin_number ),
    number_buttons( number_of_buttons )
{
  /* Initialize the repeat time and the debounce time. */
  repeat( DEFAULT_REPEAT_TIME );
  debounce( DEFAULT_DEBOUNCE_TIME );

  /* Configure the pin as an input pin. */
  pinMode( pin, INPUT );

  /* Initialize the state machine. */
  last_key = -1;
  keypad_state = KEYPAD_STATE_IDLE;
  debounce_timestamp = repeating_timestamp = millis( );
}



/**
 * Read the analog voltage of the button pin and determine which button
 * has been pressed based on the voltage divider output.
 * 
 * The button is derived from Eq. 1 by:
 * 
 *    n = ( 1 - V )N + V
 * 
 * Rounding n to the nearest integer yields the button number.
 */
int Analog_Buttons::determine_button( ) const
{
  int voltage_i = analogRead( pin );
  /* The remaining code should be converted to integer math. */
  float voltage = SUPPLY_VOLTAGE * (float)voltage_i / 1023.0;

  float button_approx = ( 1.0 - voltage ) * (float)number_buttons + voltage;
  int button = (int)roundf( button_approx );

#if 0
  int voltage = SUPPLY_VOLTAGE * voltage_i;
  int button_approx = ( 1023 - voltage_i ) * number_buttons + voltage * 1023;
  int button = ( button_approx + half_voltage ) >> 10;
#endif

  /* Full voltage means no button is pressed. */
  if( button > number_buttons )
  {
    button = -1;
  }
  return( button );
}



/**
 * Get the currently pressed keypad button, if any.  The function
 * makes use of a state machine to handle debouncing.
 *
 * @return Keypad button, or @a -1 if no button is pressed.
 */
int Analog_Buttons::read_key( )
{
  int key_reported = determine_button( );

  /* We're currently debouncing.  Determine if the debounce period has
     expired. */
  if( keypad_state == KEYPAD_STATE_DEBOUNCING )
  {
    /* Accept the current keypad measurement if the debounce period
       has expired, then prepare to repeat the key and go the IDLE state. */
    if( millis( ) > debounce_timestamp + debounce( ) )
    {
      last_key = key_reported;
      repeating_timestamp = millis( );
      keypad_state = KEYPAD_STATE_IDLE;
      return( key_reported );
    }
  }
  /* Any change while the keypad is either idle or repeating a key
     means that we should debounce. */
  else
  {
    /* Debounce the key if the keypad reports a change. */
    if( last_key != key_reported )
    {
      debounce_timestamp = millis( );
      keypad_state = KEYPAD_STATE_DEBOUNCING;
    }
    /* Otherwise, determine if the key has been held down long enough
       to treat it as repeating. */
    else
    {
      int repeat_time = repeat( );
      if( repeat_time > 0 )
      {
        if( millis( ) > repeating_timestamp + repeat_time )
        {
          repeating_timestamp = millis( );
          return( key_reported );
        }
      }
    }
  }

  return( -1 );
}
