#include <stdio.h>
#include <stdlib.h>

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

int
main(int argc, char **argv)
{
    int idx;

    printf("Program args:\n");
    print_argv(argc, argv);

    if ((idx = process_args(argc, argv)) < 0) {
        exit(99);
    }

    printf("To-be executed args:\n");
    print_argv(argc-idx, argv+idx);


    return 0;
}
