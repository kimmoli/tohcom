#ifndef SC16IS850L_REGISTERS_H
#define SC16IS850L_REGISTERS_H

/* General register set */
#define GR_RHR          (0x00<<3)
#define GR_THR          (0x00<<3)
#define GR_IER          (0x01<<3)
#define GR_FCR          (0x02<<3)
#define GR_ISR          (0x02<<3)
#define GR_LCR          (0x03<<3)
#define GR_MCR          (0x04<<3)
#define GR_LSR          (0x05<<3)
#define GR_EFCR         (0x05<<3)
#define GR_MSR          (0x06<<3)
#define GR_SPR          (0x07<<3)

/* Special register set */
#define SR_DLL          (0x00<<3)
#define SR_DLM          (0x01<<3)

/* Second special register set */
#define SSR_TXLVLCNT    (0x03<<3)
#define SSR_RXLVLCNT    (0x04<<3)

/* Enhanced feature register set */
#define EFR_EFR         (0x02<<3)
#define EFR_XON1        (0x04<<3)
#define EFR_XON2        (0x05<<3)
#define EFR_XOFF1       (0x06<<3)
#define EFR_XOFF2       (0x07<<3)

/* First extra feature register set */ 
#define FEFR_TXINTLVL   (0x02<<3)
#define FEFR_RXINTLVL   (0x04<<3)
#define FEFR_FLWCNTH    (0x06<<3)
#define FEFR_FLWCNTL    (0x07<<3)

/* Second extra feature register set */
#define SEFR_CLKPRES    (0x02<<3)
#define SEFR_RS485TIME  (0x04<<3)
#define SEFR_AFCR2      (0x06<<3)
#define SEFR_AFCR1      (0x07<<3)

#endif // SC16IS850L_REGISTERS_H
