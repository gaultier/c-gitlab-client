#pragma once

#include <curl/curl.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "deps/buf/buf.h"
#include "deps/jsmn/jsmn.h"
#include "deps/lstack/lstack.h"
#include "deps/sds/sds.c"
#include "deps/sds/sds.h"
#include "deps/sds/sdsalloc.h"

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) ((x) < (y) ? (y) : (x))
#endif

#define LEN0(s) (sizeof(s) - 1)

typedef int64_t i64;
typedef uint64_t u64;
typedef uint16_t u16;

typedef struct {
  i64 pip_id;
  sds pip_vcs_ref, pip_url, pip_created_at, pip_updated_at, pip_status,
      pip_project_path_with_namespace;
} pipeline_t;

typedef struct {
  i64 pro_id;
  sds pro_name, pro_path_with_namespace, pro_api_data;
} project_t;

typedef struct {
  sds base_url, token;
  u64 *project_ids;

  project_t *projects;
  lstack_t pipelines;
} args_t;

static void args_init(args_t *args) {
  args->base_url = sdsempty();
  args->token = sdsempty();
  lstack_init(&args->pipelines, 500);
}

static args_t args;

static void project_init(project_t *project, i64 id) {
  project->pro_id = id;
  project->pro_name = sdsempty();
  project->pro_path_with_namespace = sdsempty();
  project->pro_api_data = sdsempty();
}

static void pipeline_init(pipeline_t *pipeline, sds project_name) {
  pipeline->pip_id = 0;
  pipeline->pip_vcs_ref = sdsempty();
  pipeline->pip_url = sdsempty();
  pipeline->pip_created_at = sdsempty();
  pipeline->pip_updated_at = sdsempty();
  pipeline->pip_status = sdsempty();
  pipeline->pip_project_path_with_namespace = sdsdup(project_name);
}
