#ifndef WQCleanup_h
#define WQCleanup_h

// Returns 0 when the path is absent or the expected symlink was removed;
// otherwise returns an errno-style code and leaves unexpected items untouched.
int wq_remove_owned_symlink(const char *path, const char *expected_target);

#endif
