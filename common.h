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

typedef enum { EK_PROJECT, EK_PIPELINE } entity_kind_t;

typedef struct {
  entity_kind_t ent_kind;
  sds ent_fetch_data;
  union {
    pipeline_t ent_pipeline;
    project_t ent_project;
  } ent_e;
} entity_t;

static void entity_init(entity_t *entity, entity_kind_t kind) {
  entity->ent_kind = kind;
  entity->ent_fetch_data = sdsempty();
}

typedef struct {
  sds arg_base_url, arg_gitlab_token;
  u64 *arg_project_ids;

  project_t *arg_projects;
  lstack_t arg_channel;
} args_t;

static args_t args;

static void args_init(args_t *args) {
  args->arg_base_url = sdsempty();
  args->arg_gitlab_token = sdsempty();
  lstack_init(&args->arg_channel, 500);
}

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

static void pipeline_release(pipeline_t *pipeline) {
  sdsfree(pipeline->pip_vcs_ref);
  pipeline->pip_vcs_ref = NULL;
  sdsfree(pipeline->pip_url);
  pipeline->pip_url = NULL;
  sdsfree(pipeline->pip_created_at);
  pipeline->pip_created_at = NULL;
  sdsfree(pipeline->pip_updated_at);
  pipeline->pip_updated_at = NULL;
  sdsfree(pipeline->pip_status);
  pipeline->pip_status = NULL;
  sdsfree(pipeline->pip_project_path_with_namespace);
  pipeline->pip_project_path_with_namespace = NULL;
}
