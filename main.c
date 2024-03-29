#include <avr/io.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>

#define LED_PORT PORTB
#define LED_DDR DDRB
#define LED_PIN PINB
#define LED_PLUS_PIN PB3
#define LED_GND_PIN PB4

uint8_t is_light() {
   uint8_t retval;
   LED_PORT &= ~(1<<LED_PLUS_PIN);          //LED_PLUS_PIN to GND
   LED_PORT |= (1<<LED_GND_PIN);            //LED_GND_PIN to VCC to charge LED
   _delay_us(1);                            //charge up
   LED_DDR &= ~(1<<LED_GND_PIN);            //set pin back to input
   LED_PORT &= ~(1<<LED_GND_PIN);           //disable pullup
   _delay_ms(120);                          //wait for led to discharge
   retval  = !((1<<LED_GND_PIN) & LED_PIN); //read charge
   LED_DDR |= (1<<LED_GND_PIN);             //set LED_GND_PIN back to output
   LED_PORT &= ~(1<<LED_GND_PIN);           //pull LED_GND_PIN back to to GND
   return retval;
}

inline void led_on(){
   LED_PORT |= (1<<LED_PLUS_PIN);
}

inline void led_off(){
   LED_PORT &= ~(1<<LED_PLUS_PIN);
}

volatile int timer_overflow_count = 0;
volatile int daylight = 0;

ISR(TIM0_OVF_vect) {
cli();
if (++timer_overflow_count > 4){
if (is_light()){       //check for light
   daylight++;
      if(daylight > 2){
     wdt_enable(WDTO_4S);
     MCUCR = ~(1<<SM0);
     MCUCR = (1<<SE)|(1<<SM1);
     asm volatile("sleep");
   }
   int i;
   for(i = 0;i<4;i++){ //blink!
      //wdt_reset();
      led_on();
      _delay_ms(50);
      led_off();
      _delay_ms(100);
   }
   }
   timer_overflow_count = 0;
}
sei();
}

int main(void) {
   WDTCR = (1<<WDCE);
   WDTCR = ~(1<<WDE);
   _delay_ms(50);
   ACSR = (1<<ACD);      //disable comperator
   ADCSRA &= ~(1<<ADEN); //disable ADC
   TCCR0B |= (1<<CS02) | (1<<CS00);
   TIMSK0 |=1<<TOIE0;
   LED_DDR |= (1<<LED_GND_PIN) | (1<<LED_PLUS_PIN) | (1<<PB1); //set led as output
   sei();
   //set_sleep_mode(SLEEP_MODE_IDLE);
   //sleep_mode();
   MCUCR = ~(1<<SM0);
   MCUCR = ~(1<<SM1);
   MCUCR = (1<<SE);
   asm volatile("sleep");

   while(1){}
   return 0;
}

