#ifndef AZR_PROJECT_H
#define AZR_PROJECT_H

typedef struct {
    char *name;
    char *version;
    char *source_path;
} AzrProjectDependency;

typedef struct {
    char *root_dir;
    char *name;
    char *version;
    char *package_dir;
    char **extra_paths;
    int extra_path_count;
    AzrProjectDependency *dependencies;
    int dependency_count;
} AzrProjectConfig;

int azr_project_load(const char *start_dir, AzrProjectConfig *out);
int azr_project_collect_import_roots(const char *start_dir, char ***roots_out, int *count_out);
int azr_project_init(const char *project_dir, const char *name, const char *version, char **error_out);
int azr_project_install_local(const char *project_dir, const char *source_path, char **error_out);
void azr_project_free(AzrProjectConfig *config);

#endif
