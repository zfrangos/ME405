//*************************************************************************************
/** @file adc.cpp
 *    This file contains a very simple A/D converter driver.   
 *
 *  Revisions:
 *    @li 01-15-2008 JRR Original (somewhat useful) file
 *    @li 10-11-2012 JRR Less original, more useful file with FreeRTOS mutex added
 *    @li 10-12-2012 JRR There was a bug in the mutex code, and it has been fixed
 *
 *  License:
 *    This file is copyright 2015 by JR Ridgely and released under the Lesser GNU 
 *    Public License, version 2. It intended for educational use only, but its use
 *    is not limited thereto. */
/*    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 *    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 *    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 *    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 *    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUEN-
 *    TIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
 *    OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
 *    CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, 
 *    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
//*************************************************************************************

#include <stdlib.h>                         // Include standard library header files
#include <avr/io.h>

#include "rs232int.h"                       // Include header for serial port class
#include "adc.h"                            // Include header for the A/D class


//-------------------------------------------------------------------------------------
/** \brief This constructor sets up an A/D converter. 
 *  \details The A/D is made ready so that when a  method such as @c read_once() is 
 *  called, correct A/D conversions can be performed. 
 *  Enables the A/D converter, sets the clock prescaler to a divsion factor of 32,
 *  and selects reference voltage source as AVCC with external capactitor at AREF pin. 
 *  @param p_serial_port A pointer to the serial port which writes debugging info. 
 */

adc::adc (emstream* p_serial_port)
{
	// Defines pointer for serial to inputted parameter
	ptr_to_serial = p_serial_port;
	
	// Enable A/D converter
	ADCSRA |= 1<<ADEN;
	
	// Set clock prescaler to a division factor of 32
	ADCSRA |= 1<<ADPS0;
	ADCSRA &= ~(1<<ADPS1);
	ADCSRA |= 1<<ADPS2;
	
	// Select reference voltage source as AVCC with external capactitor at AREF pin
	ADMUX |= 1<<REFS0;
	ADMUX &= ~(1<<REFS1);

	// Print a handy debugging message
	DBG (ptr_to_serial, "A/D constructor OK" << endl);
}


//-------------------------------------------------------------------------------------
/** @brief   This method takes one A/D reading from the given channel and returns it. 
 *  @details Forces the inputted channel to be a number from 0 to 7 then sets the
 *  ADMUX register to the correct channel. Next it starts the conversion and waits until
 *  it is finished. Finally it stores the result and returns it.
 *  @param   ch The A/D channel which is being read must be from 0 to 7
 *  @return  The result of the A/D conversion
 */

uint16_t adc::read_once (uint8_t ch)
{
	// Initalizes result variable for A/D conversion 
	uint16_t result = 0;
	
	// Clears left 5 bits of channel
	ch &= 0b00000111;
	
	// Clears right 3 bits of ADMUX and sets A/D channel
	ADMUX &= 0b11111000;
	ADMUX |= ch;
	
	// Starts conversion and waits until conversion is complete
	ADCSRA |= 1<<ADSC;
	while(ADCSRA & 0b01000000);
	
	// Stores the result of the conversion
	result = ADC; 
	
	return result;
}


//-------------------------------------------------------------------------------------
/** @brief   This method takes a set number of A/D readings from a given channel and 
 *  averages them.
 *  \details Checks to see if number of samples inputted is above a threshold values
 *  then it takes samples and adds them up. Finally it returns the average of samples taken.
 *  @param   channel The A/D channel which is being read
 *  @param   samples Number of samples to take for averaging reading
 *  @return  Averaged result of A/D readings
 */

uint16_t adc::read_oversampled (uint8_t channel, uint8_t samples)
{
	// Intializes result and temporary samples variable
	uint16_t result = 0;
	uint8_t temp_samples = 0;
	
	// Checks if number of samples exceeds sample cap (assuming max A/D reading)
	// Also saves number of samples in a temporary variable
	if(samples >= 64)
	{
	    samples = 60;
	}
	temp_samples = samples;
	
	// Cumulative sum of A/D readings to be averaged
	for(int i = 0; i < samples; samples--)
	{
	    result += read_once(channel);
	}
		
	// Returns average of A/D readings
	return result/temp_samples;
 }


//-------------------------------------------------------------------------------------
/** \brief   This overloaded operator "prints the A/D converter." 
 *  \details Prints out the value of the ADCSRA, ADMUX registers, and a single reading
 *  of channels 0 through 7 of the A/D. 
 *  @param   serpt Reference to a serial port to which the printout will be printed
 *  @param   a2d   Reference to the A/D driver which is being printed
 *  @return  A reference to the same serial device on which we write information.
 *           This is used to string together things to write with @c << operators
 */

emstream& operator << (emstream& serpt, adc& a2d)
{
	// Prints all the following messages and values
	serpt << PMS ("ADCSRA: ") << ADCSRA << endl 
	      << PMS ("ADMUX: ") << ADMUX << endl
	      << PMS ("ADC0 = ") << a2d.read_once(0) << endl
	      << PMS ("ADC1 = ") << a2d.read_once(1) << endl
	      << PMS ("ADC2 = ") << a2d.read_once(2) << endl
	      << PMS ("ADC3 = ") << a2d.read_once(3) << endl
	      << PMS ("ADC4 = ") << a2d.read_once(4) << endl
	      << PMS ("ADC5 = ") << a2d.read_once(5) << endl
	      << PMS ("ADC6 = ") << a2d.read_once(6) << endl
	      << PMS ("ADC7 = ") << a2d.read_once(7) << endl;
		  
	return (serpt);
}

