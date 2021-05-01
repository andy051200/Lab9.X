/*------------------------------------------------------------------------------
Archivo: mainsproject.s
Microcontrolador: PIC16F887
Autor: Andy Bonilla
Compilador: pic-as (v2.30), MPLABX v5.45
    
Programa: Laboratorio 9, mover servo motores con modulo PWM
Hardware: PIC16F887
    
Creado: 27 de abril de 2021    
Descripcion: motor servo en P1A y P1B
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
#define  _XTAL_FREQ 8000000  //se define el delay con FreqOsc 4Mhz

/*-----------------------------------------------------------------------------
------------------------directivas del compilador------------------------------
-----------------------------------------------------------------------------*/
//#define _tmr0_value 237

/*-----------------------------------------------------------------------------
-------------------------------- variables -----------------------------------
-----------------------------------------------------------------------------*/


/*-----------------------------------------------------------------------------
-------------------------prototipos de funciones-------------------------------
-----------------------------------------------------------------------------*/
void setup(void);  //funcion para configuracion de registros


void __interrupt() isr(void) //funcion de interrupciones
{
    
    //si en caso hay interrupcion por ADC
    if (PIR1bits.ADIF==1) 
    {
        if (ADCON0bits.CHS==0)
        {
            //el valor del servo 1   
            CCPR1L = ADRESH;
            PIR1bits.ADIF=0;
        }
        else 
        //{
            //servo2=ADRESH;//el valor del servo 2
            PORTDbits.RD0=1;
            
            PIR1bits.ADIF=0;
        //}     
    }
}

/*-----------------------------------------------------------------------------
----------------------------- ciclo principal----------------------------------
-----------------------------------------------------------------------------*/

void main(void)  //funcion principal sin retorno
{  
    setup(); //en el main se manda a llamar a las configuraciones
    //ADCON0bits.GO=1; //medicion inicial
    //---------------------loop principal del programa ------------------------
    while(1) //se hace loop infinito mientras sea 1
    {
        if (ADCON0bits.GO ==0)
        {   
            if (ADCON0bits.CHS==0)
            {
                ADCON0bits.CHS=1;
                __delay_us(100);//delay 100us para cambio de canal
                ADCON0bits.GO = 1;
            }
            else
            {
                ADCON0bits.CHS=0; 
                __delay_us(100);//delay 100us para cambio de canal
                ADCON0bits.GO = 1;
            }
        }
        
    }
}

/*-----------------------------------------------------------------------------
--------------------------- configuraciones ----------------------------------
-----------------------------------------------------------------------------*/
void setup(void) //FUNCION PARA CONFIGURACION DE ENTRADAS Y SALIDAS
{
    //CONFIGURACION DE ENTRADAS/SALIDAS DIGITALES
    ANSEL = 0b00000011; //se congigura AN0 y AN1 como entrada analógica  
    ANSELH = 0; //pines de entrada analogica 11 y 13
       
    //CONFIGURACION DE ENTRADAS/SALIDAS ANLAGÓGICAS
    TRISA = 0xFF;   // se define canal de salida
    TRISC= 0;       // se define PortC como salida
    TRISD= 0;       // se define PortC como salida
    PORTC=0;        // se define POrtC como salida
    PORTD=0;        // se define POrtC como salida
    
    //CONFIGURACION DEL OSCILADOR
    OSCCONbits.IRCF2=1; //8MHz 111
    OSCCONbits.IRCF1=1; //8MHz 111
    OSCCONbits.IRCF0=1; //8MHz 111
    OSCCONbits.SCS=1;   //configuracion de oscilador interno
            
    //CONFIGURACION DE ADC
    ADCON1bits.ADFM = 0 ;        // se justifica a la iZquierda
    ADCON1bits.VCFG0 = 0 ;      // voltajes de referencia 5V
    ADCON1bits.VCFG1 = 0 ;      // voltaje de referencia gnd
    
    ADCON0bits.ADCS = 0b10 ;    // se usa division de 4us con F/32
    ADCON0bits.CHS = 0;         // seleccion de canal 11
    ADCON0bits.ADON = 1 ;       // se prende modulo ADC
    __delay_us(50);             //delay de 50us para que cargue capacitor
    //ADCON0bits.ADON = 1 ;       // se prende modulo ADC
    
    
    //CONFIGURACION DEL modulo PWM
    //CCP1 para servo 1
    TRISCbits.TRISC2=1;        //RC2/CCP1 como entrada a motor se desconecta
    TRISDbits.TRISD5=1;        //RC2/CCP1 como entrada a motor se desconecta
    PR2 = 249;                  //configurando el periodo
    
    //CCP1 para servo 1
    CCP1CONbits.P1M = 0b0;      // configuracion de una señales de salida
    CCP1CONbits.CCP1M = 0b1100; //  se configura como modo PWM
    CCPR1L = 0x0f ;         //ciclo de tabajo inicial de la onda cuadradad
    CCP1CONbits.DC1B = 0;   //

    //CCP2 para servo 2
    CCP2CON = 0x0C;      // configuracion de una sola señal
    //CCP2CONbits.DC2B1 =0;      // configuracion de una sola señal
    //CCP2CONbits.CCP2M = 0b1100; //  se configura como modo PWM
    CCPR2L = 100 ;         //ciclo de tabajo inicial de la onda cuadradad
    //CCP2CONbits. = 0;   //
  
    
   
    //configruacion del timer2
    PIR1bits.TMR2IF = 0;     //bajar bandera de interrupcion del timer2
    T2CONbits.T2CKPS = 0b11;  //preescaler del timer2 a tope 1:16
    T2CONbits.TMR2ON = 1;       //se prende el timer
    
    while(PIR1bits.TMR2IF==0);  //ciclo para que nunca se prenda bandera
    PIR1bits.TMR2IF=0;      // se apaga bandera por si las moscas
    TRISCbits.TRISC2 = 0;     //salida del pwm1
    TRISDbits.TRISD5= 0;        // salida del pwm 2
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
