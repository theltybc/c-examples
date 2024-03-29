/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <modbus.h>

#define BAUD (115200)
#define DEV "/dev/ttyUSB0"

#define SERVER_ID 1
#define ADDRESS_START 0
#define ADDRESS_END 40

// #define WRITE_BIT
// #define MULTIPLE_BITS
// #define SINGLE_REGISTER
#define MULTIPLE_REGISTERS

/* At each loop, the program works in the range ADDRESS_START to
 * ADDRESS_END then ADDRESS_START + 1 to ADDRESS_END and so on.
 */
int main(void) {
  modbus_t *ctx;
  int rc;
  int addr;
  int nb;
  unsigned error_count = 0;
  unsigned reqest_count = 0;

  /* RTU */
  ctx = modbus_new_rtu(DEV, BAUD, 'N', 8, 1);
  modbus_set_slave(ctx, SERVER_ID);

#if LIBMODBUS_VERSION_MAJOR < 3
  struct timeval tv = {0, 5000 * 1000};
  modbus_set_response_timeout(ctx, &tv);
#else
  modbus_set_response_timeout(ctx, 0, 5000 * 1000);
#endif

  if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
  }

  /* Allocate and initialize the different memory spaces */
  nb = ADDRESS_END - ADDRESS_START;

  uint8_t *tab_rq_bits = (uint8_t *)malloc(nb * sizeof(uint8_t));
  memset(tab_rq_bits, 0, nb * sizeof(uint8_t));

  uint8_t *tab_rp_bits = (uint8_t *)malloc(nb * sizeof(uint8_t));
  memset(tab_rp_bits, 0, nb * sizeof(uint8_t));

  uint16_t *tab_rq_registers = (uint16_t *)malloc(nb * sizeof(uint16_t));
  memset(tab_rq_registers, 0, nb * sizeof(uint16_t));

  uint16_t *tab_rp_registers = (uint16_t *)malloc(nb * sizeof(uint16_t));
  memset(tab_rp_registers, 0, nb * sizeof(uint16_t));

  while (errno != EIO) {
    for (addr = ADDRESS_START; addr < ADDRESS_END; addr++) {
      modbus_flush(ctx);
      int i;

      /* Random numbers (short) */
      for (i = 0; i < nb; i++) {
        tab_rq_registers[i] = (uint16_t)(65535.0 * rand() / (RAND_MAX + 1.0));
        tab_rq_bits[i] = tab_rq_registers[i] % 2;
      }
      nb = ADDRESS_END - addr;

#ifdef WRITE_BIT
      rc = modbus_write_bit(ctx, addr, tab_rq_bits[0]);
      reqest_count++;
      if (rc != 1) {
        printf("ERROR modbus_write_bit (%d) %s\n", rc, modbus_strerror(errno));
        printf("Address = %d, value = %d\n", addr, tab_rq_bits[0]);
        error_count++;
      } else {
        rc = modbus_read_bits(ctx, addr, 1, tab_rp_bits);
        reqest_count++;
        if (rc != 1 || tab_rq_bits[0] != tab_rp_bits[0]) {
          printf("ERROR modbus_read_bits single (%d) %s\n", rc, modbus_strerror(errno));
          printf("address = %d\n", addr);
          error_count++;
        }
      }
#endif // WRITE_BIT

#ifdef MULTIPLE_BITS
      rc = modbus_write_bits(ctx, addr, nb, tab_rq_bits);
      reqest_count++;
      if (rc != nb) {
        printf("ERROR modbus_write_bits (%d) %s\n", rc, modbus_strerror(errno));
        printf("Address = %d, nb = %d\n", addr, nb);
        error_count++;
      } else {
        // rc = modbus_read_bits(ctx, addr, nb, tab_rp_bits);
        // reqest_count++;
        // if (rc != nb) {
        //   printf("ERROR modbus_read_bits (%d) %s\n", rc, modbus_strerror(errno));
        //   printf("Address = %d, nb = %d\n", addr, nb);
        //   error_count++;
        // } else {
        //   for (i = 0; i < nb; i++) {
        //     if (tab_rp_bits[i] != tab_rq_bits[i]) {
        //       printf("ERROR modbus_read_bits\n");
        //       printf("Address = %d, value %d (0x%X) != %d (0x%X)\n",
        //              addr, tab_rq_bits[i], tab_rq_bits[i],
        //              tab_rp_bits[i], tab_rp_bits[i]);
        //       error_count++;
        //     }
        //   }
        // }
      }
#endif // MULTIPLE_BITS

#ifdef SINGLE_REGISTER
      rc = modbus_write_register(ctx, addr, tab_rq_registers[0]);
      reqest_count++;
      if (rc != 1) {
        printf("ERROR modbus_write_register (%d) %s\n", rc, modbus_strerror(errno));
        printf("Address = %d, value = %d (0x%X)\n", addr, tab_rq_registers[0], tab_rq_registers[0]);
        error_count++;
      } else {
        rc = modbus_read_registers(ctx, addr, 1, tab_rp_registers);
        reqest_count++;
        if (rc != 1) {
          printf("ERROR modbus_read_registers single (%d) %s\n", rc, modbus_strerror(errno));
          printf("Address = %d\n", addr);
          error_count++;
        } else {
          if (tab_rq_registers[0] != tab_rp_registers[0]) {
            printf("ERROR modbus_read_registers single\n");
            printf("Address = %d, value = %d (0x%X) != %d (0x%X)\n", addr, tab_rq_registers[0], tab_rq_registers[0],
                   tab_rp_registers[0], tab_rp_registers[0]);
            error_count++;
          }
        }
      }
#endif // SINGLE_REGISTER

#ifdef MULTIPLE_REGISTERS
      rc = modbus_write_registers(ctx, addr, nb, tab_rq_registers);
      reqest_count++;
      if (rc != nb) {
        error_count++;
        printf("ERROR modbus_write_registers (%d) %s\n", rc, modbus_strerror(errno));
        printf("Address = %d, nb = %d\n", addr, nb);
      } else {
        rc = modbus_read_registers(ctx, addr, nb, tab_rp_registers);
        reqest_count++;
        if (rc != nb) {
          error_count++;
          printf("ERROR modbus_read_registers (%d) %s\n", rc, modbus_strerror(errno));
          printf("Address = %d, nb = %d\n", addr, nb);
        } else {
          for (i = 0; i < nb; i++) {
            if (tab_rq_registers[i] != tab_rp_registers[i]) {
              printf("ERROR modbus_read_registers\n");
              printf("Address = %d, value %d (0x%X) != %d (0x%X)\n", addr, tab_rq_registers[i], tab_rq_registers[i],
                     tab_rp_registers[i], tab_rp_registers[i]);
            }
          }
        }
      }
#endif // MULTIPLE_REGISTERS

      usleep(10000);
    }
    printf("Request: %u; error: %u; %%: %f;\n", reqest_count, error_count, ((float)error_count / (float)reqest_count));
  }
  perror("Exit loop");

  /* Free the memory */
  free(tab_rq_bits);
  free(tab_rp_bits);
  free(tab_rq_registers);
  free(tab_rp_registers);

  /* Close the connection */
  modbus_close(ctx);
  modbus_free(ctx);

  return 0;
}
