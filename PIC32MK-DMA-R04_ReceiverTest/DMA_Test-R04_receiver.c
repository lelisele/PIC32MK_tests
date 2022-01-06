//!----------------------------------------------------------------------------
//! electronics-lis
//! CH-2000 Neuchatel
//! info@electronics-lis.com
//! https://electronics-lis.com
//! L. Lisowski 20 Decembre 2021
//!----------------------------------------------------------------------------
//!----------------------------------------------------------------------------
// PIC32MK ADC tests
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
//! These Configuration Bits need to be adapted for your HW and SW application.
//! This is FRC version without Quartz/Oscillator
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
#define SETPARA(D,p,i) (D |= (p<<i))
#define SYSCLK                  120000000L
#define PBCLK                   (SYSCLK/2)
#define UARTspeed               115200
#define TRUE                    1
#define FALSE                   0
/*********************************/
//! Local Variables
/*********************************/
//! Extremely important. Only this form  is working
#define VirtToPhys(p) (int)p<0?((int)p&0x1fffffffL):(unsigned int)((unsigned char*)p+0x40000000L)
/*********************************/
//! Global data
/*********************************/
#define RX_BUF_SIZE  30
char rxBuf[RX_BUF_SIZE];    
int rx_number;              // Global data
char i;
char MessageBuf[] = "  Buffer => ";
char MessageUART[] = "  UART1 receiver => ";  
/*********************************/
//! Function prototype
/*********************************/
void initU1ART(void);
void initGen(void);
void initDMA_global(void);
void initDMA0RX(void);
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
    __builtin_enable_interrupts();      // global interrupt enable
    __XC_UART = 1;                      // printf on the U1ART    
    INTCONbits.MVEC = 1;                // Set the interrupt controller for multi-vector mode  
    PMCONbits.ON = 0;                   //! no PMP
    
    // Function initialization
    PMD5bits.U1MD = 0; //! PERIPHERAL MODULE DISABLE
    initU1ART();
    printf ("\n\r This is my first DMA receiver test \n\r UART1 receiver => ");
    rx_number = 0;
    initGen();       //! PWM part initialization 
    initDMA_global();
    initDMA0RX();
    /*while (1) 
	{       
        while( !U1STAbits.URXDA);   //! wait until data available in RX buffer
        i = U1RXREG;
        while( U1STAbits.UTXBF);    //! wait while TX buffer full
        U1TXREG = i;                //! Echo
        while( !U1STAbits.TRMT);    //! wait for last transmission to finish       
	}*/
    while(1)
    { 
        //! Hit ant key and finish with CR to display message
        //! I'm using receiving "one byte by one byte" to be accumulate and
        //! and applied to Command Interpreter
    }
}
/*********************************/
/*********************************/
void initU1ART(void)
{
    // UART init
    // Pins initialization. !!!!! have to be adapted to hardware        
    TRISGbits.TRISG8= 1;       //RG8 digital input
    U1RXRbits.U1RXR = 10;      //SET RX to RG8
    RPG6Rbits.RPG6R = 1;      //SET RG6 to TX 
    // disable UART1 and autobaud, TX and RX enabled only,8N1,idle=HIGH
    U1MODE = 0x0000;         
    U1STAbits.URXEN = 1; U1STAbits.UTXEN = 1;    //!Enable RX and TX
    U1MODE = 0x0000; 
    //SETBIT(U1STA,12);           //! Enable RX 
    //SETBIT(U1STA,10);           //! Enable TX             
    U1BRG = (PBCLK/(16*UARTspeed)) - 1;
    //U1BRG = 0x40;               //! Baud Speed => 115200
    // Interrupt         
    SetPara(&IPC9,1,26);    //! IPC9bits.U1RXIP = 1  priority ==>> vector 39
    SetPara(&IPC9,0,24);    //! IPC9bits.U1RXIS = 0  sub-priority    
    SetPara(&U1STA,0,6);    //! Interrupt flag bit is asserted while receive buffer is not empty
    //! Interruption enable
    CLRBIT(IFS1,7);             //! Clear UART1 RX interruption flag  
    SETBIT(IEC1,7);             //! Enable UART1 RX interruption 
    U1MODEbits.ON = 1;          //!< U1ART ON
}
/*********************************/
/*********************************/
void initGen(void)
{
    //! all digital
    ANSELA = 0; ANSELB = 0; ANSELC = 0;ANSELE = 0; ANSELG = 0; 
    //! No pull-up resistor => necessairy to define ditital Output 
    CNPUA = 0; CNPUB= 0; CNPUC= 0; CNPUD= 0; CNPUE= 0; CNPUF= 0; CNPUG= 0; 
    CNPUF= 0;
    //! No pull-down resistor => necessairy to define ditital Output
    CNPDA = 0; CNPDB= 0; CNPDC= 0; CNPDD= 0; CNPDE= 0; CNPDF= 0; 
    CNPDG= 0; CNPDF= 0;
     //! No Change Notification (interruption generated by I/O )
    CNCONA = 0;  CNCONB = 0; CNCONC = 0; CNCOND = 0; CNCONE = 0; CNCONF = 0;
    CNCONG = 0;    
    /* Configure ADCCMPCONx No analog comparators are used.*/
        CM1CON = CM2CON = CM3CON = CM4CON = CM5CON = 0;
    /* DAC disabled */
        DAC1CON = DAC2CON = DAC3CON = 0;
    //! Disable and enable peripheral modules
    PMD2 = 1; //! Ampli op and comparator without clock
    return;
}
/*********************************/
/*********************************/
void SetPara(volatile uint32 * reg, uint32 para, uint32 ParaPos)
{     
    *reg |= (para<<ParaPos);
    return;
}
/*********************************/
/*********************************/
void initDMA_global(void)       //! Turns on the DMA controller
{
    DMACONbits.ON = 1;
    // Is it necessary ??????
    DCH2CONbits.CHEN = 0;     // Enable channel 
    DCH3CONbits.CHEN = 0;     // Enable channel
    DCH4CONbits.CHEN = 0;     // Enable channel
    DCH5CONbits.CHEN = 0;     // Enable channel
    DCH6CONbits.CHEN = 0;     // Enable channel
    DCH7CONbits.CHEN = 0;     // Enable channel
}
/*********************************/
void initDMA0RX(void)               //! This is the DMA setup for the receiver 
{
 //   DCH0CONbits.CHAEN = 1;        //! Channel is continuously enabled,
    DCH0CONbits.CHEN = 1;           // Enable channel
    IEC2bits.DMA0IE = 0;            // Initial interrupt disable
    IFS2bits.DMA0IF = 0;            // Initial interrupt flag CLR
    DCH1CONbits.CHPRI = 1;          // Channel priority
    DCH0ECON = 0;   
    DCH0ECONbits.CHSIRQ = 39;       // transfer init by U1ART RX not empty see tab. 8-3
    DCH0ECONbits.SIRQEN = 1;        // event enable
    /*DCH0ECONbits.PATEN = 1;
    DCH0CONbits.CHPATLEN = 0;
    DCH0DATbits.CHPDAT = '\r';  */  // 0x0D = CR (carriage return)*/
    DCH0SSA = VirtToPhys((void*)&U1RXREG);              // Source address
    DCH0DSA = VirtToPhys((void*)&rxBuf[rx_number]);     // Destination 
    DCH0SSIZ = 1;                        
    DCH0DSIZ = RX_BUF_SIZE;
    DCH0CSIZ = 1;
    DCH0INT = 0;                    // clear existing events, disable all interrupts                                                                     
    DCH0INTbits.CHCCIE = 1;         // Interrupt on block transfer complete
    DCH0INTbits.CHERIE = 1;         // Interrupt on errors
// Interrupt setup
    IPC18bits.DMA0IP = 1;           // Priority
    IPC18bits.DMA0IS = 0;           // Sub priority
    IFS2bits.DMA0IF = 0;            // Interrupt flag CLR
    IEC2bits.DMA0IE = 1;            // Interrupt enable
}
/*************************************/
/*********************************/
void __ISR(_DMA0_VECTOR, IPL3AUTO) DmaHandler0(void) //_DMA0_VECTOR = 72
{     
    DCH0INTCLR=0x000000ff;              // clear DMA's channel interrupt flags 
    IFS2bits.DMA0IF = 0;                // clear interruption flag      
    putchar(rxBuf[rx_number]);          //echo  __XC_UART = 1 !!!!!!! 
    if (rxBuf[rx_number] == '\r' || rx_number == RX_BUF_SIZE-1 )       // can be done by PATERN but I need for call
    {
        puts("\r");                         // output formating
        puts (MessageBuf);
        puts (rxBuf);                       // print buffer
        DCH1ECONbits.CFORCE = 1;
        memset(rxBuf, 0, strlen(rxBuf));    // clean buffer
        rx_number = 0;                      // init buffer count
        DCH0DSA = VirtToPhys((void*)&rxBuf[rx_number]);     // refresh destination 
        puts (MessageUART);
       return;                              // ????
    }
    rx_number++; 
}
/*************************************/
/*********************************/
void __ISR(_UART1_RX_VECTOR, IPL1AUTO) UartRxHandler0(void) //_DMA0_VECTOR = 72
{    
   CLRBIT(IFS1,7);             //! Clear UART1 RX interruption flag  
}
/*********************************/
/*********************************/