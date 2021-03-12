#include <getopt.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "api.h"
#include "common.h"
#include "ui.h"

static void help_print(char* program_name) {
  printf(
      // clang-format off
      "Usage: %s [options...] project_id...\n\n"
      "  -u, --url <url>    The url where gitlab resides, e.g. `https://gitlab.com`\n"
      "  -t, --token Private Gitlab token. Do not share this with others\n"
      "  -h, --help Show this message\n"
      // clang-format on
      ,
      program_name);
}

int main(int argc, char* argv[]) {
  char* argv0 = argv[0];

  args_t args = {0};
  args_init(&args);

  struct option options[] = {
      {.name = "token", .has_arg = optional_argument, .val = 't'},
      {.name = "url", .has_arg = required_argument, .val = 'u'},
      {0}};
  int index = 0, ch = 0;
  while ((ch = getopt_long(argc, argv, "ht:u:", options, &index)) != -1) {
    switch (ch) {
      case 'u':
        args.base_url = sdscat(args.base_url, optarg);
        break;
      case 't':
        args.token = sdscat(args.token, optarg);
        break;
      case 'h':
      case '?':
      default:
        help_print(argv0);
        return 0;
    }
  }
  argc -= optind;
  argv += optind;

  if (sdslen(args.base_url) == 0) {
    fprintf(stderr, "Missing url.\n");
    help_print(argv0);
    return 1;
  }

  for (int i = 0; i < argc; i++) {
    char* end = NULL;
    uint64_t id = strtoull(argv[i], &end, 10);
    if (end != NULL && *end != 0) {
      fprintf(stderr, "Invalid project id: %s\n", argv[i]);
      help_print(argv0);
      return 1;
    }
    buf_push(args.project_ids, id);
  }
  buf_grow(projects, buf_size(args.project_ids));

  pthread_mutex_init(&projects_lock, NULL);
  projects_fetch(&args);

  pthread_t fetch_thread;
  pthread_create(&fetch_thread, NULL, fetch, &args);

  ui_init();
  ui_draw();

  pthread_join(fetch_thread, NULL);
}
