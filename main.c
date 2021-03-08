#include "api.h"

typedef int64_t i64;
typedef uint64_t u64;

int main() {
  i64 *project_ids = NULL;
  buf_push(project_ids, 3472737);
  buf_push(project_ids, 278964);

  curl_global_init(CURL_GLOBAL_ALL);

  buf_trunc(json_tokens, 10 * 1024);  // 10 KiB

  CURLM *cm;

  // Project
  {
    cm = curl_multi_init();
    for (u64 i = 0; i < buf_size(project_ids); i++) {
      const i64 id = project_ids[i];

      project_t project = {0};
      project_init(&project, id);
      buf_push(projects, project);
      project_fetch_queue(cm, i);
    }
    projects_fetch(cm);
    curl_multi_cleanup(cm);

    for (u64 i = 0; i < buf_size(project_ids); i++) {
      project_t *project = &projects[i];
      project_parse_json(project);
      printf("Project: id=%lld path_with_namespace=%s name=%s\n",
             project->pro_id, project->pro_path_with_namespace,
             project->pro_name);
    }
  }

  // Pipelines
  {
    cm = curl_multi_init();
    for (u64 i = 0; i < buf_size(project_ids); i++) {
      sdsclear(projects[i].pro_api_data);
      project_pipelines_fetch_queue(cm, i);
    }
    projects_fetch(cm);
    curl_multi_cleanup(cm);
    for (u64 i = 0; i < buf_size(project_ids); i++) {
      project_t *project = &projects[i];
      project_parse_pipelines_json(project);

      for (u64 j = 0; j < buf_size(project->pro_pipelines); j++) {
        const pipeline_t *const pipeline = &project->pro_pipelines[j];
        printf(
            "[%lld] Pipeline: id=%lld ref=%s created_at=%s updated_at=%s "
            "status=%s url=%s\n",
            project->pro_id, pipeline->pip_id, pipeline->pip_vcs_ref,
            pipeline->pip_created_at, pipeline->pip_updated_at,
            pipeline->pip_status, pipeline->pip_url);
      }
    }
  }
}
