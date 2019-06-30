/* This file is auto-generated by mos build, do not edit! */

#include <stdbool.h>
#include <stdio.h>

#include "common/cs_dbg.h"

#include "mgos_app.h"


extern bool mgos_mongoose_init(void);
extern bool mgos_vfs_common_init(void);
extern bool mgos_vfs_dev_part_init(void);
extern bool mgos_vfs_fs_spiffs_init(void);
extern bool mgos_core_init(void);
extern bool mgos_i2c_init(void);
extern bool mgos_atca_init(void);
extern bool mgos_blynk_init(void);
extern bool mgos_wifi_init(void);
extern bool mgos_http_server_init(void);
extern bool mgos_mbedtls_init(void);
extern bool mgos_mjs_init(void);
extern bool mgos_provision_init(void);
extern bool mgos_rpc_common_init(void);
extern bool mgos_rpc_service_config_init(void);
extern bool mgos_rpc_service_fs_init(void);
extern bool mgos_rpc_service_wifi_init(void);
extern bool mgos_rpc_uart_init(void);

static const struct lib_descr {
  const char *title;
  bool (*init)(void);
} descrs[] = {

    // "mongoose". deps: [ ]
    {.title = "mongoose", .init = mgos_mongoose_init},

    // "vfs-common". deps: [ ]
    {.title = "vfs-common", .init = mgos_vfs_common_init},

    // "vfs-dev-part". deps: [ "vfs-common" ]
    {.title = "vfs-dev-part", .init = mgos_vfs_dev_part_init},

    // "vfs-fs-spiffs". deps: [ "vfs-common" ]
    {.title = "vfs-fs-spiffs", .init = mgos_vfs_fs_spiffs_init},

    // "core". deps: [ "mongoose" "vfs-common" "vfs-dev-part" "vfs-fs-spiffs" ]
    {.title = "core", .init = mgos_core_init},

    // "i2c". deps: [ "core" ]
    {.title = "i2c", .init = mgos_i2c_init},

    // "atca". deps: [ "i2c" ]
    {.title = "atca", .init = mgos_atca_init},

    // "blynk". deps: [ "core" ]
    {.title = "blynk", .init = mgos_blynk_init},

    // "wifi". deps: [ "core" ]
    {.title = "wifi", .init = mgos_wifi_init},

    // "http-server". deps: [ "atca" "core" "wifi" ]
    {.title = "http-server", .init = mgos_http_server_init},

    // "mbedtls". deps: [ ]
    {.title = "mbedtls", .init = mgos_mbedtls_init},

    // "mjs". deps: [ "core" ]
    {.title = "mjs", .init = mgos_mjs_init},

    // "provision". deps: [ "core" ]
    {.title = "provision", .init = mgos_provision_init},

    // "rpc-common". deps: [ "core" "http-server" "mongoose" ]
    {.title = "rpc-common", .init = mgos_rpc_common_init},

    // "rpc-service-config". deps: [ "core" "rpc-common" ]
    {.title = "rpc-service-config", .init = mgos_rpc_service_config_init},

    // "rpc-service-fs". deps: [ "core" "rpc-common" ]
    {.title = "rpc-service-fs", .init = mgos_rpc_service_fs_init},

    // "rpc-service-wifi". deps: [ "core" "rpc-common" "wifi" ]
    {.title = "rpc-service-wifi", .init = mgos_rpc_service_wifi_init},

    // "rpc-uart". deps: [ "core" "rpc-common" ]
    {.title = "rpc-uart", .init = mgos_rpc_uart_init},

};

bool mgos_deps_init(void) {
  size_t i;
  for (i = 0; i < sizeof(descrs) / sizeof(struct lib_descr); i++) {
    LOG(LL_DEBUG, ("init %s...", descrs[i].title));
    if (!descrs[i].init()) {
      LOG(LL_ERROR, ("%s init failed", descrs[i].title));
      return false;
    }
  }

  return true;
}
