#include <getopt.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

#include "api.h"
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
  api_init();

  char* argv0 = argv[0];

  args_init(&args);

  struct option options[] = {
      {.name = "token", .has_arg = optional_argument, .val = 't'},
      {.name = "url", .has_arg = required_argument, .val = 'u'},
      {0}};
  int index = 0, ch = 0;
  while ((ch = getopt_long(argc, argv, "ht:u:", options, &index)) != -1) {
    switch (ch) {
      case 'u':
        args.arg_base_url = sdscat(args.arg_base_url, optarg);
        break;
      case 't':
        args.arg_gitlab_token = sdscat(args.arg_gitlab_token, optarg);
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

  if (sdslen(args.arg_base_url) == 0) {
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
    buf_push(args.arg_project_ids, id);
  }
  buf_grow(args.arg_projects, buf_size(args.arg_project_ids));

  // ui_init();

  pthread_t fetch_thread;
  pthread_create(&fetch_thread, NULL, fetch, &args);

  // ui_draw(&args);
  entity_t* entity = NULL;
  while (true) {
    entity = lstack_pop(&args.arg_channel);
    if (!entity) continue;

    if (entity->ent_kind == EK_PROJECT) {
      project_t* project = &entity->ent_e.ent_project;
      printf("Project: id=%lld name=%s path=%s\n", project->pro_id,
             project->pro_name, project->pro_path_with_namespace);
    } else if (entity->ent_kind == EK_PIPELINE) {
      pipeline_t* pipeline = &entity->ent_e.ent_pipeline;
      printf("Pipeline: id=%lld created=%s updated=%s status=%s ref=%s\n",
             pipeline->pip_id, pipeline->pip_created_at,
             pipeline->pip_updated_at, pipeline->pip_status,
             pipeline->pip_vcs_ref);
    }
    /* entity_pop(entities, entity); */
    entity_release(entity);
  }
}
