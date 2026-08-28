#include "stdlib_native.h"

void azr_register_json(Environment *env);
void azr_register_csv(Environment *env);
void azr_register_crypto(Environment *env);
void azr_register_os_path(Environment *env);
void azr_register_net_process(Environment *env);
void azr_register_data_time(Environment *env);

void azr_stdlib_register(Environment *env) {
    azr_register_json(env);
    azr_register_csv(env);
    azr_register_crypto(env);
    azr_register_os_path(env);
    azr_register_net_process(env);
    azr_register_data_time(env);
}
