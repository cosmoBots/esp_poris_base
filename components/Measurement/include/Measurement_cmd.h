#pragma once

#include <cJSON.h>

void Measurement_parse_callback(const char *data, int len);
void Measurement_req_parse_callback(const char *data, int len);
void Measurement_compose_json_payload(cJSON *root);
