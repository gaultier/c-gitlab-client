#pragma once

#include <curl/curl.h>
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "deps/buf/buf.h"
#include "deps/jsmn/jsmn.h"
#include "deps/lstack/lstack.c"
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
  i64 pip_id, pip_project_id;
  sds pip_vcs_ref, pip_url, pip_created_at, pip_updated_at, pip_status,
      pip_project_path_with_namespace;
} pipeline_t;

typedef struct {
  i64 pro_id;
  sds pro_name, pro_path_with_namespace;
} project_t;

typedef enum {
  EK_FETCH_PROJECT,
  EK_FETCH_PIPELINES,
  EK_FETCH_PIPELINE,
  EK_PROJECT,
  EK_PIPELINE
} entity_kind_t;

struct entity_t {
  entity_kind_t ent_kind;
  sds ent_fetch_data;
  struct entity_t *ent_next;
  union {
    pipeline_t ent_pipeline;
    project_t ent_project;
  } ent_e;
};
typedef struct entity_t entity_t;

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
}

static void pipeline_init(pipeline_t *pipeline, i64 project_id) {
  pipeline->pip_id = 0;
  pipeline->pip_project_id = project_id;
  pipeline->pip_vcs_ref = sdsempty();
  pipeline->pip_url = sdsempty();
  pipeline->pip_created_at = sdsempty();
  pipeline->pip_updated_at = sdsempty();
  pipeline->pip_status = sdsempty();
  pipeline->pip_project_path_with_namespace = sdsempty();
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
  // Don't free it since we do not own it
  pipeline->pip_project_path_with_namespace = NULL;
}

static void project_release(project_t *project) {
  sdsfree(project->pro_name);
  project->pro_name = NULL;
  /* sdsfree(project->pro_path_with_namespace); */
  project->pro_path_with_namespace = NULL;
}

static entity_t *entity_new(entity_kind_t kind) {
  entity_t *entity = calloc(1, sizeof(entity_t));
  entity->ent_kind = kind;
  entity->ent_fetch_data = sdsempty();
  return entity;
}

static void entity_push(entity_t *entities, entity_t *entity) {
  entity->ent_next = entities;
  entities = entity;
}

static void entity_pop(entity_t *entities, entity_t *entity) {
  entity_t *current = entities;
  entity_t *previous = entities;
  while (current && current->ent_next) {
    if (current == entity) {
      previous->ent_next = current->ent_next;
      return;
    }

    previous = current;
    current = current->ent_next;
  }
}

static void entity_release(entity_t *entity) {
  sdsfree(entity->ent_fetch_data);
  entity->ent_fetch_data = NULL;

  if (entity->ent_kind == EK_PROJECT)
    project_release(&entity->ent_e.ent_project);
  else if (entity->ent_kind == EK_PIPELINE)
    pipeline_release(&entity->ent_e.ent_pipeline);

  free(entity);
}
