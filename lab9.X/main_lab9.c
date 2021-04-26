/*------------------------------------------------------------------------------
Archivo: mainsproject.s
Microcontrolador: PIC16F887
Autor: Andy Bonilla
Compilador: pic-as (v2.30), MPLABX v5.45
    
Programa: Laboratorio 8, medidor de dos potenciometros
Hardware: PIC16F887
    
Creado: 20 de abril de 2021    
Descripcion: valores pot1 en leds de PORTA, valores pot2 en displays PortC
------------------------------------------------------------------------------*/
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// se declara oscilador interno
#pragma config WDTE = OFF       // Watchdog Timer apagado
#pragma config PWRTE = OFF      // Power-up Timer apagado
#pragma config MCLRE = OFF      // MCLRE apagado
#pragma config CP = OFF         // Code Protection bit apagado
#pragma config CPD = OFF        // Data Code Protection bit apagado
#pragma config BOREN = OFF      // Brown Out Reset apagado
#pragma config IESO = OFF       // Internal External Switchover bit apagado
#pragma config FCMEN = ON      // Fail-Safe Clock Monitor Enabled bit apagado
#pragma config LVP = ON        // low voltaje programming prendido

// CONFIG2
#pragma config BOR4V = BOR40V   // configuración de brown out reset
#pragma config WRT = OFF        // apagado de auto escritura de cÃ?Â³digo

/*-----------------------------------------------------------------------------
------------------------librerias a implementar ------------------------------
-----------------------------------------------------------------------------*/
#include <xc.h>
#include <stdint.h>
#define  _XTAL_FREQ 4000000  //se define el delay con FreqOsc 4Mhz

/*-----------------------------------------------------------------------------
------------------------directivas del compilador------------------------------
-----------------------------------------------------------------------------*/
#define _tmr0_value 237

/*-----------------------------------------------------------------------------
-------------------------------- variables -----------------------------------
-----------------------------------------------------------------------------*/
//se declara una cadena equivalente a la tabla de valores binarios de displays
unsigned char nums_displays[] = 
{
    0b00111111, //0
    0b00000110, //1
    0b01011011, //2
    0b01001111, //3
    0b01100110, //4
    0b01101101, //5
    0b01111101, //6
    0b00000111, //7
    0b01111111, //8
    0b01101111  //9
};
unsigned int a,b,c ; //a para centenas, b para decenas, c para unidades
unsigned int contador = 0; // varible para incrementar
//unsigned int muxeo = 0; //variable que lleva cuenta de la multiplexada

/*-----------------------------------------------------------------------------
-------------------------prototipos de funciones-------------------------------
-----------------------------------------------------------------------------*/
void setup(void);  //funcion para configuracion de registros


void __interrupt() isr(void) //funcion de interrupciones
{
    //si en caso hay interrupcion por overflow del timer 0
    if (T0IF==1)
    { 
        a = ((contador/100)%10) ; // valor del contador lo divide en centenas
        b = ((contador/10)%10) ;  // valor del contador lo divide en decenas
        c = (contador%10) ;        // valor del contador en unidades
        T0IF=0; //se apaga la bandera de interrupcion
    }
    
    //si en caso hay interrupcion por ADC, ADCON0bits.CHS==11
    if (PIR1bits.ADIF==1) 
    {
        if (ADCON0bits.CHS==11)
        //{
            PORTA=ADRESH; //el valor del adresh se pasa a PortA con Leds
        //}
        else
        //{
            contador=ADRESH; //el valor del   
            //PORTA=ADRESH;
        //}       
        PIR1bits.ADIF=0; // se aaga bandera de interrupcion de ADC
    }
}

/*-----------------------------------------------------------------------------
----------------------------- ciclo principal----------------------------------
-----------------------------------------------------------------------------*/

void main(void)  //funcion principal sin retorno
{  
    setup(); //en el main se manda a llamar a las configuraciones
    __delay_ms(20);
    ADCON0bits.GO = 1; //se activa medición inicial
    
    //---------------------loop principal del programa ------------------------
    while(1) //se hace loop infinito mientras sea 1
    {
            
        //este bloque es para hacer el cambio en los tranaistores PortE   
        PORTC = nums_displays[a];
        PORTEbits.RE0 = 1;
        __delay_ms(10);
        PORTEbits.RE0 = 0;
        
        PORTC = nums_displays[b];
        PORTEbits.RE1 = 1;
        __delay_ms(10);
        PORTEbits.RE1 = 0;
          
        PORTC = nums_displays[c];
        PORTEbits.RE2 = 1;
        __delay_ms(10);
        PORTEbits.RE2 = 0;
              
        if (ADCON0bits.GO==0)
        {
            if (ADCON0bits.CHS==11)
            {
                ADCON0bits.CHS=13;
            }
            else
            //{
                ADCON0bits.CHS=11; 
                __delay_us(100);//delay 50us para cambio de canal
                ADCON0bits.GO = 1;
            //}
        }
    }
}

/*-----------------------------------------------------------------------------
--------------------------- configuraciones ----------------------------------
-----------------------------------------------------------------------------*/
void setup(void) //FUNCION PARA CONFIGURACION DE ENTRADAS Y SALIDAS
{
    //CONFIGURACION DE ENTRADAS/SALIDAS DIGITALES
    ANSEL = 0X00;  
    ANSELH = 0b00110000; //pines de entrada analogica 11 y 13
    
    //CONFIGURACION DE ENTRADAS Y SALIDAS EN PUERTOS
    TRISA = 0X00;       // Todo el puerto A es salida
    TRISB = 0b00110000;  // PortB4 y PortB5 son entradas
    TRISC = 0X00;       // Todo el PortC como salida display 7seg
    TRISE = 0x00;       // Todo el PortE como salida de transistores
            
    PORTA = 0X00;       // Todo el puerto A es salida
    PORTB = 0b00110000; // PortB4 y PortB5 son entradas
    PORTC = 0X00;       // Todo el puerto C es salida
    PORTE = 0x00;       // Todo el puerto E es salida 
           
    //CONFIGURACION DEL OSCILADOR
    OSCCONbits.IRCF2=1; //4MHz 110
    OSCCONbits.IRCF1=1; //4MHz 110
    OSCCONbits.IRCF0=0; //4MHz 110
    OSCCONbits.SCS=1;   //configuracion de oscilador interno
    
    //CONFIGURACION DEL TIMER0
    OPTION_REGbits.T0CS=0;
    OPTION_REGbits.PSA=0; //SE HABILITA EL PREESCALER
    OPTION_REGbits.PS2=1; //PREESCALER 1:256 111
    OPTION_REGbits.PS1=1; //PREESCALER 1:256 111
    OPTION_REGbits.PS0=1; //PREESCALER 1:256 111
        
    //CONFIGURACION DE ADC
    ADCON1bits.ADFM = 0 ; // se justifica a la isquierda
    ADCON1bits.VCFG0 = 0 ; // voltajes de referencia
    ADCON1bits.VCFG1 = 0 ; // voltaje de referencia
    
    ADCON0bits.ADCS = 0b01 ; // se usa division de 4us con F/8
    ADCON0bits.CHS = 11; // seleccion de canal 11
    __delay_us(50);         //delay de 50us para que cargue capacitor
    ADCON0bits.ADON = 1 ; // se prende modulo ADC
    
    //CONFIGURACION DE INTERRUPCIONES
    INTCONbits.GIE=1; //se habilitan las interrupciones globales
    INTCONbits.PEIE=1 ; //se prende interrupcion por perifericos
    INTCONbits.T0IE=1; //enable bit de int timer0
    INTCONbits.TMR0IF=0; //se apaga la bandera de int timer0
    INTCONbits.TMR0IE=1; // enable bit de IntOnCHangePortB
    INTCONbits.RBIF=0; // se apaga la bandera de IntOnChangeB  
    //CONFIGURACION INTERRUPCION DEL ADC
    PIE1bits.ADIE = 1 ; //se prende interrupcion por ADC
    PIR1bits.ADIF = 0; // se baja bandera de conversion
    
    return;
}
