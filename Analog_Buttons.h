/** @file Analog_Buttons.h
 *
 * Library to decode a number of digital buttons that are connected to 
 * a single analog input using a resistor ladder.  See the main source
 * code for documentation.
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

#ifndef _ANALOG_BUTTONS_H
#define _ANALOG_BUTTONS_H


class Analog_Buttons
{
  public:

    Analog_Buttons( int pin_number, int number_of_buttons );
    void repeat( int milliseconds )
    {
      repeat_milliseconds = milliseconds;
    }
    void debounce( int milliseconds )
    {
      debounce_milliseconds = milliseconds;
    }
    operator int( )
    {
      return( read_key( ) );
    }


  private:

    enum keypad_state_t { KEYPAD_STATE_IDLE, KEYPAD_STATE_DEBOUNCING };

    int determine_button( ) const;
    int read_key( );
    int repeat( ) const
    {
      return( repeat_milliseconds );
    }
    int debounce( ) const
    {
      return( debounce_milliseconds );
    }

    const int   pin;
    const int   number_buttons;
    int repeat_milliseconds;
    int debounce_milliseconds;

    /* State machine variables. */
    int last_key;
    enum keypad_state_t keypad_state;
    unsigned long debounce_timestamp;
    unsigned long repeating_timestamp;
};


#endif /* _ANALOG_BUTTONS_H */
