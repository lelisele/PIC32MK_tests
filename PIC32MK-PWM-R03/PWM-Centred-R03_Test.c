//!----------------------------------------------------------------------------
//! electronics-lis
//! CH-2000 Neuchatel
//! info@electronics-lis.com
//! https://electronics-lis.com
//! L. Lisowski December 2021 
//!----------------------------------------------------------------------------
//!----------------------------------------------------------------------------
//! PIC32MK ADC tests  REVISION 03
//!----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
//! check where is <p32mk1024mcf064.h> file on your PC
#include <C:\Program Files\Microchip\xc32\v2.50\pic32mx\include\proc\PIC32MK-MC\p32mk1024mcf064.h>
#include <xc.h>            
#include <sys/attribs.h> 
//!----------------------------------------------------------------------------
//!----------------------------------------------------------------------------
//! This part need to be adapted for your application.
// DEVCFG3
#pragma config PWMLOCK = OFF, FUSBIDIO2 = OFF, FVBUSIO2 = OFF, PGL1WAY = OFF    
#pragma config PMDL1WAY = OFF, IOL1WAY = OFF, FUSBIDIO1 = OFF, FVBUSIO1 = OFF                    
// DEVCFG2
#pragma config FPLLIDIV = DIV_1, FPLLRNG = RANGE_5_10_MHZ, FPLLICLK = PLL_FRC         
#pragma config FPLLMULT = MUL_60, FPLLODIV = DIV_4, VBATBOREN = ON, DSBOREN = ON      
#pragma config DSWDTPS = DSPS32, DSWDTOSC = LPRC, DSWDTEN = OFF, FDSEN = OFF          
#pragma config BORSEL = HIGH, UPLLEN = OFF               
// DEVCFG1
#pragma config FNOSC = FRC, DMTINTV = WIN_127_128, FSOSCEN = OFF, IESO = ON
#pragma config POSCMOD = OFF, OSCIOFNC = ON, FCKSM = CSECME, WDTPS = PS16  
#pragma config WDTSPGM =STOP, WINDIS = NORMAL, FWDTEN = OFF, FWDTWINSZ = WINSZ_25
#pragma config DMTCNT = DMT8, FDMTEN = OFF
// DEVCFG0
#pragma config DEBUG = OFF, JTAGEN = OFF, ICESEL = ICS_PGx2, TRCEN = ON
#pragma config BOOTISA = MIPS32,FSLEEP = OFF, DBGPER = PG_ALL, SMCLR = MCLR_NORM     
#pragma config SOSCGAIN = GAIN_2X, SOSCBOOST = ON, POSCGAIN = GAIN_LEVEL_3
#pragma config POSCBOOST = ON, EJTAGBEN = NORMAL 
// DEVCP
#pragma config CP = OFF                 // Code Protect (Protection Disabled)
//! That are my Configuration Bits version, you need adapt them for your application 
//!----------------------------------------------------------------------------
//!----------------------------------------------------------------------------
/*********************************/
//! Type declarations
/*********************************/
typedef signed char  int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef signed int int32;
typedef unsigned int uint32;
/************************************//**
** Macro Definitions
****************************************/
//! Don't forget !!! the result are uint32_l !!!!!!!!
#define TSTBIT(D,i) (D>>i & 1)
#define SETBIT(D,i) (D |= ((uint32)1<<i))
#define CLRBIT(D,i) (D &= ~((uint32)1 << i))
//#define SETPARA(D,p,i) (D |= (p<<i))
#define REG_SHIFT               4
#define ADC_MAX_LOOPS           100
#define SYSCLK                  120000000L
#define PBCLK                   (SYSCLK/2)
#define UARTspeed               115200
#define TRUE                    1
#define FALSE                   0
/*********************************/
//! Local Variables
/*********************************/
char i, k;
uint32  reg ,pos, New;
volatile uint32 *Val;
uint8   MaxLoops,NbrLoops;
/*********************************/
//! Function prototype
/*********************************/
void initU1ART(void);
void initPWMgen(void);
uint8 initPWMcentred(void);
void SetPara(volatile uint32 * reg, uint32 para, uint32 ParaPos);
/*********************************/
//! main 
void main()
{  
    //! Clock initialization
    /* switch to PLL input (SPLL) */
    SYSKEY = 0x0;                        /* Ensure OSCCON is locked */
    SYSKEY = 0xAA996655;                 /* write Key1 unlock sequence */
    SYSKEY = 0x556699AA;                 /* write Key2 unlock sequence */
    OSCCONCLR = 0x07000600;
    OSCCONSET = 0x0100;
    OSCCONSET = 1;                       /* Start clock switching */
    SYSKEY = 0x0;                        /* OSCCON is relocked */
    // Function initialization
    __builtin_enable_interrupts();  // global interrupt enable
    __XC_UART = 1;                  // printf on the U1ART    
    INTCONbits.MVEC = 1; // Set the interrupt controller for multi-vector mode  
    PMCONbits.ON = 0;               //! no PMP  
    // Function initialization
    PMD5bits.U1MD = 0; //! Enable clock
    initU1ART();
    printf ("\n\r This is my first PWM test \n\r"); 
    initPWMgen();       //! PWM part initialization
    if(initPWMcentred() == TRUE){
        printf ("\n\r initPWMcentred accomplished \n\r");
    } 
    else{
        printf ("\n\r initPWMcentred not accomplished \n\r"); 
    }               
    while (1) 
	{
        while( !U1STAbits.URXDA);   //! wait until data available in RX buffer
        i = U1RXREG;                //1 Hit any key
        while( U1STAbits.UTXBF);    //! wait while TX buffer full
        U1TXREG = i;                //! Echo
        while( !U1STAbits.TRMT);    //! wait for last transmission to finish
        if (i >= '1' && i<='9')     //! U1ART test
        {
            PDC1 = (i-'0')*PHASE1/10; //! i * 10 /100 % calculation
            printf("\n\r Duty=%u \tPhase1=%u  \tNewDuty (1-9)*10 % = ", PDC1, PHASE1);
        }
        else {
            printf("\n\r Argument has to be number");
        }
	}
}
/*********************************/
void initU1ART(void)
{
    // UART init
    // Pins initialization. !!!!! have to be adapted to hardware        
    TRISGbits.TRISG8= 1;       //RG8 digital input
    U1RXRbits.U1RXR = 10;        //SET RX to RG8
    RPG6Rbits.RPG6R = 1;        //SET RG6 to TX  
    // disable UART1 and autobaud, TX and RX enabled only,8N1,idle=HIGH
    U1MODE = 0x0000; 
    SETBIT(U1STA,12);           //! Enable RX 
    SETBIT(U1STA,10);           //! Enable TX             
    U1BRG = (PBCLK/(16*UARTspeed)) - 1;
    //U1BRG = 32;               //! Baud Speed => 115200
    //!Interrupt example   
    IPC9bits.U1RXIP = 3; IPC9bits.U1RXIS = 2; //!set IPL3 and sub-priority2
    U1STAbits.URXISEL = 0;      //!where receive one character

    IPC10bits.U1TXIP = 3; IPC10bits.U1TXIS = 2;//!set IPL3 and sub-priority2
    U1STAbits.UTXISEL = 2;      //!where transmit is empty
    // For the future applications 
    IFS1bits.U1TXIF = 0;        //!< Clear the Transmit Interrupt Flag
    IEC1bits.U1TXIE = 0;        //!< Disble Transmit Interrupts
    IFS1bits.U1RXIF = 0;        //!< Clear the Recieve Interrupt Flag
    IEC1bits.U1RXIE = 0;        //!< Disble Recieve Interrupts
    U1MODEbits.ON = 1;          //!< U1ART ON
}
/*********************************/
void initPWMgen(void)
{
    //! all digital
    ANSELA = 0; ANSELB = 0; ANSELC = 0;ANSELE = 0; ANSELG = 0; ANSELG = 0;
    //! No pull-up resistor => necessairy to define ditital Output 
    CNPUA = 0; CNPUB= 0; CNPUC= 0; CNPUD= 0; CNPUE= 0; CNPUF= 0; CNPUG= 0; 
    CNPUF= 0;
    //! No pull-down resistor => necessairy to define ditital Output
    CNPDA = 0; CNPDB= 0; CNPDC= 0; CNPDD= 0; CNPDE= 0; CNPDF= 0; 
    CNPDG= 0; CNPDF= 0;
     //! No Change Notification (interruption generated by I/O )
    CNCONA = 0;  CNCONB = 0; CNCONC = 0; CNCOND = 0; CNCONE = 0; CNCONF = 0;
    CNCONG = 0;   
    /* Initialize pins as PWM outputs */ 
    //CLRBIT(TRISB,14);  //! PIn B14 as PWMlH  
    //CLRBIT(TRISB,15);  //! PIn B15 as PWMlL
    /* Configure ADCCMPCONx No analog comparators are used.*/
        CM1CON = CM2CON = CM3CON = CM4CON = CM5CON = 0;
    /* DAC disabled */
        DAC1CON = DAC2CON = DAC3CON = 0;
    //! Disable and enable peripheral modules
    PMD2 = 1; //! Ampli op and comparator without clock
    return;
}
/*********************************/
uint8 initPWMcentred(void)
{       
    SETBIT(PB6DIV,15);          //! Output clock is enabled
    SetPara(&PB6DIV,1,0);       //! Peripheral Bus ?6? Clock Divisor = /2
    //! PTCON initialization (Primary timer)
    PTCON = 0;                  //! Initial disable
    SetPara(&PTCON,0,2);        //! PCLKDIV = 1 = Sysclock /2
    //! CHOP initialization (Chop timer)
    CHOP =0;
    //! PWMCON1 initialization
    PWMCON1 = 0;                //! Initial settings
    CLRBIT(PWMCON1,3);          //! MTBS = 0 Primary master is the clock source for the MCPWM module
    SetPara(&PWMCON1,1,10);     //! ECAM=1= Symmetric Center-Aligned mode
    SetPara(&PWMCON1,3,6);      //! Dead Time Compensation mode enabled
    SETBIT(PWMCON1,9);          //! ITB=1 PHASE1 registers provide time base 
    //! IOCON1 initialization
    IOCON1 = 0;                 //! Initial settings
    SETBIT(IOCON1,15);          //! PENH=1 PWM module controls PWM1H pin
    SETBIT(IOCON1,14);          //! PENL=1 PWM module controls PWM1L pin
    CLRBIT(IOCON1,13);          //! PWMxH pin is active-high
    CLRBIT(IOCON1,12);          //! PWMxL pin is active-high
    SetPara(&IOCON1,0,10);      //! PMOD=0>PWM I/O pin pair is in Complementary mode
    PHASE1 = 2000;              //! phase1 PHASEx registers provide time base 
    PDC1 = 1000;                 //! Duty cycles
    DTR1 = 25;                  //! Dead Time Values
    ALTDTR1 = 25;               //!
    SETBIT(PTCON,15);           //! PWM module enable
    return (TRUE);
 }
/*********************************/
void SetPara(volatile uint32 * reg, uint32 para, uint32 ParaPos)
{     
    *reg |= (para<<ParaPos);
    return;
} 

