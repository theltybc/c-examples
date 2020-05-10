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

#define SERVER_ID 1
#define ADDRESS_START 0
#define ADDRESS_END 9

/* At each loop, the program works in the range ADDRESS_START to
 * ADDRESS_END then ADDRESS_START + 1 to ADDRESS_END and so on.
 */
int main(void) {
  modbus_t *ctx;
  int rc;
  int addr;
  int nb;
  uint16_t *tab_rq_registers;
  uint16_t *tab_rp_registers;

  /* RTU */
  // ctx = modbus_new_rtu("/dev/ttyUSB0", 115200, 'N', 8, 1);
  ctx = modbus_new_rtu("/dev/ttyUSB0", 115200, 'N', 8, 1);
  modbus_set_slave(ctx, SERVER_ID);

  /* TCP */
  // ctx = modbus_new_tcp("127.0.0.1", 1502);
  // modbus_set_debug(ctx, TRUE);

  if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
  }

  /* Allocate and initialize the different memory spaces */
  nb = ADDRESS_END - ADDRESS_START;


  tab_rq_registers = (uint16_t *)malloc(nb * sizeof(uint16_t));
  memset(tab_rq_registers, 0, nb * sizeof(uint16_t));

  tab_rp_registers = (uint16_t *)malloc(nb * sizeof(uint16_t));
  memset(tab_rp_registers, 0, nb * sizeof(uint16_t));

  while (1) {
    for (addr = ADDRESS_START; addr < ADDRESS_END; addr++) {
      int i;

      /* Random numbers (short) */
      for (i = 0; i < nb; i++) {
        tab_rq_registers[i] = (uint16_t)(65535.0 * rand() / (RAND_MAX + 1.0));
      }
      nb = ADDRESS_END - addr;

      /* MULTIPLE REGISTERS */
      rc = modbus_write_registers(ctx, addr, nb, tab_rq_registers);
      if (rc != nb) {
        printf("ERROR modbus_write_registers (%d)\n", rc);
        printf("Address = %d, nb = %d\n", addr, nb);
      } else {
        rc = modbus_read_registers(ctx, addr, nb, tab_rp_registers);
        if (rc != nb) {
          printf("ERROR modbus_read_registers (%d)\n", rc);
          printf("Address = %d, nb = %d\n", addr, nb);
        } else {
          for (i = 0; i < nb; i++) {
            if (tab_rq_registers[i] != tab_rp_registers[i]) {
              printf("ERROR modbus_read_registers\n");
              printf("Address = %d, value %d (0x%X) != %d (0x%X)\n",
                     addr, tab_rq_registers[i], tab_rq_registers[i],
                     tab_rp_registers[i], tab_rp_registers[i]);
            }
          }
        }
      }
      usleep(100000);
    }
  }

  /* Free the memory */
  free(tab_rq_registers);
  free(tab_rp_registers);

  /* Close the connection */
  modbus_close(ctx);
  modbus_free(ctx);

  return 0;
}
