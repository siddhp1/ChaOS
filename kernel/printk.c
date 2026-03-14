#include "kernel/printk.h"

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#include "kernel/uart.h"

void printk(const char* fmt, ...) {
  va_list args;
  va_start(args, fmt);

  while (*fmt) {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt) {
        case '%':
          uart_putc('%');
          break;
        case 'c': {
          int32_t val = va_arg(args, int32_t);
          uart_putc((char)val);
          break;
        }
        case 'd': {
          int32_t val = va_arg(args, int32_t);
          if (val < 0) {
            uart_putc('-');
            val = -val;
          }
          char buf[12];
          int i = 0;
          if (val == 0) {
            uart_putc('0');
            break;
          }
          while (val > 0) {
            buf[i++] = '0' + (val % 10);
            val /= 10;
          }
          while (i--) {
            uart_putc(buf[i]);
          }
          break;
        }
        case 's':
          const char* s = va_arg(args, const char*);
          while (*s) {
            uart_putc(*s++);
          }
          break;
        case 'u': {
          uint32_t val = va_arg(args, uint32_t);
          char buf[11];
          int32_t i = 0;
          if (val == 0) {
            uart_putc('0');
            break;
          }
          while (val > 0) {
            buf[i++] = '0' + (val % 10);
            val /= 10;
          }
          while (i--) {
            uart_putc(buf[i]);
          }
          break;
        }
        case 'x':
          uint32_t val = va_arg(args, uint32_t);
          char buf[11];
          buf[0] = '0';
          buf[1] = 'x';
          for (int i = 0; i < 8; i++) {
            uint32_t nibble = (val >> (28 - 4 * i)) & 0xF;
            buf[2 + i] = (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
          }
          buf[10] = '\0';
          for (size_t i = 0; i < 11; i++) {
            uart_putc(buf[i]);
          }
          break;
        case 'l':
          fmt++;
          switch (*fmt) {
            case 'd': {
              int64_t val = va_arg(args, int64_t);
              if (val < 0) {
                uart_putc('-');
                val = -val;
              }
              char buf[20];
              int i = 0;
              if (val == 0) {
                uart_putc('0');
                break;
              }
              while (val > 0) {
                buf[i++] = '0' + (val % 10);
                val /= 10;
              }
              while (i--) {
                uart_putc(buf[i]);
              }
              break;
            }
            case 'u': {
              uint64_t val = va_arg(args, uint64_t);
              char buf[19];
              int32_t i = 0;
              if (val == 0) {
                uart_putc('0');
                break;
              }
              while (val > 0) {
                buf[i++] = '0' + (val % 10);
                val /= 10;
              }
              while (i--) {
                uart_putc(buf[i]);
              }
              break;
            }
            case 'x': {
              uint64_t val = va_arg(args, uint64_t);
              char buf[19];
              buf[0] = '0';
              buf[1] = 'x';
              for (int i = 0; i < 16; i++) {
                uint64_t nibble = (val >> (60 - 4 * i)) & 0xF;
                buf[2 + i] =
                    (nibble < 10) ? ('0' + nibble) : ('A' + (nibble - 10));
              }
              buf[18] = '\0';
              for (size_t i = 0; i < 19; i++) {
                uart_putc(buf[i]);
              }
              break;
            }
            default:
              break;
          }
        default:
          // Ignore
          break;
      }
    } else {
      uart_putc(*fmt);
    }
    fmt++;
  }

  va_end(args);
}
