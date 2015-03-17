#ifndef SC16IS850L_REGISTERS_H
#define SC16IS850L_REGISTERS_H

/* General register set */
#define GR_RHR          (char)(0x00<<3)
#define GR_THR          (char)(0x00<<3)
#define GR_IER          (char)(0x01<<3)
#define GR_FCR          (char)(0x02<<3)
#define GR_ISR          (char)(0x02<<3)
#define GR_LCR          (char)(0x03<<3)
#define GR_MCR          (char)(0x04<<3)
#define GR_LSR          (char)(0x05<<3)
#define GR_EFCR         (char)(0x05<<3)
#define GR_MSR          (char)(0x06<<3)
#define GR_SPR          (char)(0x07<<3)

/* Special register set */
#define SR_DLL          (char)(0x00<<3)
#define SR_DLM          (char)(0x01<<3)

/* Second special register set */
#define SSR_TXLVLCNT    (char)(0x03<<3)
#define SSR_RXLVLCNT    (char)(0x04<<3)

/* Enhanced feature register set */
#define EFR_EFR         (char)(0x02<<3)
#define EFR_XON1        (char)(0x04<<3)
#define EFR_XON2        (char)(0x05<<3)
#define EFR_XOFF1       (char)(0x06<<3)
#define EFR_XOFF2       (char)(0x07<<3)

/* First extra feature register set */ 
#define FEFR_TXINTLVL   (char)(0x02<<3)
#define FEFR_RXINTLVL   (char)(0x04<<3)
#define FEFR_FLWCNTH    (char)(0x06<<3)
#define FEFR_FLWCNTL    (char)(0x07<<3)

/* Second extra feature register set */
#define SEFR_CLKPRES    (char)(0x02<<3)
#define SEFR_RS485TIME  (char)(0x04<<3)
#define SEFR_AFCR2      (char)(0x06<<3)
#define SEFR_AFCR1      (char)(0x07<<3)

/* IER - interrupt enable bits */
#define IER_RHRI        (char)(0x01)

/* LCR - line parameters */
#define PARITY_NONE     (char)(0x00<<3)
#define PARITY_ODD      (char)(0x01<<3)
#define PARITY_EVEN     (char)(0x03<<3)
#define PARITY_FORCE1   (char)(0x05<<3)
#define PARITY_FORCE0   (char)(0x07<<3)
#define STOP_1          (char)(0x00<<2)
#define STOP_2          (char)(0x01<<2)
#define WORDLEN_5       (char)(0x00)
#define WORDLEN_6       (char)(0x01)
#define WORDLEN_7       (char)(0x02)
#define WORDLEN_8       (char)(0x03)

/* EFCR Register bits */
#define EFCR_GR         (char)(0x00<<1)
#define EFCR_FEFR       (char)(0x01<<1)
#define EFCR_SEFR       (char)(0x02<<1)

/* LSR Register bits */
#define LSR_FIFOERR     (char)(0x80)
#define LSR_THRTSREMPTY (char)(0x40)
#define LSR_THREMPTY    (char)(0x20)
#define LSR_BREAK       (char)(0x10)
#define LSR_FRAMINGERR  (char)(0x08)
#define LSR_PARITYERR   (char)(0x04)
#define LSR_OVERRUN     (char)(0x02)
#define LSR_RXREADY     (char)(0x01)

/* MCR Register bits */
#define MCR_CLKSEL      (char)(0x80)
#define MCR_IREN        (char)(0x40)
#define MCR_LOOPBACK    (char)(0x10)
#define MCR_OP2         (char)(0x08)
#define MCR_OP1         (char)(0x04)
#define MCR_RTSL        (char)(0x02)
#define MCR_DTRL        (char)(0x01)


#endif // SC16IS850L_REGISTERS_H
