/*******************************************************
This program was created by the
CodeWizardAVR V3.12 Advanced
Automatic Program Generator
� Copyright 1998-2014 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : micro processor project (kashan university winter 2018)
Version : 1.0.0
Date    : 1/29/2018
Author  : Alireza Mohammadi
Company : 
Comments: 


Chip type               : ATmega32
Program type            : Application
AVR Core Clock frequency: 8.000000 MHz
Memory model            : Small
External RAM size       : 0
Data Stack size         : 512
*******************************************************/

#include <mega32.h>
#include <delay.h>
#include <glcd.h>
#include <font5x7.h>
#include <stdio.h>
#include "drawings_hex/apple_logo.c"
#include "drawings_hex/kashanu_logo.c"
#include "drawings_hex/circle.c"

//global variables
int delay = 70;
char speed = 10;
unsigned char width = 64;
unsigned char height = 64;
int x = 0, y = 0;
unsigned char step1[4] = {0x03, 0x06, 0x0C, 0x09};
unsigned char step2[4] = {0x30, 0x60, 0xC0, 0x90};
bool drawing_stopped = true; //declares a drawing process is running or not

//shows head location (x, y) on LCD
void show_location()
{
    char buffer[10];
    sprintf(buffer, "(%2d,%2d)", y, x);
    glcd_outtextxy(70, 10, buffer);
}

void clear_rigth_side_of_lcd()
{
    int i, j;
    for (i = 0; i < 64; i++)
    {
        for (j = 0; j < 64; j++)
        {
            glcd_clrpixel(i, j);
        }
    }
}

//move head to location (0, 0)
void take_head_to_0_0()
{
    for (; x > 0; x--)
    {
        PORTC &= 0xF0;
        PORTC |= step1[(x - 1) % 4];
        delay_ms(delay);
        show_location();

        //this if takes motor to zero degree
        if (x == 1)
        {
            PORTC &= 0xF0;
            PORTC |= step1[3];
        }
    }

    for (; y > 0; y--)
    {
        PORTC &= 0x0F;
        PORTC |= step2[(y - 1) % 4];
        delay_ms(delay);
        if (y == 1)
        {
            PORTC &= 0x0F;
            PORTC |= step2[3];
        }
    }
    show_location();
}

//executes after an interrupt (draw new design while micro is drawing another one OR stop drwaing key press)
void reset_system()
{
    glcd_outtextxy(70, 50, "None    ");
    clear_rigth_side_of_lcd();
    take_head_to_0_0();
}

void goto_xy(int x_dest, int y_dest)
{
    int i = 0, j = 0;
    if (x_dest > x)
    {
        for(j = x; j <= x_dest; j++)
        {
            x = j;
            delay_ms(delay);
            PORTC &= 0xF0; //mask second 4 bit
            PORTC |= step1[j % 4];
            
            if(PORTD .6){
                glcd_setpixel(x,y);
            }

            show_location();    
        }
    }
    else if (x_dest < x)
    {
        for(j = x; j >= x_dest; j--)
        {
            x = j;
            delay_ms(delay);
            PORTC &= 0xF0; //mask second 4 bit
            PORTC |= step1[j % 4];
            
            if(PORTD .6){
                glcd_setpixel(x,y);
            }

            show_location();    
        }
    }

    if (y_dest > y)
    {
        for(i = y; i <= y_dest; i++)
        {
            y = i;
            delay_ms(delay);
            PORTC &= 0x0F; //mask second 4 bit
            PORTC |= step2[i % 4];
            
            if(PORTD .6){
                glcd_setpixel(x,y);
            }

            show_location();    
        }
    }
    else if (y_dest < y)
    {
        for(i = y; i >= y_dest; i--)
        {
            y = i;
            delay_ms(delay);
            PORTC &= 0x0F; //mask second 4 bit
            PORTC |= step2[i % 4];
            
            if(PORTD .6){
                glcd_setpixel(x,y);
            }

            show_location();    
        }
    }
}

void increase_speed()
{
    char buffer[2];
    if (delay - 30 >= 70)
    {
        delay -= 30;
        speed++;
        sprintf(buffer, "%2d", speed);
        glcd_outtextxy(70, 30, buffer); //update speed number on LCD
    }
}

void decrease_speed()
{
    char buffer[2];
    if (delay + 30 <= 330)
    {
        delay += 30;
        speed--;
        sprintf(buffer, "%2d", speed);
        glcd_outtextxy(70, 30, buffer);
    }
}

// keyborad scan function
int get_key(void)
{
    DDRB &= 0b10000000; // define B as input (except B7 that is LCD Enable)
    PORTB = 0b00001111; //sets rows of keypad (B0 ~ B3) and clears columns of keypad

    DDRB .4 = true; //set first column (if pressed key is from this column one of the 3 if's below executes)
    if (PINB .0 == 0)
        return 1;
    if (PINB .1 == 0)
        return 4;
    if (PINB .2 == 0)
        return 7;
    if (PINB .3 == 0)
        return 10;
    DDRB .4 = false; //clears column that setted before

    DDRB .5 = true; //like whath happened before but for second column
    if (PINB .0 == 0)
        return 2;
    if (PINB .1 == 0)
        return 5;
    if (PINB .2 == 0)
        return 8;
    if (PINB .3 == 0)
        return 0;
    DDRB .5 = false;

    DDRB .6 = true; //third column
    if (PINB .0 == 0)
        return 3;
    if (PINB .1 == 0)
        return 6;
    if (PINB .2 == 0)
        return 9;
    if (PINB .3 == 0)
        return 11;
    DDRB .6 = false;

    return 255;
}

//returns pixel value of image in a specified column and row
bool find_pixel_bit(int row, int col, flash unsigned char *design)
{
    /*suppose our images are 64*64 and saves in an array with 516 items that each item is 1 byte
    that means each item of array containes 8 pixel, first we must find our pixel is in whath index of array
    arrays and matrixes store in memory as rows, such if suppose an image as matrix, pixel in row=i and col=j is row*64 + col 't pixel
    because in each byte 8 pixel is saved we divide expression by 8. result+4 (first 4 byte is height and width of image) is index of 
    array item, remaining of this expression by 8 is number of bit*/

    int mid_exp = row * 64 + col;
    int index = mid_exp / 8 + 4;
    int bit_number = mid_exp % 8;

    return (design[index] & (1 << bit_number)) >> bit_number;
}

void engraving_design(flash unsigned char *design)
{
    int i, j;
    bool pixel;

    drawing_stopped = false;

    for (i = 0; i < height; i++)
    {

        y = i;

        if (i % 2 == 0)
        {
            for (j = 0; j < width; j++)
            {

                pixel = find_pixel_bit(i, j, design);
                PORTD .6 = pixel;
                if (pixel)
                    glcd_setpixel(j, i);

                x = j;
                delay_ms(delay);
                PORTC &= 0xF0; //mask second 4 bit
                PORTC |= step1[j % 4];

                show_location();

                // after occuration an interrupt this if executes and ends drawing and resets system (takes head to (0, 0) and clear LCD)
                if (drawing_stopped)
                {
                    reset_system();
                    return;
                }
            }
        }
        else
        {
            for (j = width - 1; j >= 0; j--)
            {

                pixel = find_pixel_bit(i, j, design);
                PORTD .6 = pixel;
                if (pixel)
                    glcd_setpixel(j, i);

                x = j;
                delay_ms(delay);
                PORTC &= 0xF0;
                PORTC |= step1[j % 4];

                show_location();

                if (drawing_stopped)
                {
                    reset_system();
                    return;
                }
            }
        }

        delay_ms(delay);
        PORTC &= 0x0F;
        PORTC |= step2[i % 4];

        show_location();
    }
    reset_system();
}

void draw_rectaangle()
{
    goto_xy(10, 10);
     
    PORTD .6 = true;
    goto_xy(50,10);
    goto_xy(50,40);
    goto_xy(10,40);
    goto_xy(10,10);
    PORTD .6 = false;
    
    reset_system();
    
}

// External Interrupt 0 service routine
interrupt[EXT_INT0] void ext_int0_isr(void)
{
    /*Whene interrupt routine executes we know a key is pressed
    but we dont know witch key was that*/

    int key;
    key = get_key(); //here we detect witch key is pressed

    DDRB |= 0b11110000; //port is ready to takes next key press

//active nested interrupts
#asm("sei")

    switch (key)
    {
    case 0:
        drawing_stopped = true;
        break;
    case 1:
        if (!drawing_stopped)
        {
            drawing_stopped = true;
        }
        else
        {
            glcd_outtextxy(70, 50, "Apple     ");
            engraving_design(apple_logo);
        }
        break;
    case 2:
        if (!drawing_stopped)
        {
            drawing_stopped = true;
        }
        else
        {
            glcd_outtextxy(70, 50, "KashanU   ");
            engraving_design(kashanu_logo);
        }
        break;
    case 3:
        if (!drawing_stopped)
        {
            drawing_stopped = true;
        }
        else
        {
            glcd_outtextxy(70, 50, "Rectangle");
            engraving_design(rectangle);
        }
        break;
    case 4:
        if (!drawing_stopped)
        {
            drawing_stopped = true;
        }
        else
        {
            glcd_outtextxy(70, 50, "Circle    ");
            engraving_design(circle);
        }
        break;
    case 10:
        decrease_speed();
        break;
    case 11:
        increase_speed();
        break;
    }
}

void main(void)
{
    char buffer[255]; // A buffer for saving strings that sends to LCD

    //Initialize graphical LCD
    GLCDINIT_t glcd_init_data;
    glcd_init_data.font = font5x7;
    glcd_init(&glcd_init_data);

    //Initialize Ports
    DDRC = 0xFF;        //ports that order motors
    DDRB |= 0b11110000; //keypad port

    /*always is ready to detect keypress
    and give controle to interrupt routine
    to detect witch key is pressed*/
    PORTB |= 0b00001111;
    DDRD .6 = 1;  //Laser Outpur
    PORTD .6 = 0; //Laser is OFF!

    //Print required data on LCD
    glcd_outtextxy(70, 0, "X & Y:");
    glcd_outtextxy(70, 20, "Speed:\n\n");
    glcd_outtextxy(70, 40, "Design:");

    //print primitive (X&Y) , speed and design name on LCD
    show_location();
    sprintf(buffer, "%2d", speed);
    glcd_outtextxy(70, 30, buffer);
    glcd_outtextxy(70, 50, "None    ");

    // External Interrupt(s) initialization
    // INT0: On
    // INT0 Mode: Falling Edge
    // INT1: Off
    // INT2: Off
    GICR |= (0 << INT1) | (1 << INT0) | (0 << INT2);
    MCUCR = (0 << ISC11) | (0 << ISC10) | (1 << ISC01) | (0 << ISC00);
    MCUCSR = (0 << ISC2);
    GIFR = (0 << INTF1) | (1 << INTF0) | (0 << INTF2);

// Global enable interrupts
#asm("sei")

    //Hold AVR ON to takes inputs from keypad   
    draw_rectaangle();
    //while (1)
    //    ;
}