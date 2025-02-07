/*
 * Pin Configuration
 * 
 * Only 8 bit data communication supported for now
 */ 
#define LCD_PIN_CLOCK 	2 //connect this to the 'E' pin of the LCD

//the following pins MUST be consecutive and available to the PIO state machine (e.g. on the same pin bank in some cases).
#define LCD_PIN_RW		4 // Read/Write
#define LCD_PIN_RS		5 // Register Set
#define LCD_PIN_D0		6  //Data pin start
//all in between ...	   // ....	
#define LCD_PIN_D7		13 //Data pin end

/**
 * PIO Clock Divider.
 * The state machine was tested at 20 Khz. The value below is meant to divide the 150 Mhz clock running on a Pico 2.
 */ 
#define LCD_PIO_CLOCK_DIV 7500 // bring down to 20Khz ( T= 50ns)


#define LCD_CURSOR_HIDE 	0
#define LCD_CURSOR_SHOW 	1
#define LCD_CURSOR_BLINK	2

/**
 * Init LCD display. This will configure a PIO SM and launch it, and perform basic configuration. 
 * @param single_line if true set display in single line mode otherwise 2 lines are enabled
 * @param cursor @see lcd_config
 *
 */ 
void lcd_init(bool single_line);

/**
 * Manage display and cursor settings
 * @param display enable display
 * @param cursor setting for cursor  LCD_CURSOR_HIDE / LCD_CURSOR_SHOW / LCD_CURSOR_BLINK
 */ 
void lcd_config(bool display,uint cursor);

/**
 * Get character at current address location.No address validity check performed.
 */ 
char lcd_get_char();

/**
 * Write at current address location. Address counter will be increased.No address validity check performed.
 */ 
void lcd_put_char(char c); 
/**
 * Get character at specified screen location.
 * @param line 0 or 1
 */  
char lcd_get_char_at(uint line,uint column);
/** 
 * Write at specified screen location.
 * @param line 0 or 1
 */ 
 void lcd_put_char_at(uint line,uint column, char c);
 
/**
 * @param line 0 or 1
 * @param column
 * @param string null terminated
 */ 
void lcd_put_string_at(uint line,uint column,const char* string);
/**
 * Write string at current position
 * @param string null terminated
 */ 
void lcd_put_string(const char* string);

/**
 * @return true if device is busy
 */ 
bool lcd_busy();
/**
 * Clear LCD 
 */ 
void lcd_clear();
/**
 * Move cursor to first position
 */ 
void lcd_home();
/**
 * Move cursor one position
 *@param right  direction 0 : left  1: right
 */  
void lcd_cursor_shift(bool right); 
/**
 *@param right direction 0 : left  1: right
 */
void lcd_scroll(bool right);

/**
 * Create a custom character
 * @param cc_index between 0 and 7 
 * @param def_lines array of a line definition character. Only the first 5 LSB should be set.
 */ 
void lcd_set_custom_character(uint cc_index,const char def_lines[8]);
