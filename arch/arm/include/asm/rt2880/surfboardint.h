#include <mach/mt_irq.h>

//#define SURFBOARDINT_SYSCTL      0      /* SYSCTL */
#define SURFBOARDINT_FE          MT_FE_ORIG_IRQ_ID	/* FE */
#define SURFBOARDINT_PCM         MT_PCM_IRQ_ID      /* PCM */
//#define SURFBOARDINT_GPIO        6      /* GPIO */
#define SURFBOARDINT_HSGDMA      MT_HSDMA_IRQ_ID      /* HSGDMA */
#define SURFBOARDINT_DMA         MT_GDMA_IRQ_ID      /* DMA */
//#define SURFBOARDINT_PC          9      /* Performance counter */
#define SURFBOARDINT_I2S         MT_I2S_IRQ_ID     /* I2S */
//#define SURFBOARDINT_SPI         11     /* SPI */
//#define SURFBOARDINT_AES         13     /* AES */
//#define SURFBOARDINT_AESENGINE      13     /* AES Engine */
#define SURFBOARDINT_CRYPTO      MT_CRYPTO_IRQ_ID     /* CryptoEngine */
//#define SURFBOARDINT_SDXC        14     /* SDXC */
//#define SURFBOARDINT_ESW         17     /* ESW */
#define SURFBOARDINT_USB0         MT_SSUSB_XHCI0_IRQ_ID     /* USB0 */
#define SURFBOARDINT_USB1         MT_SSUSB_XHCI1_IRQ_ID     /* USB1 */
//#define SURFBOARDINT_UART_LITE1  20     /* UART Lite */
//#define SURFBOARDINT_UART_LITE2  21     /* UART Lite */
//#define SURFBOARDINT_UART_LITE3  22     /* UART Lite */
//#define SURFBOARDINT_UART1       SURFBOARDINT_UART_LITE1
//#define SURFBOARDINT_UART        SURFBOARDINT_UART_LITE2
//#define SURFBOARDINT_WDG         23     /* WDG timer */
//#define SURFBOARDINT_TIMER0      24     /* Timer0 */
//#define SURFBOARDINT_TIMER1      25     /* Timer1 */
//#define SURFBOARDINT_ILL_ACC     35     /* illegal access */
#define RALINK_INT_PCIE0         MT_PCIE0_IRQ_ID     /* PCIE0 */
#define RALINK_INT_PCIE1         MT_PCIE1_IRQ_ID     /* PCIE1 */
#define RALINK_INT_PCIE2         MT_PCIE2_IRQ_ID     /* PCIE2 */

// Wait for RD to define IRQ source

//#define RALINK_INT_xxx         MT_CRYPTO_RING0_IRQ_ID     /*  */
//#define RALINK_INT_xxx         MT_CRYPTO_RING1_IRQ_ID     /*  */
//#define RALINK_INT_xxx         MT_CRYPTO_RING2_IRQ_ID     /*  */
//#define RALINK_INT_xxx         MT_FE_PDMA_IRQ_ID     /*  */
//#define RALINK_INT_xxx         MT_FE_QDMA_IRQ_ID     /*  */
//#define RALINK_INT_xxx         MT_PCIE_LINK_DOWN_RST_IRQ_ID     /*  */



