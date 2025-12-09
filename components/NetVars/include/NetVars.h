#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <cJSON.h>
#include <nvs.h>

// ------------------ BEGIN Return code ------------------
typedef enum {
    NetVars_ret_error = -1,
    NetVars_ret_ok    = 0
} NetVars_return_code_t;
// ------------------ END   Return code ------------------

// ------------------ BEGIN Datatypes ------------------
typedef enum {
    NETVARS_TYPE_BOOL,
    NETVARS_TYPE_U8,
    NETVARS_TYPE_I8,
    NETVARS_TYPE_I32,
    NETVARS_TYPE_U32,
    NETVARS_TYPE_FLOAT,
    NETVARS_TYPE_STRING,
} NetVars_type_t;

typedef enum {
    PRJCFG_NVS_NONE,
    PRJCFG_NVS_LOAD,
    PRJCFG_NVS_SAVE,
    PRJCFG_NVS_LOADSAVE,
} NetVars_nvs_mode_t;

typedef struct {
    const char           *name;
    const char           *nvs_key;
    const char           *json_key;
    const char           *group;
    const char           *module;
    NetVars_type_t           type;
    NetVars_nvs_mode_t            nvs_mode;
    bool                  json;     // se expone en JSON
    void                 *ptr;      // &prjcfg.campo o prjcfg.array
} NetVars_desc_t;

// ------------------ END   Datatypes ------------------

void NetVars_nvs_load(const NetVars_desc_t netvars_desc[], const size_t netvars_count, nvs_handle_t h);
void NetVars_nvs_save(const NetVars_desc_t netvars_desc[], const size_t netvars_count, nvs_handle_t h);
void NetVars_append_json(const NetVars_desc_t netvars_desc[], const size_t netvars_count, cJSON *root);
bool NetVars_parse_json_dict(const NetVars_desc_t netvars_desc[], const size_t netvars_count, cJSON *root);

void NetVars_nvs_load_component(const char *ident, const NetVars_desc_t netvars_desc[], const size_t netvars_count);
void NetVars_nvs_save_component(const char *ident, const NetVars_desc_t netvars_desc[], const size_t netvars_count);
void NetVars_append_json_component(const char *ident, const NetVars_desc_t netvars_desc[], const size_t netvars_count, cJSON *root);
bool NetVars_parse_json_component(const char *ident, const NetVars_desc_t netvars_desc[], const size_t netvars_count, cJSON *root);
bool NetVars_parse_json_component_data(const char *ident, const NetVars_desc_t netvars_desc[], const size_t netvars_count, const char *data);


#ifdef __cplusplus
}
#endif
