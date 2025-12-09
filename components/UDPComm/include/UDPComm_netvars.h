#pragma once
#ifndef UDPCOMM_NETVARS_H
#define UDPCOMM_NETVARS_H

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <NetVars.h>

void UDPComm_netvars_nvs_load(void);
void UDPComm_netvars_nvs_save(void);

void UDPComm_netvars_append_json(cJSON *root);
bool UDPComm_netvars_parse_json_dict(cJSON *root);

void UDPComm_config_parse_json(const char *data);

void UDPComm_nvs_set_dirty(void);
void UDPComm_nvs_spin(void);

#ifdef __cplusplus
}
#endif

#endif // UDPCOMM_NETVARS_H
