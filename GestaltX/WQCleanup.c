#include "WQCleanup.h"

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <unistd.h>

int wq_remove_owned_symlink(const char *path, const char *expected_target) {
    if (!path || !expected_target) return EINVAL;

    struct stat info;
    if (lstat(path, &info) != 0) return errno == ENOENT ? 0 : errno;
    if (!S_ISLNK(info.st_mode)) return EINVAL;

    char link_target[PATH_MAX];
    ssize_t length = readlink(path, link_target, sizeof(link_target));
    if (length < 0) return errno;
    if ((size_t)length == sizeof(link_target)) return ENAMETOOLONG;

    size_t expected_length = strlen(expected_target);
    if ((size_t)length != expected_length ||
        memcmp(link_target, expected_target, expected_length) != 0) {
        return EPERM;
    }

    return unlink(path) == 0 ? 0 : errno;
}
