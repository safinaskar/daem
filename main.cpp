#define _POSIX_C_SOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

#include <unistd.h>
#include <time.h>

#include <libsh.h>

void set_cloexec(int fd)
{
  sh_x_fcntl(fd, F_SETFD, sh_x_fcntl(fd, F_GETFD) | FD_CLOEXEC);
}

int main(int argc, char *argv[])
{
  /* ---- Init ---- */

  sh_init (argv[0]);

  if (argc < 2)
    {
      sh_x_fprintf(stderr, "Usage: %s PROGRAM [ARG]...\n", argv[0]);
      exit (EXIT_FAILURE);
    }

  /* ---- stdin and stdout. Это нужно сделать до pipe, так как иначе концы пайпа могут получить номера 0 или 1 и мне лень думать, что будет в этом случае ---- */

  close(0);

  sh_x_open("/dev/null", O_RDWR);

  sh_x_dup2(0, 1);

  /* ---- stderr. Это нужно сделать после stdin и stdout, иначе err_fd может получить номер 0 или 1, и мне снова лень думать ---- */

  {
    int err_fd = dup(2);

    if (err_fd != -1){
      set_cloexec(err_fd);

      sh_set_err (sh_x_fdopen (err_fd, "w"));

      fclose(stderr);
    }
  }

  /* ---- stderr ---- */

  sh_x_dup2(0, 2);

  /* ---- etc ---- */

  signal(SIGHUP, SIG_IGN);

  int pipefd[2];
  sh_x_pipe(pipefd);

  if (sh_safe_fork() == 0){
    /* Child */

    sh_x_close(pipefd[0]);
    set_cloexec(pipefd[1]);

    sh_x_setsid();

    if (sh_safe_fork() != 0){
      /* Parent */
      exit (EXIT_SUCCESS);
    }

    /* Child */
    sh_x_execvp(argv[1], argv + 1);

    /* NOTREACHED */
  }

  sh_x_close(pipefd[1]);

  {
    char c;
    sh_x_read(pipefd[0], &c, 1);
  }
}
