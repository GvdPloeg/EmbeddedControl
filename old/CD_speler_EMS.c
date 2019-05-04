#include<avr\io.h>
#include<util\delay.h>
#include<avr\interrupt.h>
#include "dplyfunc.h"

uint8_t state = 1;
uint8_t paused_state = 0;
#define not_blank		0	//paused_state
#define blank			1	//paused_state
#define paused			2	//state
#define event_ADCW_1to10	(buttons_adcw() > 0   && buttons_adcw() < 889)	//

uint8_t read_ADC(){
    ADCSRA |= 0x40;			  // Start conversie
    while ( ADCSRA & 0x40 );  // Wacht op conversie
    return ADCW;              // Lees resultaat
}

void delay_ms(uint16_t ms)
{
	for(uint16_t t=0; t<=ms; t++)
	_delay_ms(1);
}

uint8_t buttons_pinf() {
	uint8_t bpf_return_val = 0xFF;  	//Return value, event number
	uint8_t input;              //input waarde (PORTF)
	uint8_t input_oud = 0xFF;	//input memory
	uint16_t usec = 20000;	  	//microseconde teller
	//Check op constant signaal gedurende circa 20 ms
	input = ((PINF & 0x0E)>>1);
	while(((PINF & 0x0E)>>1)==input && --usec > 0) {
		_delay_us(1);
	}
	if(usec ==0) {			  //Constant signaal gedurende 20ms
		if(input != input_oud) {  //Flank
			input_oud = input;
			bpf_return_val = input;
		}
	}
	return bpf_return_val;
}

uint8_t buttons_adcw() {
	uint16_t badcw_return_val = 0xFFFF;  	//Return value, event number
	uint16_t input;              //input waarde (analog)
	uint16_t input_oud = 0xFFFF;	//input memory
	uint16_t usec = 20000;	  	//microseconde teller

	//Check op constant signaal gedurende circa 20 ms
	input = (ADCW);
	while((ADCW)==input && --usec > 0) {
		_delay_us(1);
	}
	if(usec ==0) {			  //Constant signaal gedurende 20ms
		if(input != input_oud) {  //Flank
			input_oud = input;
			badcw_return_val = input;
		}
	}
	return badcw_return_val;
}
volatile uint16_t teller = 0;
volatile uint8_t pause_timer = 0;
volatile uint8_t start_pt = 0;

volatile uint8_t start_timer_plus10 = 0;
uint16_t teller_plus10 = 0;
uint8_t tens = 0;
//uint8_t disable_main_display = 0;
ISR(TIMER0_OVF_vect){
	if(state == paused){
		start_pt = 1;
		if(start_pt == 1){
			if(paused_state == 0){
				if(teller<30){
					teller++;
				}
				else if(teller==30){
					paused_state = 1;
				}
			}
			else if(paused_state == 1){
				if(teller<90){
					teller++;
				}
				else if(teller==90){
					teller=0;
					paused_state =0;
				}
			}
		}
		else paused_state = 0;
	}
	else start_pt = 0;
	
	if(start_timer_plus10 ==1){
		if(teller_plus10<244){
			teller_plus10++;
		}
		else if(teller_plus10==244 || event_ADCW_1to10){
			tens=0;
			//disable_main_display = 0;
			teller_plus10=0;
			start_timer_plus10=0;
		}
	}		
}
// Hoofdprogramma
int main(void)
{
	// Bepaal per bit de IO-richting van de poorten
	DDRA = 0x30; //0011 0000  b6,7 not used
	DDRB = 0xFE; //1111 1110
	DDRC = 0x3D;
	DDRD = 0xFF; //1111 1111
	DDRE = 0x07; //0000 0111

	// Zet uitgangen laag (alles uit)
	PORTA = 0x0E; //A1,A2,A3: pull-up
	PORTB = 0x01; //B0: pull-up
	PORTC = 0xC2;
	PORTD = 0;
	PORTE = 0;

	//Prescalers
	TCCR0A = 0b00000101;
	TIMSK0 = 0b00000001;
	sei();

 	// Declaraties
	uint8_t cd_state = 0;
	uint8_t track = 0;
	uint8_t	input_f= 0xFF;
	uint8_t maxtrack = 0;
	uint8_t maxtrack_scanned = 0;
	uint16_t apv = 0; //analog pulse value

	//Defines
	#define no_cd_loaded	0	//cd_state
	#define cd_loaded		1	//cd_state

	#define playing			0	//state
	#define stopped			1	//state

	#define event_cd_ready  		(PINC & 0x02)
	#define event_play				(input_f == 6)
	#define event_next_track		(input_f == 2)
	#define event_stop				(input_f == 4)
	#define event_previous_track	(input_f == 3)
	#define event_end_of_track		(PINC & 0x80)
	#define event_end_of_cd			(PINC & 0x40)
	#define event_pause				(input_f == 5)

	#define event_ADCW_1    	(apv > 0   && apv < 50)		//change apv to ADCW inbetween 
	#define event_ADCW_2      	(apv > 50  && apv < 142)	//parenthesis to change from 
	#define event_ADCW_3      	(apv > 142 && apv < 234)	//edge- to constant detection
	#define event_ADCW_4      	(apv > 234 && apv < 326)	//
	#define event_ADCW_5      	(apv > 326 && apv < 418)	//
	#define event_ADCW_6      	(apv > 418 && apv < 521)	//
	#define event_ADCW_7      	(apv > 521 && apv < 613)	//
	#define event_ADCW_8      	(apv > 613 && apv < 705)	//
	#define event_ADCW_9      	(apv > 705 && apv < 797)	//
	#define event_ADCW_1to9		(apv > 0   && apv < 797)	//
	#define event_ADCW_10      	(apv > 797 && apv < 889)	//
	#define event_ADCW_plus10   (apv > 889 && apv < 982)	//
	#define event_ADCW_none		(ADCW > 982 && ADCW < 1055)	//
	#define last	maxtrack

	#define led0D 0x01
	#define led1D 0x02
	#define led2D 0x04
	#define led3D 0x08
	#define led4D 0x10
	#define led5D 0x20
	#define led5A 0x2E
	
	cd_state = no_cd_loaded;
	
	
	while (1) {
	input_f = buttons_pinf();
	apv = buttons_adcw();
		switch(cd_state){
			case no_cd_loaded:
				PORTD = led0D;
				if(event_cd_ready){
					track = 1;
					if(maxtrack_scanned == 0){
						dply_number(maxtrack);
						if(event_end_of_track){
							maxtrack++;
							while(event_end_of_track){}
						}
						else if(event_end_of_cd){
							maxtrack++;
							state = stopped;
							cd_state = cd_loaded;
						}
					}
					else state = stopped;
				}
			break;
			case cd_loaded:
				maxtrack_scanned = 1;		//sets maxtrack as scanned.
				
			/*	if(disable_main_display == 1){ 
					dply_ch(1,tens);
					dply_ch(0,0x80);
				}
				else if(paused_state != blank){
					dply_number(track);
				} */
				if(paused_state != blank /*&& disable_main_display != 1*/){
					dply_number(track);
				}
				
				if(/*!event_ADCW_none &&*/ (state == playing) || state == playing || state == paused ){	//trackchoice
				PORTD |= led5D; 
					if(event_ADCW_plus10){
						tens++; 
						start_timer_plus10 = 1;
					}
					if(start_timer_plus10 == 1){
						if(event_ADCW_1to10){
							if(event_ADCW_1)
								track=1+tens*10;
							if(event_ADCW_2)
								track=2+tens*10;
							if(event_ADCW_3)
								track=3+tens*10;
							if(event_ADCW_4)
								track=4+tens*10;
							if(event_ADCW_5)
								track=5+tens*10;
							if(event_ADCW_6)
								track=6+tens*10;
							if(event_ADCW_7)
								track=7+tens*10;
							if(event_ADCW_8)
								track=8+tens*10;
							if(event_ADCW_9)
								track=9+tens*10;
							if(event_ADCW_10)
								track=10+tens*10;
						}
						else {
							//disable_main_display = 1;
							dply_ch(1,tens);
							dply_ch(0,0x80);
						}
					}	
					else if(start_timer_plus10 == 0){
						if(event_ADCW_1)
							track=1;
						if(event_ADCW_2)
							track=2;
						if(event_ADCW_3)
							track=3;
						if(event_ADCW_4)
							track=4;
						if(event_ADCW_5)
							track=5;
						if(event_ADCW_6)
							track=6;
						if(event_ADCW_7)
							track=7;
						if(event_ADCW_8)
							track=8;
						if(event_ADCW_9)
							track=9;
						if(event_ADCW_10)
							track=10;		
					}		
				}
				if(!event_cd_ready){
					track = 0;
					cd_state = no_cd_loaded;
				}
				if((event_next_track && track == last) || (event_previous_track && track == 1)){
					track = 1;
					state = stopped;
				}
				else if((event_next_track) && (track != last)){
					track++;
				}
				else if(event_previous_track && (track > 1)){
					track--;
				}
				else if(event_end_of_track && !event_end_of_cd){
					track++;
					while(event_end_of_track && !event_end_of_cd){}
				}

				switch(state){
					case playing:
						PORTD = led1D;
						if(event_stop || (event_end_of_track && event_end_of_cd)){
							track = 1;
							state = stopped;
						}
						else if(event_pause){
							paused_state = not_blank;
							state = paused;
						}
						break;
					case stopped:
						PORTD = led2D;
						track = 1;
						if(event_play || event_next_track || event_previous_track){
							state = playing;
						}
						break;

					case paused:
						if(event_stop){
							track = 1;
							state = stopped;
						}
						else if(event_play || event_pause){
							state = playing;
						}
						switch(paused_state){
							case not_blank:
								PORTD = led3D;
							//	while(pause_timer != 1){}
							//	dply_number(track);
								break;
							case blank:
								PORTD = led4D;
							//	while(pause_timer != 0){}
							//	dply_blank();
								break;
						}
						break;
					} //end switch(state)
					break; //break case playing	

		} // end switch(cd_state)
	} // end while(1)
} // end main
