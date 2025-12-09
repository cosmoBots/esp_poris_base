#pragma once
#ifndef MQTTCOMM_NETVARS_H
#define MQTTCOMM_NETVARS_H

#include <stdbool.h>
#include <stddef.h>
#include "cJSON.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <NetVars.h>

void MQTTComm_netvars_nvs_load(void);
void MQTTComm_netvars_nvs_save(void);

void MQTTComm_netvars_append_json(cJSON *root);
bool MQTTComm_netvars_parse_json_dict(cJSON *root);

void MQTTComm_config_parse_json(const char *data);

#ifdef __cplusplus
}
#endif

#endif // MQTTCOMM_NETVARS_H
