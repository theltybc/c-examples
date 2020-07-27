/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// compile with `pkg-config --cflags --libs libmodbus`

#include <stdio.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <stdlib.h>
#include <errno.h>

#include <modbus.h>

#define SLAVE_ID 1

int main(void) {
  modbus_t *ctx;
  modbus_mapping_t *mb_mapping;
  unsigned reqest_count = 0;
  unsigned error_count = 0;

  ctx = modbus_new_rtu("/dev/ttyUSB0", 115200, 'N', 8, 1);
  // modbus_set_debug(ctx, TRUE);

  if (modbus_set_slave(ctx, SLAVE_ID)) {
    fprintf(stderr, "Invalid slave ID\n");
    modbus_free(ctx);
    return -1;
  }

  if (modbus_connect(ctx) == -1) {
    fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
  }
  // modbus_mapping_t* modbus_mapping_new(int nb_coil_status, int nb_input_status,
  //                                      int nb_holding_registers, int nb_input_registers)
  mb_mapping = modbus_mapping_new(MODBUS_MAX_RW_WRITE_REGISTERS, MODBUS_MAX_RW_WRITE_REGISTERS,
                                  MODBUS_MAX_RW_WRITE_REGISTERS, MODBUS_MAX_RW_WRITE_REGISTERS);
  if (mb_mapping == NULL) {
    fprintf(stderr, "Failed to allocate the mapping: %s\n", modbus_strerror(errno));
    modbus_free(ctx);
    return -1;
  }

  while (errno != ECONNRESET) {
    modbus_flush(ctx);
    uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int rc;

    rc = modbus_receive(ctx, query);
    reqest_count++;
    if (rc > 0) {
      // printf("reply\n");
      // printf("speed: %u\n", mb_mapping->tab_registers[0]);
      // printf("coil: 0b");
      // for (int i = 0; i < 10; i++) {
      //   printf("%u", mb_mapping->tab_bits[i]);
      // }
      // puts("");
      /* rc is the query size */
      modbus_reply(ctx, query, rc, mb_mapping);
      // for (int i = 0; i < mb_mapping->nb_registers; i++) {
      //   mb_mapping->tab_registers[i]++;
      // }
    } else if (rc == -1) {
      error_count++;
      printf("Fail: %s\n", modbus_strerror(errno));
    }
    if (reqest_count % 100 == 0) {
      printf("Request: %u; error: %u; %%: %f;\n", reqest_count, error_count,
             ((float)error_count / (float)reqest_count));
    }
  }

  printf("Quit the loop: %s\n", modbus_strerror(errno));

  modbus_mapping_free(mb_mapping);
  modbus_close(ctx);
  modbus_free(ctx);

  return 0;
}
