#ifndef PYTYLIKI_DIVISION_CLI_H
#define PYTYLIKI_DIVISION_CLI_H

/* Dispatches division and partition planning commands.
 * Returns -1 when argv[1] belongs to the main CLI, otherwise an exit code. */
int pl_division_cli(int argc, char **argv);

#endif
