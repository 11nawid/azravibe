#ifndef AZR_VERSION_H
#define AZR_VERSION_H

#define AZR_DEFAULT_VERSION "v.0.1"

void azr_version_init(const char *argv0);
const char *azr_version_get(void);
void azr_version_shutdown(void);

#endif
