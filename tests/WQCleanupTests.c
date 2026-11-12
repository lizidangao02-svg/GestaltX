#include "../GestaltX/WQCleanup.h"

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "check failed at line %d: %s\n", __LINE__, #condition); \
        return 1; \
    } \
} while (0)

static int write_file(const char *path, const char *contents) {
    FILE *file = fopen(path, "w");
    if (!file) return -1;
    int result = fputs(contents, file);
    return fclose(file) == 0 && result >= 0 ? 0 : -1;
}

int main(void) {
    char root[] = "/tmp/wq-cleanup-tests.XXXXXX";
    CHECK(mkdtemp(root) != NULL);

    char target[PATH_MAX], other_target[PATH_MAX], path[PATH_MAX], child[PATH_MAX];
    CHECK(snprintf(target, sizeof(target), "%s/target", root) < (int)sizeof(target));
    CHECK(snprintf(other_target, sizeof(other_target), "%s/other-target", root) < (int)sizeof(other_target));
    CHECK(snprintf(path, sizeof(path), "%s/Caches", root) < (int)sizeof(path));
    CHECK(write_file(target, "target data") == 0);
    CHECK(write_file(other_target, "other data") == 0);

    CHECK(wq_remove_owned_symlink(path, target) == 0);

    CHECK(mkdir(path, 0700) == 0);
    CHECK(snprintf(child, sizeof(child), "%s/keep", path) < (int)sizeof(child));
    CHECK(write_file(child, "preserve directory contents") == 0);
    CHECK(wq_remove_owned_symlink(path, target) == EINVAL);
    CHECK(access(child, F_OK) == 0);
    CHECK(unlink(child) == 0);
    CHECK(rmdir(path) == 0);

    CHECK(write_file(path, "preserve file contents") == 0);
    CHECK(wq_remove_owned_symlink(path, target) == EINVAL);
    CHECK(access(path, F_OK) == 0);
    CHECK(unlink(path) == 0);

    CHECK(symlink(other_target, path) == 0);
    CHECK(wq_remove_owned_symlink(path, target) == EPERM);
    struct stat info;
    CHECK(lstat(path, &info) == 0 && S_ISLNK(info.st_mode));
    CHECK(unlink(path) == 0);

    CHECK(symlink(target, path) == 0);
    CHECK(wq_remove_owned_symlink(path, target) == 0);
    CHECK(lstat(path, &info) != 0 && errno == ENOENT);
    CHECK(access(target, F_OK) == 0);

    CHECK(mkdir(path, 0700) == 0);
    CHECK(snprintf(child, sizeof(child), "%s/keep", path) < (int)sizeof(child));
    CHECK(write_file(child, "preserve non-empty cache directory") == 0);
    CHECK(wq_remove_empty_directory(path) != 0);
    CHECK(access(child, F_OK) == 0);
    CHECK(unlink(child) == 0);
    CHECK(rmdir(path) == 0);

    CHECK(mkdir(path, 0700) == 0);
    CHECK(wq_remove_empty_directory(path) == 0);
    CHECK(access(path, F_OK) != 0 && errno == ENOENT);

    CHECK(unlink(target) == 0);
    CHECK(unlink(other_target) == 0);
    CHECK(rmdir(root) == 0);
    puts("WQ cleanup path tests passed");
    return 0;
}
