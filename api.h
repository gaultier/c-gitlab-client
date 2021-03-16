#pragma once

#include "common.h"

static jsmntok_t *json_tokens;
static char url[4097];
static CURLM *cm = NULL;
static entity_t *entities = NULL;

static void api_init() { cm = curl_multi_init(); }

static int json_eq(const char *json, const jsmntok_t *tok, const char *s,
                   u64 s_len) {
  if (tok->type == JSMN_STRING && ((int)s_len == tok->end - tok->start) &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

static void project_parse_json(entity_t *entity, lstack_t *channel) {
  buf_trunc(json_tokens, 10 * 1024);  // 10 KiB
  buf_clear(json_tokens);

  jsmn_parser parser;
  jsmn_init(&parser);

  project_t *project = &entity->ent_e.ent_project;

  const char *const s = entity->ent_fetch_data;
  int res = jsmn_parse(&parser, s, sdslen((char *)s), json_tokens,
                       buf_capacity(json_tokens));
  if (res <= 0 || json_tokens[0].type != JSMN_OBJECT) {
    fprintf(stderr, "%s:%d:Malformed JSON for project\n", __FILE__, __LINE__);
    return;
  }

  for (i64 i = 1; i < res; i++) {
    jsmntok_t *const tok = &json_tokens[i];
    if (tok->type != JSMN_STRING)
      continue;

    else if (json_eq(s, tok, "name", LEN0("name")) == 0) {
      project->pro_name =
          sdscatlen(project->pro_name, s + json_tokens[i + 1].start,
                    json_tokens[i + 1].end - json_tokens[i + 1].start);
      i++;
    } else if (json_eq(s, tok, "path_with_namespace",
                       LEN0("path_with_namespace")) == 0) {
      project->pro_path_with_namespace = sdscatlen(
          project->pro_path_with_namespace, s + json_tokens[i + 1].start,
          json_tokens[i + 1].end - json_tokens[i + 1].start);
      i++;
    }
  }
  lstack_push(channel, entity);
}

static void pipelines_parse_json(entity_t *dummy_entity, lstack_t *channel) {
  buf_trunc(json_tokens, 10 * 1024);  // 10 KiB
  buf_clear(json_tokens);

  jsmn_parser parser;
  jsmn_init(&parser);

  const char *const s = dummy_entity->ent_fetch_data;
  int res = jsmn_parse(&parser, s, sdslen((char *)s), json_tokens,
                       buf_capacity(json_tokens));
  if (res <= 0 || json_tokens[0].type != JSMN_ARRAY) {
    fprintf(stderr,
            "%s:%d:Malformed JSON for project pipelines: "
            "json=`%s`\n",
            __FILE__, __LINE__, dummy_entity->ent_fetch_data);
    return;
  }

  entity_t *e_pipeline = NULL;
  pipeline_t *pipeline = NULL;
  for (i64 i = 1; i < res; i++) {
    const jsmntok_t *const tok = &json_tokens[i];
    if (tok->type == JSMN_OBJECT) {
      if (e_pipeline) {
        lstack_push(channel, e_pipeline);
      }
      e_pipeline = entity_new(EK_PIPELINE);
      pipeline = &e_pipeline->ent_e.ent_pipeline;
      pipeline_init(pipeline, dummy_entity->ent_e.ent_pipeline.pip_project_id);
      continue;
    }

    if (json_eq(s, tok, "id", LEN0("id")) == 0) {
      const jsmntok_t *const t = &json_tokens[++i];
      const char *const value = s + t->start;
      if (t->type != JSMN_PRIMITIVE) {
        fprintf(stderr, "%s:%d:Malformed JSON for project: res=%d json=`%s`\n",
                __FILE__, __LINE__, res, s);
        return;
      }

      pipeline->pip_id = strtoll(value, NULL, 10);
    }
    if (json_eq(s, tok, "ref", LEN0("ref")) == 0) {
      const jsmntok_t *const t = &json_tokens[++i];
      const char *const value = s + t->start;
      pipeline->pip_vcs_ref =
          sdscatlen(pipeline->pip_vcs_ref, value, t->end - t->start);
    }
    if (json_eq(s, tok, "created_at", LEN0("created_at")) == 0) {
      const jsmntok_t *const t = &json_tokens[++i];
      const char *const value = s + t->start;
      pipeline->pip_created_at =
          sdscatlen(pipeline->pip_created_at, value, t->end - t->start);
    }
    if (json_eq(s, tok, "updated_at", LEN0("updated_at")) == 0) {
      const jsmntok_t *const t = &json_tokens[++i];
      const char *const value = s + t->start;
      pipeline->pip_updated_at =
          sdscatlen(pipeline->pip_updated_at, value, t->end - t->start);
    }
    if (json_eq(s, tok, "status", LEN0("status")) == 0) {
      const jsmntok_t *const t = &json_tokens[++i];
      const char *const value = s + t->start;
      pipeline->pip_status =
          sdscatlen(pipeline->pip_status, value, t->end - t->start);
    }
    if (json_eq(s, tok, "web_url", LEN0("web_url")) == 0) {
      const jsmntok_t *const t = &json_tokens[++i];
      const char *const value = s + t->start;
      pipeline->pip_url =
          sdscatlen(pipeline->pip_url, value, t->end - t->start);
    }
  }
  entity_pop(entities, dummy_entity);
  entity_release(dummy_entity);
}

static size_t curl_write_cb(char *data, size_t n, size_t l, void *userp) {
  entity_t *entity = userp;
  entity->ent_fetch_data = sdscatlen(entity->ent_fetch_data, data, n * l);

  return n * l;
}

static void project_queue_fetch(CURLM *cm, entity_t *entity, args_t *args) {
  memset(url, 0, sizeof(url));

  if (args->arg_gitlab_token)
    snprintf(url, LEN0(url),
             "%s/api/v4/projects/%llu?simple=true&private_token=%s",
             args->arg_base_url, entity->ent_e.ent_project.pro_id,
             args->arg_gitlab_token);
  else
    snprintf(url, LEN0(url), "%s/api/v4/projects/%llu?simple=true",
             args->arg_base_url, entity->ent_e.ent_project.pro_id);

  CURL *eh = curl_easy_init();
  curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, curl_write_cb);
  curl_easy_setopt(eh, CURLOPT_WRITEDATA, entity);
  curl_easy_setopt(eh, CURLOPT_URL, url);
  curl_easy_setopt(eh, CURLOPT_PRIVATE, entity);
  curl_multi_add_handle(cm, eh);
}

static void pipelines_queue_fetch(CURLM *cm, entity_t *entity, args_t *args) {
  memset(url, 0, sizeof(url));

  if (args->arg_gitlab_token)
    snprintf(url, LEN0(url),
             "%s/api/v4/projects/%llu/pipelines?private_token=%s",
             args->arg_base_url, entity->ent_e.ent_pipeline.pip_project_id,
             args->arg_gitlab_token);
  else
    snprintf(url, LEN0(url), "%s/api/v4/projects/%llu/pipelines",
             args->arg_base_url, entity->ent_e.ent_pipeline.pip_project_id);
  CURL *eh = curl_easy_init();
  curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, curl_write_cb);
  curl_easy_setopt(eh, CURLOPT_URL, url);
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
        // fprintf(stderr, "R: %d - %s\n", msg->data.result,
        //        curl_easy_strerror(msg->data.result));

        if (msg->data.result == CURLE_OK) {
          if (entity->ent_kind == EK_PROJECT)
            project_parse_json(entity, &args.arg_channel);
          else if (entity->ent_kind == EK_PIPELINE)
            pipelines_parse_json(entity, &args.arg_channel);
        }
        curl_multi_remove_handle(cm, e);
        curl_easy_cleanup(e);
      } else {
        fprintf(stderr, "Failed to fetch from API: err=%d\n", msg->msg);
        // TODO: entity_pop?
      }
    }
    if (still_alive) curl_multi_wait(cm, NULL, 0, 100, NULL);

  } while (still_alive);
}

static void *fetch(void *v_args) {
  args_t *args = v_args;
  curl_global_init(CURL_GLOBAL_ALL);

  // Fetch projects
  for (u64 i = 0; i < buf_size(args->arg_project_ids); i++) {
    entity_t *entity = entity_new(EK_PROJECT);
    project_t *project = &entity->ent_e.ent_project;
    project_init(project, args->arg_project_ids[i]);
    entity_push(entities, entity);

    project_queue_fetch(cm, entity, args);
  }

  for (;;) {
    for (u64 i = 0; i < buf_size(args->arg_project_ids); i++) {
      entity_t *entity = entity_new(EK_PIPELINE);
      pipeline_t *pipeline = &entity->ent_e.ent_pipeline;
      pipeline_init(pipeline, args->arg_project_ids[i]);
      entity_push(entities, entity);
      pipelines_queue_fetch(cm, entity, args);
    }

    api_do_fetch(cm);
    sleep(5);
  }
  return NULL;
}
