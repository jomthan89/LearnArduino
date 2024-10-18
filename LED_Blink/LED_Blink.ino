#include <Servo.h>

/*
Sattawat Jomthan Example project Blinking LED 4 pattern.
Interface with user via liquid crystal LCD display, 4 buttons and 4 leds.
*/
#include <LiquidCrystal.h> /* Include liquid cLiquidCrystalrystal display library. */

/* Macro PIN assigned. */
#define red_led A0    /* A0 is red led */
#define yellow_led A1 /* A1 is yellow led */
#define green_led A2  /* A2 is green led */
#define blue_led A3   /* A3 is blue led */
/* Declare 4 cap buttons. */
#define cap1 8
#define cap2 9
#define cap3 10
#define cap4 11

/* Macro Constance's declaration. */
#define uint8_t_max 0xFF        /* Unsigned integer 8 bit max value. */
#define uint16_t_max 0xFFFF     /* Unsigned integer 16 bit max value. */
#define uint32_t_max 0xFFFFFFFF /* Unsigned integer 32 bit max value. */

/* PIN assigned. */
//uint8_t cap_button[4] = {8,9,10,11};  /* It's bug now!!! */

/* LCD parameter declaration. */
LiquidCrystal lcd(2, 3, 4, 5, 6, 7); // Creates an LCD object. Parameters: (rs, enable, d4, d5, d6, d7) 

/* Functions Declaration. */
static void Timer_Count_Cp(void);
static void LED_Controller(void);
static void Ext_Communication(void);
static void Pattern_Declare(void);
static void LCD_Controller(void);
static void RYGB_LED_Controller(void);
static void Button_Controller(void);

/* Constance's Declaration. */
const bool LED_flag[4][20] = /* LED pattern */
  {{1,1,0,0,1,1,0,0,0,1,0,1,0,1,0,1,0,1,0,0}, /* Pattern 1 */
   {0,1,0,0,0,1,0,1,0,1,0,0,0,1,0,1,0,0,0,1},  /* Pattern 2 */
   {1,1,0,0,1,1,0,1,0,1,0,0,0,1,1,1,0,0,1,1},  /* Pattern 3 */
   {1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0}}; /*Default Pattern. */


/* Array of bytes */
const byte heart[8] = {
  B00000,          /* B stands for binary formatter and the five numbers are the pixels */
  B01010,
  B11111,
  B11111,
  B01110,
  B00100,
  B00000,
  B00000
};

const byte smile[8] = {
  B00000,
  B00000,
  B01010,
  B00000,
  B10001,
  B01110,
  B00000,
  B00000
};

/* Variable Declaration */
static uint8_t count_led = 0, led_pattern =3;
static char ext_buff = '0';
static uint32_t counter_buff[2] = {1,1}, counter = 0;
volatile bool system_state = 0; /* system flag. */

/* Pattern tag */
enum led_pattern_tag {
  Pattern1,
  Pattern2,
  Pattern3,
  PatternMAX
};

/* Button state flag */
volatile bool bstate[4] = {0,0,0,0}; /* Declare and first initializes buttons state. */

/* the setup function runs once when you press reset or power the board */
void setup() {
  /* initializes digital output pins. */
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(red_led, OUTPUT);
  pinMode(yellow_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(blue_led, OUTPUT);

  /* Initializes digital input pins */
  pinMode(cap1,INPUT);
  pinMode(cap2,INPUT);
  pinMode(cap3,INPUT);
  pinMode(cap4,INPUT);

  /* Initializes the interface to the LCD screen, and specifies the dimensions (width and height) of the display */
  lcd.begin(16,2);
  lcd.createChar(0, heart); /* Create a custom character. */
  lcd.createChar(1, smile);

  /* Second initialize buttons state. */
  /* First reading button state (In case all button are pull-up that must be HIGH state). */  
  bstate[0] = digitalRead(cap4);
  bstate[1] = digitalRead(cap3);
  bstate[2] = digitalRead(cap2);
  bstate[3] = digitalRead(cap1);

  /* initialize serial communication. */
  Serial.begin(115200);
  Pattern_Declare();

  /*First LCD message*/
  lcd.clear();
  lcd.setCursor(0,0); /* Set Cursor (column, Row) */
  lcd.print("SAWADDEE");
  lcd.setCursor(0,1); /* Set Cursor (column, Row) */
  delay(400);
  for(uint8_t k = 0 ; k < 16 ; k++)
  {
    lcd.setCursor(k, 1); /* move cursor to right */
    lcd.write(byte(1)); /* Display the custom character 1, or the smile */
    if(system_state == 0)
    {
      digitalWrite(red_led,HIGH);
      digitalWrite(yellow_led,HIGH);
      digitalWrite(green_led,HIGH);
      digitalWrite(blue_led,HIGH);
      system_state = 1;
    }else
    {
      digitalWrite(red_led, LOW);
      digitalWrite(yellow_led, LOW);
      digitalWrite(green_led, LOW);
      digitalWrite(blue_led, LOW);
      system_state = 0;
    }
    delay(300); /* delay 300 ms */
  }

/* First Jobs. */
  lcd.clear(); /* Clear display */
  lcd.setCursor(0,0); /* Set Cursor (column, Row) */
  lcd.print("LED Blinking."); /* Print messages on the LCD */
  lcd.setCursor(0,1); /* Set Cursor (column, Row) */
  lcd.print("Default Pattern."); /* Print messages on the LCD */
}

/* the loop function runs over and over again forever */
void loop() {
  Button_Controller();
  Ext_Communication();
  Timer_Count_Up();
  LED_Controller();
  //LCD_Controller();
}

static void Timer_Count_Up()
{
  counter = millis();
  if(counter == uint32_t_max) /* Reach max value */
  {
    /* Reset counter!! */
    counter = 0;
    counter_buff[0] = 1;
    counter_buff[1] = 1;
  }
  if((counter - counter_buff[0]) > 100) /* 100 ms counter. */
  {
    counter_buff[0] = counter;
    count_led++; /* Count every 100ms */
  }
  if((counter - counter_buff[1]) > 500) /* 500 ms Counter */
  {
    counter_buff[1] = counter;
  }
}

static void LED_Controller()
{
  if(count_led >= 10)
    count_led = 0; /* Reset counting */

  digitalWrite(LED_BUILTIN, LED_flag[led_pattern][count_led]);

   /* Blink color led according to built in LED */
  if(bstate[0] == 1 && bstate[1] == 1 && bstate[2] == 1 && bstate[3] == 1)
  {
    RYGB_LED_Controller();
  }
}

static void Ext_Communication()
{
  if (Serial.available() > 0) /* Got data from COM */
  {
    ext_buff = Serial.read(); /* Get serial value */
    Serial.println(ext_buff);

    switch(ext_buff){
    case '\0':
            /* Do nothing. */
            break;
    case '\n':
            /* Do nothing. */
            break;
    case '0':
            led_pattern = PatternMAX;
            Serial.println("THE LED is blinking default pattern.");
            digitalWrite(red_led,     LOW);
            digitalWrite(yellow_led,  LOW);
            digitalWrite(green_led,   LOW);
            digitalWrite(blue_led,    LOW);
            Pattern_Declare();
            break;
    case '1': 
            led_pattern = Pattern1;
            Serial.println("THE LED is blinking pattern 1.");
            digitalWrite(red_led,     LOW);
            digitalWrite(yellow_led,  LOW);
            digitalWrite(green_led,   LOW);
            digitalWrite(blue_led,    LOW);
            Pattern_Declare();
            break;
    case '2': 
            led_pattern = Pattern2;
            Serial.println("THE LED is blinking pattern 2.");
            digitalWrite(red_led,     LOW);
            digitalWrite(yellow_led,  LOW);
            digitalWrite(green_led,   LOW);
            digitalWrite(blue_led,    LOW);
            Pattern_Declare();
            break;
    case '3': 
            led_pattern = Pattern3;
            Serial.println("THE LED is blinking pattern 3.");
            digitalWrite(red_led,     LOW);
            digitalWrite(yellow_led,  LOW);
            digitalWrite(green_led,   LOW);
            digitalWrite(blue_led,    LOW);
            Pattern_Declare();
            break;
    default: 
            led_pattern = PatternMAX;
            Serial.println("THE LED is blinking default pattern.");
            digitalWrite(red_led,     LOW);
            digitalWrite(yellow_led,  LOW);
            digitalWrite(green_led,   LOW);
            digitalWrite(blue_led,    LOW);
            Pattern_Declare();
            break;
    }
  }
}

static void Pattern_Declare ()
{
  Serial.println("===================================================================");
  Serial.println("Choose LED blinking pattern.");
  Serial.println("Default blinking pattern  : 1,1,1,1,1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0");
  Serial.println("Pattern 1                 : 1,1,0,0,1,1,0,0,1,1,1,1,1,0,0,0,0,0,1,0");
  Serial.println("Pattern 2                 : 1,1,1,1,1,0,0,0,0,0,1,1,1,0,0,0,1,1,0,0");
  Serial.println("Pattern 3                 : 1,1,0,0,1,1,0,1,0,1,0,0,0,1,1,1,0,0,1,1");
  Serial.print("Choose pattern: ");
}

static void LCD_Controller()
{
  lcd.clear(); /* Clear display */
  lcd.setCursor(0,0); /* Set Cursor (column, Row) */
  lcd.print("LED Blinking."); /* Print messages on the LCD */
  lcd.setCursor(0,1); /* Set Cursor (column, Row) */
  switch(led_pattern){
    case Pattern1:
            lcd.print("Pattern 1."); /* Print messages on the LCD */
            break;
    case Pattern2:
            lcd.print("Pattern 2."); /* Print messages on the LCD */
            break;
    case Pattern3:
            lcd.print("Pattern 3."); /* Print messages on the LCD */
            break;
    default:
            lcd.print("Default Pattern."); /* Print messages on the LCD */
  }
}

static void RYGB_LED_Controller()
{
  switch(led_pattern){
    case Pattern1:
            digitalWrite(yellow_led, LED_flag[led_pattern][count_led]);
            break;
    case Pattern2:
            digitalWrite(green_led, LED_flag[led_pattern][count_led]);
            break;
    case Pattern3:
            digitalWrite(blue_led, LED_flag[led_pattern][count_led]);
            break;
    default:
            digitalWrite(red_led, LED_flag[led_pattern][count_led]);
  }
}

static void Button_Controller()
{
  bstate[0] = digitalRead(cap4);
  bstate[1] = digitalRead(cap3);
  bstate[2] = digitalRead(cap2);
  bstate[3] = digitalRead(cap1);

  /* Blink pattern judgment from button. */
  if(bstate[0] == 0)
  {
    digitalWrite(red_led,     HIGH);
    digitalWrite(yellow_led,  LOW);
    digitalWrite(green_led,   LOW);
    digitalWrite(blue_led,    LOW);
    led_pattern = PatternMAX;
    Serial.println("THE LED is blinking default pattern.");
    Pattern_Declare ();
    LCD_Controller();
  }
  if(bstate[1] == 0)
  {
    digitalWrite(red_led,     LOW);
    digitalWrite(yellow_led,  HIGH);
    digitalWrite(green_led,   LOW);
    digitalWrite(blue_led,    LOW);
    led_pattern = Pattern1;
    Serial.println("THE LED is blinking pattern 1.");
    Pattern_Declare ();
    LCD_Controller();
  }
  if(bstate[2] == 0)
  {
    digitalWrite(red_led,     LOW);
    digitalWrite(yellow_led,  LOW);
    digitalWrite(green_led,   HIGH);
    digitalWrite(blue_led,    LOW);
    led_pattern = Pattern2;
    Serial.println("THE LED is blinking pattern 2.");
    Pattern_Declare ();
    LCD_Controller();
  }
  if(bstate[3] == 0)
  {
    digitalWrite(red_led,     LOW);
    digitalWrite(yellow_led,  LOW);
    digitalWrite(green_led,   LOW);
    digitalWrite(blue_led,    HIGH);
    led_pattern = Pattern3;
    Serial.println("THE LED is blinking pattern 3.");
    Pattern_Declare ();
    LCD_Controller();
  }
}