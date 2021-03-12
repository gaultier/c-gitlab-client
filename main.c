#include <getopt.h>
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
  char* argv0 = argv[0];

  char *url = NULL, *token = NULL;

  struct option options[] = {
      {.name = "token", .has_arg = optional_argument, .val = 't'},
      {.name = "url", .has_arg = required_argument, .val = 'u'},
      {0}};
  int index = 0, ch = 0;
  while ((ch = getopt_long(argc, argv, "ht:u:", options, &index)) != -1) {
    switch (ch) {
      case 'u':
        url = optarg;
        break;
      case 't':
        token = optarg;
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

  if (!url) {
    fprintf(stderr, "Missing url.\n");
    help_print(argv0);
    return 1;
  }

  u64* project_ids = NULL;
  for (int i = 0; i < argc; i++) {
    char* end = NULL;
    uint64_t id = strtoull(argv[i], &end, 10);
    if (end != NULL && *end != 0) {
      fprintf(stderr, "Invalid project id: %s\n", argv[i]);
      help_print(argv0);
      return 1;
    }
    buf_push(project_ids, id);
  }

#ifndef TEST
  curl_global_init(CURL_GLOBAL_ALL);

  buf_trunc(json_tokens, 10 * 1024);  // 10 KiB

  projects_fetch(project_ids, url, token);
  pipelines_fetch(project_ids, url, token);
#else
  sds proj_name_1 = sdsnew("Nginx");
  pipeline_t* pipelines1 = NULL;
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 1000000,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/100"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("pending")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 101,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/101"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 102,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/102"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 103,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/103"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 104,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/104"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 105,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/105"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 106,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/106"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 107,
                         .pip_project_name = proj_name_1,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/107"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 108,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/108"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 109,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/109"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 110,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/110"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 111,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/111"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 112,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/112"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 113,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/113"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 114,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/114"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 115,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/115"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 116,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/116"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 117,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/117"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 118,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/118"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 119,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/119"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 120,
                         .pip_project_name = proj_name_1,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/120"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(projects, ((project_t){
                         .pro_id = 1,
                         .pro_name = sdsnew("Freebsd"),
                         .pro_path_with_namespace = sdsnew("bsd/Freebsd"),
                         .pro_pipelines = pipelines1,
                     }));
  pipeline_t* pipelines2 = NULL;
  sds proj_name_2 = sdsnew("Apache");
  buf_push(pipelines2,
           ((pipeline_t){.pip_id = 201,
                         .pip_project_name = proj_name_2,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/201"),
                         .pip_created_at = sdsnew("2021-01-02T04:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-09T03:06:05Z"),
                         .pip_status = sdsnew("failed")}));
  buf_push(pipelines2,
           ((pipeline_t){.pip_id = 20002,
                         .pip_project_name = proj_name_2,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/202"),
                         .pip_created_at = sdsnew("2021-01-02T04:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-07T03:06:05Z"),
                         .pip_status = sdsnew("canceled")}));
  buf_push(projects, ((project_t){
                         .pro_id = 1,
                         .pro_name = sdsnew("Freebsd"),
                         .pro_path_with_namespace = sdsnew("bsd/Freebsd"),
                         .pro_pipelines = pipelines2,
                     }));
#endif

  ui_init();
  ui_draw();
}
