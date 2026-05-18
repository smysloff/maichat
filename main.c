
// file: main.c

#include "util.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/wait.h>

#define BUFFER_STD_SIZE 4096
#define COMMAND_MAX_SIZE 8
#define HOST_MAX_SIZE 1024

typedef struct {
  int fd;
  struct sockaddr_storage addr;
  socklen_t len;
} socket_t;

typedef enum {
  APP_STATE_QUIT,
  APP_STATE_WELCOME,
  APP_STATE_INPUT,
  APP_STATE_PARSE,
  APP_STATE_DISPATCH,
  APP_STATE_SERVER,
  APP_STATE_CLIENT,
} app_state_t;


void sig_chld(int signo)
{
  pid_t pid;
  int stat;

  (void) signo;

  while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
    printf("child %d terminated\n", pid);

  return;
}

typedef void sigfunc(int);

void *Signal(int signo, sigfunc *func)
{
  struct sigaction act, oact;

  act.sa_handler = func;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;

  if (signo == SIGALRM) {
#ifdef SA_INTERRUPT
  act.sa_flags |= SA_INTERRUPT; // SunOS 4.x
#endif
  } else {
#ifdef SA_RESTART
  act.sa_flags |= SA_RESTART; // SRV4.44BSD
#endif
  }

  if ((sigaction(signo, &act, &oact) < 0))
    return SIG_ERR;

  return oact.sa_handler;
}


app_state_t step_welcome(void);
app_state_t step_input(char *buf);
app_state_t step_parse(const char *buf, char **cmd, char **arg);
app_state_t step_dispatch(const char *cmd);
app_state_t step_server(socket_t *sock, const char *arg);
app_state_t step_client(socket_t *sock, const char *arg);


int main(int argc, char *argv[])
{
  (void) argc;
  (void) argv;

  static socket_t sock = {0};
  char *buf = NULL;
  char *cmd = NULL;
  char *arg = NULL;

  app_state_t state = APP_STATE_WELCOME;

  while (state != APP_STATE_QUIT) {
    switch (state) {
      case APP_STATE_WELCOME:  state = step_welcome();              break;
      case APP_STATE_INPUT:    state = step_input(buf);             break;
      case APP_STATE_PARSE:    state = step_parse(buf, &cmd, &arg); break;
      case APP_STATE_DISPATCH: state = step_dispatch(cmd);          break;
      case APP_STATE_SERVER:   state = step_server(&sock, arg);     break;
      case APP_STATE_CLIENT:   state = step_client(&sock, arg);     break;
      default: break;
    }
  }

  free(buf);
  free(cmd);
  free(arg);

  return EXIT_SUCCESS;
}


app_state_t step_welcome(void)
{
  printf(
    "This is MaiChat - a simple messaging application" "\n"
    "type:" "\n"
    "/exit or /quit      - to exit the app" "\n"
    "/help               - to view help" "\n"
    "/join <host>:<port> - to join an existing server" "\n"
    "/serv <host>:<port> - to start new server" "\n"
  );

  return APP_STATE_INPUT;
}

app_state_t step_input(char *buf)
{
  printf("> ");

  if ((buf = readline(stdin)) == NULL) {
    fprintf(stderr, "error: unable to get user command" "\n");
    return APP_STATE_INPUT;
  }

  return APP_STATE_PARSE;
}

// @todo refactor!
// maybe create another state manager?
app_state_t step_parse(const char *buf, char **cmd, char **arg)
{
  size_t len;
  const char *left, *right;

  // find '/'
  for (left = buf; *left != '/'; ++left) {
    if (*left == '\0') {
      fprintf(stderr, "error: unable to parse command" "\n");
      return APP_STATE_INPUT;
    }
  }

  // move to first char of command
  ++left;
  if (*left == '\0') {
    fprintf(stderr, "error: unable to parse command" "\n");
    return APP_STATE_INPUT;
  }

  // find end of command name
  for (right = left + 1; *right != ' '; ++right)
    if (*right == '\0')
      break;
  --right;

  // copy command from buffer
  len = right - left + 1;
  if (len >= sizeof(cmd)) {
    fprintf(stderr, "error: command is too long" "\n");
    return APP_STATE_INPUT;
  }
  memcpy(cmd, left, len);
  cmd[len] = '\0';

  // find first char of argument
  for (left = right + 1; *left == ' '; ++left)
    if (*left == '\0')
      return APP_STATE_DISPATCH;

  // find last char of arg
  for (right = left + 1; *right != '\0'; ++right)
    ;
  --right;

  // copy to arg
  len = right - left + 1;
  if (len >= sizeof(arg)) {
    fprintf(stderr, "error: arg is too long" "\n");
    return APP_STATE_INPUT;
  }
  memcpy(arg, left, len);
  arg[len] = '\0';

  return APP_STATE_DISPATCH;
}

// @todo refactor!
// create sructure to O1 search
app_state_t step_dispatch(const char *cmd)
{
  if ((strcmp(cmd, "exit") == 0) || (strcmp(cmd, "quit") == 0))
    return APP_STATE_QUIT;

  if ((strcmp(cmd, "help") == 0)) {
    fprintf(stderr, "warn: help page is not implemented yet" "\n");
    return APP_STATE_INPUT;
  }

  if ((strcmp(cmd, "serv") == 0))
    return APP_STATE_SERVER;

  if ((strcmp(cmd, "join") == 0))
    return APP_STATE_CLIENT;

  fprintf(stderr, "error: unknown cmd '%s'" "\n", cmd);
  return APP_STATE_INPUT;
}



app_state_t step_server(socket_t *sock, const char *arg)
{
  char host[1024];
  uint16_t port;
  size_t len;
  char *delim;

  // @todo add IPv6 support
  struct sockaddr_in *addr;
  socket_t client;
  pid_t child_pid;

  if (!arg) {
    fprintf(stderr, "error: can't start server without address info" "\n");
    return APP_STATE_INPUT;
  }

  if ((delim = strrchr(arg, ':')) == NULL) {
    fprintf(stderr, "error: address must be in format <host>:<port>" "\n");
    return APP_STATE_INPUT;
  }

  len = delim - arg;
  if (len >= sizeof(host)) {
    fprintf(stderr, "error: <host> is too long" "/n");
    return APP_STATE_INPUT;
  }
  memcpy(host, arg, len);
  host[len] = '\0';

  port = atoi(++delim);
  if (port == 0) {
    fprintf(stderr, "error: address must be in format <host>:<port>" "\n");
    return APP_STATE_INPUT;
  }

  sock->len = sizeof(struct sockaddr_in);
  addr = (struct sockaddr_in *) &sock->addr;
  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  if (!inet_pton(AF_INET, host, &addr->sin_addr.s_addr)) {
    fprintf(stderr, "error: unable to parse host" "\n");
    return APP_STATE_INPUT;
  }

  if ((sock->fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
    fprintf(stderr, "error: socket()" "\n");
    return APP_STATE_INPUT;
  }

  if ((bind(sock->fd, (struct sockaddr *) &sock->addr, sock->len) == -1)) {
    fprintf(stderr, "error: bind()" "\n");
    return APP_STATE_INPUT;
  }

  if ((listen(sock->fd, SOMAXCONN)) == -1) {
    fprintf(stderr, "error: listen()" "\n");
    return APP_STATE_INPUT;
  }

  Signal(SIGCHLD, sig_chld);

  while (true) {

    char buf[128];
    ssize_t bytes;

    client.len = sizeof(client.addr);
    memset(&client.addr, 0, client.len);

    client.fd = accept(sock->fd, (struct sockaddr *) &client.addr, &client.len);
    if (client.fd == -1) {
      if (errno == EINTR)
        continue;
      fprintf(stderr, "accept() error" "\n");
      break;
    }

    if ((child_pid = fork()) == 0) {
      close(sock->fd);
      bytes = read(client.fd, buf, sizeof(buf) - 1);
      if (bytes > 0) {
        write(client.fd, buf, bytes);
        write(STDOUT_FILENO, buf, bytes);
      }
      close(client.fd);
      exit(EXIT_SUCCESS);
    } else {
      close(client.fd);
    }

  }

  close(sock->fd);
  return APP_STATE_INPUT;
}

app_state_t step_client(socket_t *sock, const char *arg)
{
  fprintf(stdout, "start client with argument: %s" "\n", arg);
  fprintf(stderr, "warn: client code is not implemented yet" "\n");
  return APP_STATE_INPUT;
}
