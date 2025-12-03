#pragma once

#include <cJSON.h>

void PrjCfg_parse_callback(const char *data, int len);
void PrjCfg_req_parse_callback(const char *data, int len);
void PrjCfg_compose_json_payload(cJSON *root);
