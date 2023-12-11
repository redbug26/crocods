#include "gen_dirent.h"

#if defined (_WIN32)

DIR * opendir(const char *name)
{
    DIR *dir = 0;

    // printf("opendir %s\n", name);

    if (name && name[0]) {
        size_t base_length = strlen(name);
        const char *all = /* search pattern must end with suitable wildcard */
            strchr("/\\", name[base_length - 1]) ? "*" : "/*";

        if ((dir = (DIR *)malloc(sizeof *dir)) != 0 &&
            (dir->name = (char *)malloc(base_length + strlen(all) + 1)) != 0) {
            strcat(strcpy(dir->name, name), all);

            // printf("filter: %s\n", dir->name);

            if ((dir->handle =
                     (handle_type)_findfirst(dir->name, &dir->info)) != -1) {
                dir->result.d_name = 0;

                // printf("findfirst: %s\n", dir->info.name);


            } else { /* rollback */
                free(dir->name);
                free(dir);
                dir = 0;
            }
        } else { /* rollback */
            free(dir);
            dir = 0;
            errno = ENOMEM;
        }
    } else {
        errno = EINVAL;
    }

    return dir;
} /* opendir */

int closedir(DIR *dir)
{
    int result = -1;

    if (dir) {
        if (dir->handle != -1) {
            result = _findclose(dir->handle);
        }

        free(dir->name);
        free(dir);
    }

    if (result == -1) { /* map all errors to EBADF */
        errno = EBADF;
    }

    return result;
}

struct dirent * readdir(DIR *dir)
{
    struct dirent *result = 0;

    if (dir && dir->handle != -1) {
        if (!dir->result.d_name || _findnext(dir->handle, &dir->info) != -1) {

            // printf("findnext: %s\n", dir->info.name);


            result = &dir->result;
            result->d_name = dir->info.name;
        } else {
            // printf("findnext: empty\n");

        }
    } else {
        errno = EBADF;

        // printf("findnext: EBADF\n");

    }

    return result;
} /* readdir */

void rewinddir(DIR *dir)
{
    if (dir && dir->handle != -1) {
        _findclose(dir->handle);
        dir->handle = (handle_type)_findfirst(dir->name, &dir->info);
        dir->result.d_name = 0;
    } else {
        errno = EBADF;
    }
}

#endif /* if defined (_WIN32) */