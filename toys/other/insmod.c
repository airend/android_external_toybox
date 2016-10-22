/* insmod.c - Load a module into the Linux kernel.
 *
 * Copyright 2012 Elie De Brauwer <eliedebrauwer@gmail.com>

USE_INSMOD(NEWTOY(insmod, "<1", TOYFLAG_SBIN|TOYFLAG_NEEDROOT))

config INSMOD
  bool "insmod"
  default y
  help
    usage: insmod MODULE [MODULE_OPTIONS]

    Load the module named MODULE passing options if given.
*/

#include "toys.h"

#ifndef NO_FINIT_MODULE
#include <sys/syscall.h>
#define finit_module(fd, opts, flags) syscall(SYS_finit_module, fd, opts, flags)
#define init_module(mod, len, opts) syscall(SYS_init_module, mod, len, opts)
#else
// System call provided by bionic but not in any header file.
extern int init_module(void *, unsigned long, const char *);
#endif

void insmod_main(void)
{
#ifndef NO_FINIT_MODULE
  int fd = !strcmp(*toys.optargs, "-") ? 0 : xopen(*toys.optargs, O_RDONLY);
#endif
  int i, rc;

  i = 1;
  while (toys.optargs[i] &&
    strlen(toybuf) + strlen(toys.optargs[i]) + 2 < sizeof(toybuf))
  {
    strcat(toybuf, toys.optargs[i++]);
    strcat(toybuf, " ");
  }

#ifndef NO_FINIT_MODULE
  // finit_module was new in Linux 3.8, and doesn't work on stdin,
  // so we fall back to init_module if necessary.
  rc = finit_module(fd, toybuf, 0);
  if (rc && (fd == 0 || errno == ENOSYS)) {
#endif
    off_t len = 0;
    char *path = !strcmp(*toys.optargs, "-") ? "/dev/stdin" : *toys.optargs;
    char *buf = readfileat(AT_FDCWD, path, NULL, &len);

    rc = init_module(buf, len, toybuf);
    if (CFG_TOYBOX_FREE) free(buf);
#ifndef NO_FINIT_MODULE
  }
#endif

  if (rc) perror_exit("failed to load %s", toys.optargs[0]);

#ifndef NO_FINIT_MODULE
  if (CFG_TOYBOX_FREE) close(fd);
#endif
}
