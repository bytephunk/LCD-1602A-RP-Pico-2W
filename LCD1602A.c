#include <stdio.h>
#include <pico/stdlib.h>
#include <LCD1602A.pio.h> 
#include "LCD1602A.h"

PIO pio;
uint sm;
uint offset;

/**
 * DDRAM 
 */ 
#define LCD_LINE_1_ADDR_BASE 		0x00
#define LCD_LINE_1_ADDR_MAX_SINGLE 	0x4F  	//for single row config  	(79 base 10)
#define LCD_LINE_1_ADDR_MAX_DOUBLE 	0x27  	//for double row config	 	(39 base 10)

#define LCD_LINE_2_ADDR_BASE 		0x40	// 						(64 base 10)
#define LCD_LINE_2_ADDR_MAX	 		0x67 	//						(103 base 10)

/**
 * CGRAM
 */
#define LCD_CGRAM_BASE		0x00	
#define LCD_CGRAM_MAX		0x3F 


void init_pio(){
	
	bool result =  pio_claim_free_sm_and_add_program(&LCD_1602_COMM_program,&pio,&sm,&offset);
	hard_assert(result);
	pio_sm_config sm_config = LCD_1602_COMM_program_get_default_config(offset);
	sm_config_set_clkdiv(&sm_config,LCD_PIO_CLOCK_DIV); 
	sm_config_set_set_pins (&sm_config,LCD_PIN_CLOCK,1);
	sm_config_set_out_pin_base(&sm_config,LCD_PIN_RW);
	sm_config_set_out_pin_count(&sm_config,10);

	sm_config_set_in_pin_base(&sm_config,LCD_PIN_RW+2);
	sm_config_set_in_pin_count(&sm_config,8);
	sm_config_set_in_shift(&sm_config,0,0,0);
	sm_config_set_jmp_pin(&sm_config,LCD_PIN_RW);	
	
    uint init_res = pio_sm_init(pio,sm,offset,&sm_config);
	hard_assert(init_res == PICO_OK);
	
	pio_gpio_init(pio,LCD_PIN_CLOCK);
	for(uint k=0; k < 10 ;k++){
		pio_gpio_init(pio,LCD_PIN_RW + k);
	} 
	
	pio_sm_set_enabled(pio,sm,true);
}

uint32_t pio_data_pack(bool rw,bool rs,uint8_t data){
	uint32_t pindirs ;
	pindirs= (!rw)?0b1111111111:0b0000000011  ;// write pindirs
	uint32_t pindata = data << 1;
	pindata |=rs;
	pindata <<=1;
	pindata |=rw; // first LSB is rw
	pindata <<=10;
	uint32_t datapack = pindata | pindirs;


	return datapack;
}


/**
 * Checks busy status and current address. This is an unsafe call : if 'busy' the address is garbled. 
 * @param address if not null the current address is returned ( value is incorrect if busy )
 * @return true if busy 
 */  
bool get_status(uint8_t* address){  
	uint32_t dpack = pio_data_pack(1,0,0b10000000); //get busy and address function
	pio_sm_put(pio,sm,dpack);
	uint8_t ba = pio_sm_get_blocking(pio,sm);
	bool busy = ba >> 7; // b7 is the busy status 
	if (address!=NULL) *address = ba & 0b01111111; //only [b6:b0] are valid address bits 
	return busy;
}
bool lcd_busy(){
	return get_status(NULL);
}


#define BUSY_SLEEP_MS 10

void pio_sm_put_safe(PIO pio, uint sm,uint32_t dpack){
	bool busy = get_status(NULL);
	if(busy) sleep_ms(BUSY_SLEEP_MS) ;
	pio_sm_put(pio,sm,dpack);
}
/**
 * Debug/Development purpose function
 * Get current memory address from the Address Counter, DDRAM or CGRAM. This is a busy-safe call.Ambiguous return. DDRAM addresses are returned as set. CGRAM addresses return with b[7][6] set to [0][1] as packed by the low level function. The CGRAM return is offset by 0x40 which overlaps with valid CGRAM addresses.
 * @return current address. 
 */ 
 uint8_t lcd_getaddr_current(){
	 uint8_t address;
	 bool busy = get_status(&address);
	 if(busy){
		 sleep_ms(BUSY_SLEEP_MS);
		 busy = get_status(&address);
	 }
	 assert(!busy);
	 
	 return address;
}

void ddram_setaddr(uint8_t addr){
	uint8_t mask = 1<<7;
	assert((addr & mask) == 0); // MSB 1 is reserved, trigger debug assertion if clobbered by address
	addr |= mask;
	uint32_t dpack = pio_data_pack(0,0,addr); 
	pio_sm_put_safe(pio,sm,dpack);
}

void cgram_setaddr(uint8_t addr){
	uint8_t mask 	= 0b11<<6; 		//the 2 MSB are reserved
	assert((addr & mask) == 0); 	//and should be empty, trigger debug assertion otherwise
	addr &= ~mask; 					//erase masked bits from original address in case they aren't 0
	uint8_t padding = 0b01<<6; 		//the 2 MSB as required
	addr |= padding;
	uint32_t dpack = pio_data_pack(0,0,addr); 
	pio_sm_put_safe(pio,sm,dpack);
}
/**
 * Write data to previously selected memory address
 */ 
void data_write(uint8_t value){
	uint32_t dpack=pio_data_pack(0,1,value);
	pio_sm_put_safe(pio,sm,dpack);
}
/**
 * Read data from previously selected memory address
 */ 
uint8_t data_read(){
	uint32_t dpack = pio_data_pack(1,1,0x00);
	pio_sm_put_safe(pio,sm,dpack);
	uint32_t result= pio_sm_get_blocking(pio,sm);
	return result;	
}
 void ddram_set(uint8_t address, uint8_t value){
	ddram_setaddr(address);
	data_write(value); 
}
uint8_t ddram_get(uint8_t address){
	ddram_setaddr(address);
	return data_read();
}
void set_display_control(bool display,bool cursor,bool blink){
	uint8_t data = 1<<3 | display <<2 | cursor<<1 | blink ;
	uint32_t dpack = pio_data_pack(0,0,data);
	pio_sm_put_safe(pio,sm,dpack);
}
void lcd_clear(){
	uint32_t data = 1;
	uint32_t dpack = pio_data_pack(0,0,data);
	pio_sm_put_safe (pio,sm,dpack);
}
void lcd_home (){
	uint8_t data = 0b10;
	uint32_t dpack = pio_data_pack(0,0,data);
	pio_sm_put_safe (pio,sm,dpack);
}

/**
 *	@param  target 		: 	SHIFT_CURSOR=0,SHIFT_SCREEN=1
 *	@param direction 	:	SHIFT_LEFT =0, SHIFT_RIGHT =1
 */
void shift( bool target, bool direction){
	uint8_t data = 0 | (1 <<4 ) | (target <<3)|(direction<<2);
	uint32_t dpack = pio_data_pack(0,0,data);
	pio_sm_put_safe(pio,sm,dpack);
}
void lcd_cursor_shift(bool right){
	shift(0,right);
}
void lcd_scroll(bool right){
	shift(1,right);
}
/**
 * @param data_length ignored only 8 bit data line supported
 * @param line number 0: 1 line  1: 2 lines
 * @param format 0 : 5x8 , 1 : 5 x 11    
 */ 
void function_set(bool data_length, bool line_number, bool format){
	data_length = 1;
	uint8_t data = 0 | (1<<5) | (data_length<<4) | (line_number<<3) | (format<<2);
	uint32_t dpack = pio_data_pack(0,0,data);
	pio_sm_put_safe(pio,sm,dpack);
} 

char lcd_get_char(){
	return data_read();
}
void lcd_put_char(char c){
	data_write(c);  
}
inline static uint8_t ddram_compute(uint line,uint column){
	return ( !line )?column:column+LCD_LINE_2_ADDR_BASE ;
}
char lcd_get_char_at(uint line,uint column){
	return ddram_get(ddram_compute(line,column));
}
 void lcd_put_char_at(uint line,uint column, char c){
	uint8_t ddram_address = ddram_compute(line,column);
	ddram_set(ddram_address,c);
}

void lcd_put_string_at(uint line,uint column,const char* string){
	 uint8_t ddram_address = ddram_compute(line,column);
	 ddram_setaddr(ddram_address);
	 int k=0;
	 while(string[k]!= 0) {
		 data_write(string[k]);
		 k++;
	}
}
void lcd_put_string(const char* string){
	int k=0;
	 while(string[k]!= 0) {
		 data_write(string[k]);
		 k++;
	}
}

void lcd_set_custom_character(uint cc_index,const char def_lines[8]){
	assert(cc_index<8); 
	uint8_t cgram_start = 	LCD_CGRAM_BASE + cc_index * 8;
	uint8_t cgram_end	=	cgram_start + 7;
	assert( cgram_start >= LCD_CGRAM_BASE && cgram_end <= LCD_CGRAM_MAX);
	
	uint8_t mask = 0b111 << 5 ; 		// The 3 MSB are unused
	uint8_t i_mask = ~mask;				// inverted mask
	for(int k=0;k<8;k++){
		char line = def_lines[k];
		assert( (line & mask) == 0);	// The 3 MSB should NOT be used
		line &= i_mask;					// clip them anyway
		cgram_setaddr(cgram_start + k);
		data_write(line);
	}
	
	ddram_setaddr(0x00);//move AC to ddram to avoid issues
	
}

void lcd_config(bool display,uint cursor){
	set_display_control(display,cursor > LCD_CURSOR_HIDE, cursor == LCD_CURSOR_BLINK);
}
void lcd_init(bool single_line){
	init_pio();
	
	function_set(0, !single_line , 0);
	lcd_config(1,LCD_CURSOR_HIDE);
	
	lcd_clear();
}
