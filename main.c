#include "api.h"
#include "ui.h"

int main() {
#if 0
  i64 *project_ids = NULL;
  buf_push(project_ids, 3472737);
  buf_push(project_ids, 278964);

  curl_global_init(CURL_GLOBAL_ALL);

  buf_trunc(json_tokens, 10 * 1024);  // 10 KiB

  projects_fetch(project_ids);
  pipelines_fetch();
#endif
  pipeline_t* pipelines1 = NULL;
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 1000000,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/100"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("pending")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 101,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/101"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 102,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/102"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 103,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/103"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 104,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/104"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 105,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/105"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 106,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/106"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 107,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/107"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 108,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/108"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 109,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/109"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 110,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/110"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 111,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/111"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 112,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/112"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 113,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/113"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 114,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/114"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 115,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/115"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 116,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/116"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 117,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/117"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 118,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/118"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 119,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/119"),
                         .pip_created_at = sdsnew("2021-01-02T03:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-02T03:06:05Z"),
                         .pip_status = sdsnew("succeeded")}));
  buf_push(pipelines1,
           ((pipeline_t){.pip_id = 120,
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
  buf_push(pipelines2,
           ((pipeline_t){.pip_id = 201,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/201"),
                         .pip_created_at = sdsnew("2021-01-02T04:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-09T03:06:05Z"),
                         .pip_status = sdsnew("failed")}));
  buf_push(pipelines2,
           ((pipeline_t){.pip_id = 20002,
                         .pip_vcs_ref = sdsnew("git://gitlab.git"),
                         .pip_url = sdsnew(
                             "https://gitlab.com/bsd/freebsd/pipelines/202"),
                         .pip_created_at = sdsnew("2021-01-02T04:04:05Z"),
                         .pip_updated_at = sdsnew("2021-01-07T03:06:05Z"),
                         .pip_status = sdsnew("failed")}));
  buf_push(projects, ((project_t){
                         .pro_id = 1,
                         .pro_name = sdsnew("Freebsd"),
                         .pro_path_with_namespace = sdsnew("bsd/Freebsd"),
                         .pro_pipelines = pipelines2,
                     }));
  ui_init();
  ui_draw();
}
