
#include <stdio.h>
#include <pico/stdlib.h>
#include <LCD1602A.pio.h>
#include "LCD1602A.h"
#include "LCD1602A_5x8_custom_chars.h"


int main(int argc, char **argv)
{
	stdio_init_all();
	
	printf("Start LCD\n");
	
	lcd_init(false); // initialize LCD
	
	
	lcd_put_string("The quick brown fox jumps over the lazy dog."); //string wraps to the second line
	sleep_ms(1000);
	for(int k=1;k< 30;k++){
		lcd_scroll(0);
		sleep_ms(200);
	}
	sleep_ms(1000);
	lcd_home();
	sleep_ms(3000);
	lcd_clear();
	
	//add some custom characters
	//you can customize 8 characters ( 0 to 7 )
	lcd_set_custom_character(0,LCD_CHAR_SMILEY);
	lcd_set_custom_character(1,LCD_CHAR_STICK_MAN);
	
	//custom characters are mapped to codes 0 - 7 and again to 8 - 15.
	lcd_put_char(0);		// code 0 is mapped to custom character 0
	lcd_cursor_shift(1); 	// move cursor right
	lcd_put_char(1);		// custom character 1
	lcd_put_char('*'); 		// print a star 
	lcd_put_char(' '); 		// print space
	lcd_put_char(2);		// character code 2 is uncustomized yet so it should print either blank or a default character.
	lcd_config(true,LCD_CURSOR_BLINK);//make cursor blink
	lcd_put_char_at(1,0,'>'); //print first cell on the bottom line
	sleep_ms(1000);
	lcd_cursor_shift(0); //move cursor back
	for(int k=0xc0; k<=0xcf;k++){ // the LCD comes preloaded with many characters besides ascii. These codes should print some katakana.
		lcd_put_char(k); 
	}
	
	sleep_ms(1000);
	lcd_config(false,0);// display off 
	sleep_ms(1000);
	lcd_config(true,0); // display on again 
	lcd_home(); //move to beginning.
	sleep_ms(3000);
	lcd_config(true,LCD_CURSOR_SHOW); // cursor as line
	lcd_clear();
	lcd_put_string_at(0,6,"Bye!");
	
	printf("Done!\n");
	return 0;
}

