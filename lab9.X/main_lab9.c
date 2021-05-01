/*------------------------------------------------------------------------------
Archivo: mainsproject.s
Microcontrolador: PIC16F887
Autor: Andy Bonilla
Compilador: pic-as (v2.30), MPLABX v5.45
    
Programa: Laboratorio 9, mover dos servo motores con modulo PWM
Hardware: PIC16F887, 2 servo motores y 2 potenciometros
    
Creado: 26 de abril de 2021    
Descripcion: motor servo en CCP1 y CCP2, con potenciometros en AN0 y AN1
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
-------------------------prototipos de funciones-------------------------------
-----------------------------------------------------------------------------*/
void setup(void);  //funcion para configuracion de registros del PIC
void __interrupt() isr(void) //funcion de interrupciones
{
    
    //si en caso hay interrupcion por ADC
    if (PIR1bits.ADIF==1)
    {
         //conversion de potenciometro del servo 1   
        if (ADCON0bits.CHS == 0)  
        { 
            CCPR1L = (ADRESH>>1)+124;          //rando de 180°
            CCP1CONbits.DC1B1 = ADRESH & 0b01; //resolucion de movimiento
            CCP1CONbits.DC1B0 = (ADRESL>>7);
            ADCON0bits.CHS = 1;             //se cambia al canal dse conversion
        }
        //conversion del potenciometro del servo2
        else   
        {
            CCPR2L = (ADRESH>>1)+124;           //rando de 180°
            CCP2CONbits.DC2B1 = ADRESH & 0b01;  // resolucion de movimiento
            CCP2CONbits.DC2B0 = (ADRESL>>7);
            ADCON0bits.CHS = 0;                //se cambia a canal de conversion
        } 
        __delay_us(50);         //delay de 50 us para cargar capacitor
        PIR1bits.ADIF = 0;      // se apaga bandera de interrupcion ADC
        ADCON0bits.GO = 1;      // se prende la siguiente conversión
    }
}
/*-----------------------------------------------------------------------------
----------------------------- ciclo principal----------------------------------
-----------------------------------------------------------------------------*/
void main(void)  //funcion principal sin retorno
{  
    setup();                    //Configuraciones generales del PIC
    ADCON0bits.GO=1;            //conversion inicial del ADC
    //---------------------loop principal del programa ------------------------
    while(1){}                  //se hace loop infinito mientras sea  1  
}
/*-----------------------------------------------------------------------------
--------------------------- configuraciones ----------------------------------
-----------------------------------------------------------------------------*/
void setup(void) //FUNCION PARA CONFIGURACION DE ENTRADAS Y SALIDAS
{
    //CONFIGURACION DE ENTRADAS/SALIDAS DIGITALES
    ANSEL = 0b00000011;         //AN0 y AN1 como entrada analógica  
          
    //CONFIGURACION DE ENTRADAS/SALIDAS ANLAGÓGICAS
    TRISA = 0b00000011;         // se define RA0 y RA1 copmo canal de ADC
    TRISC= 0;                   // se define PortC como salida
    PORTA=0;                    // se limpia POrtA
    PORTC=0;                    // se limpia POrtC 
    
    //CONFIGURACION DEL OSCILADOR
    OSCCONbits.IRCF2=1;         //8MHz 111
    OSCCONbits.IRCF1=1;         //8MHz 111
    OSCCONbits.IRCF0=1;         //8MHz 111
    OSCCONbits.SCS=1;           //configuracion de oscilador interno
            
    //CONFIGURACION DE ADC
    ADCON1bits.ADFM = 0 ;       // se justifica a la izquierda
    ADCON1bits.VCFG0 = 0 ;      // voltajes de referencia 5V
    ADCON1bits.VCFG1 = 0 ;      // voltaje de referencia gnd
    ADCON0bits.ADCS = 2 ;       // se usa division de 4us con F/32
    ADCON0bits.CHS = 0;         // seleccion de canal 1
    ADCON0bits.ADON = 1 ;       // se prende modulo ADC
    __delay_us(50);             // delay de 50us para que cargue capacitor
    
    //CONFIGURACION DEL MODULO PWM
    PR2 = 249;                  //configurando el periodo de oscilación
    //CCP1 para servo 1
    TRISCbits.TRISC2=1;         // RC2/CCP1 como entrada a motor se desconecta
    CCP1CONbits.P1M = 0;        // configuracion de una señales de salida
    CCP1CONbits.CCP1M = 0b1100; // se configura como modo PWM
    CCPR1L = 0x0f ;             // ciclo de trabajo inicial de la onda cuadrada
    CCP1CONbits.DC1B = 0;       // LSB para ciclo de trabajo
    //CCP2 para servo 2
    TRISCbits.TRISC1 = 1;       // RC1/CCP2 como entrada a motor se desconecta
    CCP2CONbits.CCP2M = 0b1100; // se configura como modo PWM
    CCPR2L = 0x0f;              // ciclo de trabajo inicial de la onda cuadrada
    CCP2CONbits.DC2B1 = 0;      // LSB para ciclo de trabajo
    
    //CONFIGURACION DEL TIMER2
    PIR1bits.TMR2IF = 0;        // apagar bandera de interrupcion del timer2
    T2CONbits.T2CKPS = 0b11;    // preescaler del timer2 1:16
    T2CONbits.TMR2ON = 1;       //se prende el timer2
    //configuracion del timer2 para el PWM
    while(PIR1bits.TMR2IF==0);  //ciclo para que nunca se prenda bandera
    PIR1bits.TMR2IF=0;          // se apaga bandera por si las moscas
    TRISCbits.TRISC2 = 0;       //salida del pwm1
    TRISCbits.TRISC1= 0;        // salida del pwm 2
    
    //CONFIGURACION DE INTERRUPCIONES
    //interrupcioness globales
    INTCONbits.GIE=1;           //se habilitan las interrupciones globales
    INTCONbits.PEIE=1 ;         //se prende interrupcion por perifericos
    //CONFIGURACION INTERRUPCION DEL ADC
    PIE1bits.ADIE = 1 ;         //se prende interrupcion por ADC
    PIR1bits.ADIF = 0;          // se baja bandera de conversion
    return;
}