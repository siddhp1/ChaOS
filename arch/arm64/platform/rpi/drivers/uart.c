#include "drivers/uart.h"

#include <stdbool.h>
#include <stdint.h>

#include "kernel/irq.h"
#include "kernel/scheduler/scheduler.h"
#include "kernel/scheduler/wait.h"
#include "mm/kmap.h"

#define MMIO_BASE (KERNEL_VIRT_BASE + 0x3F000000UL)
#define GPFSEL1 (*(volatile unsigned int*)(MMIO_BASE + 0x200004))
#define GPPUD (*(volatile unsigned int*)(MMIO_BASE + 0x200094))
#define GPPUDCLK0 (*(volatile unsigned int*)(MMIO_BASE + 0x200098))

#define UART0_BASE (MMIO_BASE + 0x201000)
#define UART_DR (*(volatile unsigned int*)(UART0_BASE + 0x00))
#define UART_FR (*(volatile unsigned int*)(UART0_BASE + 0x18))
#define UART_IBRD (*(volatile unsigned int*)(UART0_BASE + 0x24))
#define UART_FBRD (*(volatile unsigned int*)(UART0_BASE + 0x28))
#define UART_LCRH (*(volatile unsigned int*)(UART0_BASE + 0x2C))
#define UART_CR (*(volatile unsigned int*)(UART0_BASE + 0x30))
#define UART_IFLS (*(volatile uint32_t*)(UART0_BASE + 0x34))
#define UART_IMSC (*(volatile uint32_t*)(UART0_BASE + 0x38))
#define UART_MIS (*(volatile uint32_t*)(UART0_BASE + 0x40))
#define UART_ICR (*(volatile uint32_t*)(UART0_BASE + 0x44))

#define RXFE (1 << 4)
#define TXFF (1 << 5)

#define UARTIMSC_RXIM (1u << 4)
#define UARTIMSC_TXIM (1u << 5)
#define UARTIMSC_RTIM (1u << 6)

#define UARTMIS_RXMIS (1u << 4)
#define UARTMIS_TXMIS (1u << 5)
#define UARTMIS_RTMIS (1u << 6)

#define UARTICR_RXIC (1u << 4)
#define UARTICR_TXIC (1u << 5)
#define UARTICR_RTIC (1u << 6)

#define UART_RX_BUF_SIZE 256
#define UART_TX_BUF_SIZE 256

static char rx_buf[UART_RX_BUF_SIZE];
static uint32_t rx_head;
static uint32_t rx_tail;
static uint32_t rx_count;

static char tx_buf[UART_TX_BUF_SIZE];
static uint32_t tx_head;
static uint32_t tx_tail;
static uint32_t tx_count;

static struct wait_queue rx_wait_queue;
static struct wait_queue tx_wait_queue;

static void delay(int count) {
  while (count--) {
    asm volatile("nop");
  }
}

static void rx_push(char c) {
  if (rx_count == UART_RX_BUF_SIZE) return;
  rx_buf[rx_head] = c;
  rx_head = (rx_head + 1) % UART_RX_BUF_SIZE;
  rx_count++;
}

static bool rx_pop(char* out) {
  if (!out || rx_count == 0) return false;
  *out = rx_buf[rx_tail];
  rx_tail = (rx_tail + 1) % UART_RX_BUF_SIZE;
  rx_count--;
  return true;
}

static void tx_push(char c) {
  if (tx_count == UART_TX_BUF_SIZE) return;
  tx_buf[tx_head] = c;
  tx_head = (tx_head + 1) % UART_TX_BUF_SIZE;
  tx_count++;
}

static bool tx_pop(char* out) {
  if (!out || tx_count == 0) return false;
  *out = tx_buf[tx_tail];
  tx_tail = (tx_tail + 1) % UART_TX_BUF_SIZE;
  tx_count--;
  return true;
}

static void load_tx_fifo(void) {
  while (tx_count > 0 && !(UART_FR & TXFF)) {
    char c;
    if (!tx_pop(&c)) break;
    UART_DR = (uint32_t)(uint8_t)c;
  }

  if (tx_count > 0) {
    UART_IMSC |= UARTIMSC_TXIM;
  } else {
    UART_IMSC &= ~UARTIMSC_TXIM;
  }
}

static void uart_irq_handler(void* unused) {
  (void)unused;

  bool rx_progress = false;
  bool tx_progress = false;

  if (UART_MIS & (UARTMIS_RXMIS | UARTMIS_RTMIS)) {
    while (!(UART_FR & RXFE)) {
      rx_push((char)(UART_DR & 0xFF));
      rx_progress = true;
    }
    UART_ICR = UARTICR_RXIC | UARTICR_RTIC;
  }

  if (UART_MIS & UARTMIS_TXMIS) {
    uint32_t before = tx_count;
    load_tx_fifo();
    if (tx_count < before) tx_progress = true;
    UART_ICR = UARTICR_TXIC;
  }

  if (rx_progress) unwait_all(&rx_wait_queue);
  if (tx_progress) unwait_all(&tx_wait_queue);
}

void uart_init(void) {
  // Disable UART
  UART_CR = 0;
  UART_IMSC = 0;
  UART_ICR = 0x7FF;

  // Set GPIO14/15 to ALT0 (TXD0/RXD0)
  unsigned int r = GPFSEL1;
  r &= ~((7u << 12) | (7u << 15));
  r |= ((4u << 12) | (4u << 15));
  GPFSEL1 = r;

  // Disable pulls for GPIO14/15
  GPPUD = 0;
  delay(150);
  GPPUDCLK0 = (1u << 14) | (1u << 15);
  delay(150);
  GPPUDCLK0 = 0;

  // Set baud rate (115200)
  UART_IBRD = 26;
  UART_FBRD = 3;

  // 8N1 + FIFO enabled
  UART_LCRH = (1u << 4) | (1u << 5) | (1u << 6);
  UART_IFLS = 0;  // 1/8 RX and TX to reduce latency for console use.

  rx_head = rx_tail = rx_count = 0;
  tx_head = tx_tail = tx_count = 0;

  wait_queue_init(&rx_wait_queue);
  wait_queue_init(&tx_wait_queue);

  // Enable UART, TX, RX
  UART_CR = (1u << 0) | (1u << 8) | (1u << 9);
}

void uart_irq_init(void) {
  register_irq(IRQ_UART, uart_irq_handler);

  while (!(UART_FR & RXFE)) {
    (void)(UART_DR & 0xFF);
  }
  UART_ICR = 0x7FF;
  UART_IMSC = UARTIMSC_RXIM | UARTIMSC_RTIM;
}

char uart_getc(void) {
  while (1) {
    char c = 0;

    irq_disable();

    if (rx_pop(&c)) {
      irq_enable();
      return c;
    }

    if (!(UART_FR & RXFE)) {
      c = (char)(UART_DR & 0xFF);
      irq_enable();
      return c;
    }

    if (!current_task) {
      irq_enable();
      while (UART_FR & RXFE);
      return (char)(UART_DR & 0xFF);
    }
    task_wait(&rx_wait_queue, current_task);
  }
}

void uart_putc(char c) {
  if (c == '\n') {
    uart_putc('\r');
  }
  while (UART_FR & TXFF);
  UART_DR = (uint32_t)(uint8_t)c;
}

long uart_read(char* dst, long len) {
  if (!dst || len < 0) return -1;
  if (len == 0) return 0;

  long i = 0;
  while (i < len) dst[i++] = uart_getc();
  return i;
}

long uart_write(const char* src, long len) {
  if (!src || len < 0) return -1;
  if (len == 0) return 0;

  long written = 0;
  bool emit_lf_after_cr = false;
  while (written < len) {
    char outc;
    if (emit_lf_after_cr) {
      outc = '\n';
    } else if (src[written] == '\n') {
      outc = '\r';
    } else {
      outc = src[written];
    }

    irq_disable();

    if (!(UART_FR & TXFF) && tx_count == 0) {
      UART_DR = (uint32_t)(uint8_t)outc;
      irq_enable();

      if (emit_lf_after_cr) {
        emit_lf_after_cr = false;
        written++;
      } else if (src[written] == '\n') {
        emit_lf_after_cr = true;
      } else {
        written++;
      }
      continue;
    }

    if (tx_count < UART_TX_BUF_SIZE) {
      tx_push(outc);
      load_tx_fifo();
      irq_enable();

      if (emit_lf_after_cr) {
        emit_lf_after_cr = false;
        written++;
      } else if (src[written] == '\n') {
        emit_lf_after_cr = true;
      } else {
        written++;
      }
      continue;
    }

    if (!current_task) {
      irq_enable();
      while (UART_FR & TXFF);
      UART_DR = (uint32_t)(uint8_t)outc;

      if (emit_lf_after_cr) {
        emit_lf_after_cr = false;
        written++;
      } else if (src[written] == '\n') {
        emit_lf_after_cr = true;
      } else {
        written++;
      }
      continue;
    }
    task_wait(&tx_wait_queue, current_task);
  }

  return written;
}
