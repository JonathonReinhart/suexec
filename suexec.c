#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include <grp.h>

/**
 * SYNOPSIS
 *      suexec [options] user argument...
 */

#define APPNAME             "suexec"

#define errmsg(fmt, ...)     fprintf(stderr, APPNAME ": " fmt, ##__VA_ARGS__)
#define verbose(fmt, ...)    errmsg(fmt, ##__VA_ARGS__)

static const char *m_user;

/**
 * Returns the index into argv where the to-be-executed argv begins.
 */
static int 
process_args(int argc, char **argv)
{
    int i;

    /**
     * Process options
     * All elements of argv up to and including the user argument are consumed.
     */
    int done = 0;
    for (i = 1; i < argc && !done; i++) {
        char opt;

        if (argv[i][0] != '-') {
            m_user = argv[i];
            verbose("Processing 'user' argument: \"%s\"\n", m_user);

            /* All remaining arguments are for execv() */
            done = 1;
            continue;
        }

        verbose("Processing option string: \"%s\"\n", argv[i]);
        opt = argv[i][1];
        switch (opt) {

            default:
                errmsg("Invalid option '%c'\n", opt);
                return -1;
        }
    }

    if (m_user == NULL) {
        errmsg("Missing 'user' argument\n");
        return -1;
    }

    if (i == argc) {
        errmsg("Missing command\n");
        return -1;
    }

    return i;
}

static void
print_argv(int argc, char **argv)
{
    int i;

    /* <= is intentional; include NULL terminator */
    for (i = 0; i <= argc; i++) {
        printf("   [%d] = \"%s\"\n", i, argv[i]);
    }
}

static int
change_user(const char *username)
{
    struct passwd *pw;

    errno = 0;

    if (!(pw = getpwnam(username))) {
        errmsg("Failed to get pwd entry for user \"%s\": %m\n", username);
        return -1;
    }

    if (chdir(pw->pw_dir) != 0) {
        errmsg("Failed to chdir to home dir %s: %m\n", pw->pw_dir);
        return -1;
    }
    verbose("Changed working directory to %s\n", pw->pw_dir);

    if (setgroups(0, NULL) != 0) {
        errmsg("Failed to setgroups(): %m\n");
        return -1;
    }
    verbose("Cleared supplementary group list\n");

    if (setgid(pw->pw_gid) != 0) {
        errmsg("Failed to setgid(%u): %m\n", (unsigned int)pw->pw_gid);
        return -1;
    }

    if (setuid(pw->pw_uid) != 0) {
        errmsg("Failed to setuid(%u): %m\n", (unsigned int)pw->pw_uid);
        return -1;
    }

    verbose("Changed to uid=%u euid=%u  gid=%u egid=%u\n",
            getuid(), geteuid(), getgid(), getegid());

    return 0;
}

int
main(int argc, char **argv)
{
    char **new_argv;
    int new_argc;
    int idx;

    /**
     * This is the assumption that execv() makes.
     * This appears to be true on every system I've tried, although I cannot
     * find anything that guarantees this. So make sure the element after the
     * last is a NULL terminator. If we find that this faults on a system, we
     * can do the annoying and expensive duplication.
     */
    argv[argc] = NULL;

    printf("Program args:\n");
    print_argv(argc, argv);

    if ((idx = process_args(argc, argv)) < 0) {
        exit(99);
    }

    new_argc = argc - idx;
    new_argv = argv + idx;

    printf("To-be executed args:\n");
    print_argv(new_argc, new_argv);

    if (change_user(m_user) < 0)
        exit(99);

    execvp(new_argv[0], new_argv);

    errmsg("execv() failed: %m\n");
    exit(99);
}
