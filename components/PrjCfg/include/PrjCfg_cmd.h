#pragma once

#include "PrjCfg.h"

void prjcfg_parse_callback(const char *data, int len);
void prjcfg_req_parse_callback(const char *data, int len);
void prjcfg_compose_callback(char *data, int *len);
