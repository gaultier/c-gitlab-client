#pragma once

#include <curl/curl.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
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

#define CLAMP(value, min, max) \
  do {                         \
    if ((value) < (min))       \
      (value) = (min);         \
    else if ((value) > (max))  \
      (value) = (max);         \
  } while (0)

#define LEN0(s) (sizeof(s) - 1)

#define STR(s) #s

#define CHECK(a, cond, b, fmt)                                                \
  do {                                                                        \
    if (!((a)cond(b))) {                                                      \
      fprintf(stderr,                                                         \
              __FILE__ ":%d:CHECK failed: %s " STR(                           \
                  cond) " %s i.e.: " fmt " " STR(cond) " " fmt " is false\n", \
              __LINE__, STR(a), STR(b), a, b);                                \
      assert(0);                                                              \
    }                                                                         \
  } while (0)

typedef int64_t i64;
typedef uint64_t u64;
typedef uint16_t u16;

typedef struct {
  i64 pip_id, pip_project_id, pip_duration_second;
  sds pip_vcs_ref, pip_url, pip_created_at, pip_updated_at, pip_started_at,
      pip_finished_at, pip_status, pip_project_path_with_namespace,
      pip_duration, pip_id_s;
  time_t pip_created_at_time, pip_updated_at_time, pip_started_at_time,
      pip_finished_at_time;
} pipeline_t;

typedef struct {
  i64 pro_id;
  sds pro_path_with_namespace;
} project_t;

typedef enum {
  EK_FETCH_PROJECT,
  EK_FETCH_PIPELINES,
  EK_FETCH_PIPELINE,
  EK_PROJECT,
  EK_PIPELINE
} entity_kind_t;

struct entity_t {
  union {
    pipeline_t ent_pipeline;
    project_t ent_project;
  } ent_e;
  entity_kind_t ent_kind;
  sds ent_fetch_data, ent_api_url;
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
  project->pro_path_with_namespace = sdsempty();
}

static void pipeline_init(pipeline_t *pipeline, i64 project_id) {
  pipeline->pip_id = 0;
  pipeline->pip_project_id = project_id;
  pipeline->pip_vcs_ref = sdsempty();
  pipeline->pip_url = sdsempty();
  pipeline->pip_created_at = sdsempty();
  pipeline->pip_updated_at = sdsempty();
  pipeline->pip_started_at = sdsempty();
  pipeline->pip_finished_at = sdsempty();
  pipeline->pip_status = sdsempty();
  pipeline->pip_project_path_with_namespace = sdsempty();
  pipeline->pip_duration = sdsempty();
  pipeline->pip_id_s = sdsempty();
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

  sdsfree(pipeline->pip_started_at);
  pipeline->pip_started_at = NULL;

  sdsfree(pipeline->pip_finished_at);
  pipeline->pip_finished_at = NULL;

  sdsfree(pipeline->pip_status);
  pipeline->pip_status = NULL;

  sdsfree(pipeline->pip_duration);
  pipeline->pip_duration = NULL;

  sdsfree(pipeline->pip_id_s);
  pipeline->pip_id_s = NULL;

  // Don't free it since we do not own it
  pipeline->pip_project_path_with_namespace = NULL;
}

static void project_release(project_t *project) {
  sdsfree(project->pro_path_with_namespace);
  project->pro_path_with_namespace = NULL;
}

static entity_t *entity_new(entity_kind_t kind) {
  entity_t *entity = calloc(1, sizeof(entity_t));
  assert(entity);

  entity->ent_kind = kind;
  entity->ent_fetch_data = sdsempty();
  entity->ent_api_url = sdsempty();
  return entity;
}

static void project_shrink(project_t *project) {
  assert(project);

  if (project->pro_path_with_namespace)
    project->pro_path_with_namespace =
        sdsRemoveFreeSpace(project->pro_path_with_namespace);
}

static void pipeline_shrink(pipeline_t *pipeline) {
  assert(pipeline);

  if (pipeline->pip_vcs_ref)
    pipeline->pip_vcs_ref = sdsRemoveFreeSpace(pipeline->pip_vcs_ref);

  if (pipeline->pip_url)
    pipeline->pip_url = sdsRemoveFreeSpace(pipeline->pip_url);

  if (pipeline->pip_created_at)
    pipeline->pip_created_at = sdsRemoveFreeSpace(pipeline->pip_created_at);

  if (pipeline->pip_updated_at)
    pipeline->pip_updated_at = sdsRemoveFreeSpace(pipeline->pip_updated_at);

  if (pipeline->pip_started_at)
    pipeline->pip_started_at = sdsRemoveFreeSpace(pipeline->pip_started_at);

  if (pipeline->pip_finished_at)
    pipeline->pip_finished_at = sdsRemoveFreeSpace(pipeline->pip_finished_at);

  if (pipeline->pip_status)
    pipeline->pip_status = sdsRemoveFreeSpace(pipeline->pip_status);

  if (pipeline->pip_duration)
    pipeline->pip_duration = sdsRemoveFreeSpace(pipeline->pip_duration);

  if (pipeline->pip_id_s)
    pipeline->pip_id_s = sdsRemoveFreeSpace(pipeline->pip_id_s);
}

static void entity_shrink(entity_t *entity) {
  assert(entity);

  sdsfree(entity->ent_fetch_data);
  entity->ent_fetch_data = NULL;

  sdsfree(entity->ent_api_url);
  entity->ent_api_url = NULL;

  if (entity->ent_kind == EK_PROJECT)
    project_shrink(&entity->ent_e.ent_project);
  else if (entity->ent_kind == EK_PIPELINE)
    pipeline_shrink(&entity->ent_e.ent_pipeline);
}

static void entity_release(entity_t *entity) {
  assert(entity);

  sdsfree(entity->ent_fetch_data);
  entity->ent_fetch_data = NULL;

  sdsfree(entity->ent_api_url);
  entity->ent_api_url = NULL;

  if (entity->ent_kind == EK_PROJECT)
    project_release(&entity->ent_e.ent_project);
  else if (entity->ent_kind == EK_PIPELINE)
    pipeline_release(&entity->ent_e.ent_pipeline);

  free(entity);
}

static int common_duration_second_to_short(char *res, u64 res_size,
                                           u64 duration) {
  CHECK(res_size, >=, 9LLU, "%llu");
  const int MINUTE = 60;
  const int HOUR = 60 * MINUTE;
  const int DAY = 24 * HOUR;
  const int MONTH = 30 * DAY;
  const int YEAR = 365 * DAY;

  if (duration < MINUTE)
    return sprintf(res, "%llus", duration);
  else if (duration < HOUR)
    return sprintf(res, "%llum", duration / MINUTE);
  else if (duration < DAY)
    return sprintf(res, "%lluh", duration / HOUR);
  else if (duration < MONTH)
    return sprintf(res, "%llud", duration / DAY);
  else if (duration < YEAR)
    return sprintf(res, "%lluM", duration / MONTH);
  else if (duration / YEAR < 100)
    return sprintf(res, "%lluy", duration / YEAR);
  else
    return sprintf(res, "?");
}
