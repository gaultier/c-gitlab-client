#pragma once

#include "common.h"

static jsmntok_t *api_json_tokens;
static char api_url[4097];
static CURLM *api_cm = NULL;

#define JSON_PARSE_KV_STRING(key, api_json_tokens, i, s, field) \
  do {                                                          \
    jsmntok_t *const tok = &api_json_tokens[i];                 \
    if (api_json_eq(s, tok, key, LEN0(key)) == 0) {             \
      const jsmntok_t *const t = &api_json_tokens[++i];         \
      const char *const value = s + t->start;                   \
      if (sdslen(field)) sdssetlen(field, 0);                   \
      field = sdscatlen(field, value, t->end - t->start);       \
      continue;                                                 \
    }                                                           \
  } while (0)

#define JSON_PARSE_KV_NUMBER(key, api_json_tokens, i, s, field) \
  do {                                                          \
    jsmntok_t *const tok = &api_json_tokens[i];                 \
    if (api_json_eq(s, tok, key, LEN0(key)) == 0) {             \
      const jsmntok_t *const t = &api_json_tokens[++i];         \
      if (t->type != JSMN_PRIMITIVE) break;                     \
      const char *const value = s + t->start;                   \
      field = strtoll(value, NULL, 10);                         \
      continue;                                                 \
    }                                                           \
  } while (0)

static void api_init() {
  curl_global_init(CURL_GLOBAL_ALL);
  api_cm = curl_multi_init();
}

static size_t api_curl_write_cb(char *data, size_t n, size_t l, void *userp) {
  entity_t *entity = userp;
  entity->ent_fetch_data = sdscatlen(entity->ent_fetch_data, data, n * l);

  return n * l;
}

static int api_json_eq(const char *json, const jsmntok_t *tok, const char *s,
                       u64 s_len) {
  if (tok->type == JSMN_STRING && ((int)s_len == tok->end - tok->start) &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

static void api_pipeline_json_status_parse(pipeline_t *pipeline, i64 *i,
                                           const char *s) {
  const char key[] = "status";
  jsmntok_t *const tok = &api_json_tokens[*i];
  if (api_json_eq(s, tok, key, LEN0(key)) == 0) {
    *i += 1;
    const jsmntok_t *const t = &api_json_tokens[*i];
    const char *const value = s + t->start;
    const int len = t->end - t->start;

    static const struct status_mapping_t {
      status_t status;
      char *s;
    } statuses[] = {
        {.status = ST_PENDING, .s = "pending"},
        {.status = ST_RUNNING, .s = "running"},
        {.status = ST_FAILED, .s = "failed"},
        {.status = ST_SUCCEEDED, .s = "succeeded"},
        {.status = ST_CANCELED, .s = "canceled"},
    };
    for (int j = 0; j < (int)ARR_SIZE(statuses); j++) {
      const struct status_mapping_t status = statuses[j];
      if (LEN0(status.s) == len && memcmp(status.s, value, len) == 0) {
        pipeline->pip_status = status.status;
        fprintf(stderr, "status=%d\n", pipeline->pip_status);
        return;
      }
    }
  }
}

static void api_project_parse_json(entity_t *entity, lstack_t *channel) {
  assert(entity->ent_kind == EK_FETCH_PROJECT);
  buf_trunc(api_json_tokens, 10 * 1024);  // 10 KiB
  buf_clear(api_json_tokens);

  jsmn_parser parser;
  jsmn_init(&parser);

  project_t *project = &entity->ent_e.ent_project;

  const char *const s = entity->ent_fetch_data;
  int res = jsmn_parse(&parser, s, sdslen((char *)s), api_json_tokens,
                       buf_capacity(api_json_tokens));
  if (res <= 0 || api_json_tokens[0].type != JSMN_OBJECT) {
    fprintf(stderr,
            "%s:%d:Malformed JSON for project: url=%s "
            "json=%s\n",
            __FILE__, __LINE__, entity->ent_api_url, entity->ent_fetch_data);
    entity_release(entity);
    return;
  }

  for (i64 i = 1; i < res; i++) {
    JSON_PARSE_KV_STRING("path_with_namespace", api_json_tokens, i, s,
                         project->pro_path_with_namespace);
  }
  entity->ent_kind = EK_PROJECT;

  entity_shrink(entity);
  lstack_push(channel, entity);
}

static void api_pipeline_queue_fetch(CURLM *cm, u64 project_id, u64 pipeline_id,
                                     args_t *args) {
  memset(api_url, 0, sizeof(api_url));

  if (args->arg_gitlab_token)
    snprintf(api_url, LEN0(api_url),
             "%s/api/v4/projects/%llu/pipelines/%llu?private_token=%s",
             args->arg_base_url, project_id, pipeline_id,
             args->arg_gitlab_token);
  else
    snprintf(api_url, LEN0(api_url), "%s/api/v4/projects/%llu/pipelines/%llu",
             args->arg_base_url, project_id, pipeline_id);

  entity_t *entity = entity_new(EK_FETCH_PIPELINE);
  entity->ent_api_url = sdsnew(api_url);
  CURL *eh = curl_easy_init();
  curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, api_curl_write_cb);
  curl_easy_setopt(eh, CURLOPT_URL, entity->ent_api_url);

  pipeline_init(&entity->ent_e.ent_pipeline, project_id);
  entity->ent_e.ent_pipeline.pip_id = pipeline_id;

  curl_easy_setopt(eh, CURLOPT_WRITEDATA, entity);
  curl_easy_setopt(eh, CURLOPT_PRIVATE, entity);
  curl_multi_add_handle(cm, eh);
}

static void api_pipeline_parse_json(entity_t *entity, lstack_t *channel) {
  assert(entity->ent_kind == EK_FETCH_PIPELINE);
  buf_trunc(api_json_tokens, 10 * 1024);  // 10 KiB
  buf_clear(api_json_tokens);

  jsmn_parser parser;
  jsmn_init(&parser);

  const char *const s = entity->ent_fetch_data;
  int res = jsmn_parse(&parser, s, sdslen((char *)s), api_json_tokens,
                       buf_capacity(api_json_tokens));
  if (res <= 0 || api_json_tokens[0].type != JSMN_OBJECT) {
    fprintf(stderr,
            "%s:%d:Malformed JSON for pipeline: url=%s "
            "json=%s\n",
            __FILE__, __LINE__, entity->ent_api_url, entity->ent_fetch_data);
    entity_release(entity);
    return;
  }

  pipeline_t *pipeline = &entity->ent_e.ent_pipeline;
  for (i64 i = 1; i < res; i++) {
    const jsmntok_t *key = &api_json_tokens[i];
    const int key_len = key->end - key->start;
    const char *const key_s = &s[key->start];
    // Skip 'user' object for now
    if (key->type == JSMN_STRING && key_len == LEN0("user") &&
        memcmp("user", key_s, key_len) == 0 &&
        api_json_tokens[i + 1].type == JSMN_OBJECT) {
      const jsmntok_t *const val = &api_json_tokens[i + 1];
      i += 1 + 2 * val->size;
      continue;
    }
    // Skip 'detailed_status' for now
    if (api_json_eq(s, &api_json_tokens[i], "detailed_status",
                    LEN0("detailed_status") == 0) &&
        api_json_tokens[i + 1].type == JSMN_OBJECT) {
      const jsmntok_t *const val = &api_json_tokens[i + 1];
      i += 1 + 2 * val->size;
      continue;
    }

    JSON_PARSE_KV_STRING("ref", api_json_tokens, i, s, pipeline->pip_vcs_ref);
    JSON_PARSE_KV_STRING("created_at", api_json_tokens, i, s,
                         pipeline->pip_created_at);
    JSON_PARSE_KV_STRING("updated_at", api_json_tokens, i, s,
                         pipeline->pip_updated_at);
    JSON_PARSE_KV_STRING("started_at", api_json_tokens, i, s,
                         pipeline->pip_started_at);
    JSON_PARSE_KV_STRING("finished_at", api_json_tokens, i, s,
                         pipeline->pip_finished_at);
    JSON_PARSE_KV_STRING("web_url", api_json_tokens, i, s, pipeline->pip_url);
    api_pipeline_json_status_parse(pipeline, &i, s);
  }
  // Post-processing
  entity->ent_kind = EK_PIPELINE;

  struct tm time = {0};
  if (strptime(pipeline->pip_created_at, "%FT%T", &time))
    pipeline->pip_created_at_time = timegm(&time);

  time = (struct tm){0};
  if (strptime(pipeline->pip_updated_at, "%FT%T", &time))
    pipeline->pip_updated_at_time = timegm(&time);

  time = (struct tm){0};
  if (strptime(pipeline->pip_started_at, "%FT%T", &time))
    pipeline->pip_started_at_time = timegm(&time);

  time = (struct tm){0};
  if (strptime(pipeline->pip_finished_at, "%FT%T", &time))
    pipeline->pip_finished_at_time = timegm(&time);

  pipeline->pip_id_s = sdsfromlonglong(pipeline->pip_id);

  entity_shrink(entity);
  lstack_push(channel, entity);
}

static void api_pipelines_parse_json(entity_t *dummy_entity, args_t *args) {
  assert(dummy_entity->ent_kind == EK_FETCH_PIPELINES);
  buf_trunc(api_json_tokens, 10 * 1024);  // 10 KiB
  buf_clear(api_json_tokens);

  jsmn_parser parser;
  jsmn_init(&parser);

  const char *const s = dummy_entity->ent_fetch_data;
  int res = jsmn_parse(&parser, s, sdslen((char *)s), api_json_tokens,
                       buf_capacity(api_json_tokens));
  if (res <= 0 || api_json_tokens[0].type != JSMN_ARRAY) {
    entity_release(dummy_entity);
    return;
  }

  entity_t *e_pipeline = NULL;
  pipeline_t *pipeline = NULL;
  for (i64 i = 1; i < res; i++) {
    const jsmntok_t *const tok = &api_json_tokens[i];
    if (tok->type == JSMN_OBJECT) {
      if (e_pipeline) {
        api_pipeline_queue_fetch(api_cm,
                                 e_pipeline->ent_e.ent_pipeline.pip_project_id,
                                 e_pipeline->ent_e.ent_pipeline.pip_id, args);
        // Post-processing
        CHECK(e_pipeline->ent_kind, ==, EK_PIPELINE, "%d");
        struct tm time = {0};
        strptime(pipeline->pip_created_at, "%FT%T", &time);
        pipeline->pip_created_at_time = timegm(&time);
        strptime(pipeline->pip_updated_at, "%FT%T", &time);
        pipeline->pip_updated_at_time = timegm(&time);
        pipeline->pip_id_s = sdsfromlonglong(pipeline->pip_id);

        entity_shrink(e_pipeline);
        lstack_push(&args->arg_channel, e_pipeline);
      }
      e_pipeline = entity_new(EK_PIPELINE);
      pipeline = &e_pipeline->ent_e.ent_pipeline;
      pipeline_init(pipeline, dummy_entity->ent_e.ent_pipeline.pip_project_id);
      continue;
    }

    JSON_PARSE_KV_NUMBER("id", api_json_tokens, i, s, pipeline->pip_id);
    JSON_PARSE_KV_STRING("ref", api_json_tokens, i, s, pipeline->pip_vcs_ref);
    JSON_PARSE_KV_STRING("created_at", api_json_tokens, i, s,
                         pipeline->pip_created_at);
    JSON_PARSE_KV_STRING("updated_at", api_json_tokens, i, s,
                         pipeline->pip_updated_at);
    JSON_PARSE_KV_STRING("web_url", api_json_tokens, i, s, pipeline->pip_url);
    api_pipeline_json_status_parse(pipeline, &i, s);
  }

  if (e_pipeline) {
    api_pipeline_queue_fetch(api_cm,
                             e_pipeline->ent_e.ent_pipeline.pip_project_id,
                             e_pipeline->ent_e.ent_pipeline.pip_id, args);
    // Post-processing
    CHECK(e_pipeline->ent_kind, ==, EK_PIPELINE, "%d");
    struct tm time = {0};
    strptime(pipeline->pip_created_at, "%FT%T", &time);
    pipeline->pip_created_at_time = timegm(&time);
    strptime(pipeline->pip_updated_at, "%FT%T", &time);
    pipeline->pip_updated_at_time = timegm(&time);
    pipeline->pip_id_s = sdsfromlonglong(pipeline->pip_id);

    entity_shrink(e_pipeline);
    lstack_push(&args->arg_channel, e_pipeline);
  }
  entity_release(dummy_entity);
}

static void api_project_queue_fetch(CURLM *cm, entity_t *entity, args_t *args) {
  memset(api_url, 0, sizeof(api_url));

  if (args->arg_gitlab_token)
    snprintf(api_url, LEN0(api_url),
             "%s/api/v4/projects/%llu?simple=true&private_token=%s",
             args->arg_base_url, entity->ent_e.ent_project.pro_id,
             args->arg_gitlab_token);
  else
    snprintf(api_url, LEN0(api_url), "%s/api/v4/projects/%llu?simple=true",
             args->arg_base_url, entity->ent_e.ent_project.pro_id);

  entity->ent_api_url = sdsnew(api_url);
  CURL *eh = curl_easy_init();
  curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, api_curl_write_cb);
  curl_easy_setopt(eh, CURLOPT_WRITEDATA, entity);
  curl_easy_setopt(eh, CURLOPT_URL, entity->ent_api_url);
  curl_easy_setopt(eh, CURLOPT_PRIVATE, entity);
  curl_multi_add_handle(cm, eh);
}

static void api_pipelines_queue_fetch(CURLM *cm, entity_t *entity,
                                      args_t *args) {
  assert(entity->ent_kind == EK_FETCH_PIPELINES);
  memset(api_url, 0, sizeof(api_url));

  if (args->arg_gitlab_token)
    snprintf(api_url, LEN0(api_url),
             "%s/api/v4/projects/%llu/"
             "pipelines?private_token=%s&order_by=updated_at",
             args->arg_base_url, entity->ent_e.ent_pipeline.pip_project_id,
             args->arg_gitlab_token);
  else
    snprintf(api_url, LEN0(api_url),
             "%s/api/v4/projects/%llu/pipelines?order_by=updated_at",
             args->arg_base_url, entity->ent_e.ent_pipeline.pip_project_id);

  entity->ent_api_url = sdsnew(api_url);
  CURL *eh = curl_easy_init();
  curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, api_curl_write_cb);
  curl_easy_setopt(eh, CURLOPT_URL, entity->ent_api_url);
  curl_easy_setopt(eh, CURLOPT_WRITEDATA, entity);
  curl_easy_setopt(eh, CURLOPT_PRIVATE, entity);
  curl_multi_add_handle(cm, eh);
}

static void api_do_fetch(CURLM *cm) {
  int still_alive = 1;
  int msgs_left = -1;
  do {
    curl_multi_perform(cm, &still_alive);

    CURLMsg *msg;
    while ((msg = curl_multi_info_read(cm, &msgs_left))) {
      CURL *e = msg->easy_handle;

      entity_t *entity = NULL;
      curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &entity);

      if (msg->msg == CURLMSG_DONE) {
        i64 http_code = 0;
        curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &http_code);
        if (http_code != 200) {
          fprintf(stderr, "%s:%d:%lld url=%s data=%s ent_kind=%d\n", __FILE__,
                  __LINE__, http_code, entity->ent_api_url,
                  entity->ent_fetch_data, entity->ent_kind);
        }

        if (entity->ent_kind == EK_FETCH_PROJECT)
          api_project_parse_json(entity, &args.arg_channel);
        else if (entity->ent_kind == EK_FETCH_PIPELINES)
          api_pipelines_parse_json(entity, &args);
        else if (entity->ent_kind == EK_FETCH_PIPELINE)
          api_pipeline_parse_json(entity, &args.arg_channel);
        else
          assert(0 && "Unreachable");

        curl_multi_remove_handle(cm, e);
        curl_easy_cleanup(e);
      } else {
        fprintf(stderr, "%s:%d:Failed to fetch from API: err=%d\n", __FILE__,
                __LINE__, msg->msg);
        entity_release(entity);
      }
    }
    if (still_alive) curl_multi_wait(cm, NULL, 0, 100, NULL);

  } while (still_alive);
}

static void *api_fetch(void *v_args) {
  args_t *args = v_args;

  // Fetch projects
  for (u64 i = 0; i < buf_size(args->arg_project_ids); i++) {
    entity_t *entity = entity_new(EK_FETCH_PROJECT);
    project_t *project = &entity->ent_e.ent_project;
    project_init(project, args->arg_project_ids[i]);

    api_project_queue_fetch(api_cm, entity, args);
  }

  for (;;) {
    for (u64 i = 0; i < buf_size(args->arg_project_ids); i++) {
      entity_t *entity = entity_new(EK_FETCH_PIPELINES);
      pipeline_t *pipeline = &entity->ent_e.ent_pipeline;
      pipeline_init(pipeline, args->arg_project_ids[i]);
      api_pipelines_queue_fetch(api_cm, entity, args);
    }

    api_do_fetch(api_cm);
    sleep(5);
  }
  return NULL;
}
