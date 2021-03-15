#pragma once

#include "common.h"
#include "deps/lstack/lstack.c"
#include "deps/lstack/lstack.h"

static jsmntok_t *json_tokens;
static char url[4097];
static CURLM *cm = NULL;

static void api_init() { cm = curl_multi_init(); }

static int json_eq(const char *json, const jsmntok_t *tok, const char *s,
                   u64 s_len) {
  if (tok->type == JSMN_STRING && ((int)s_len == tok->end - tok->start) &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}

static void project_parse_json(project_t *project) {
  buf_grow(json_tokens, 10 * 1024);

  jsmn_parser parser;
  jsmn_init(&parser);

  const char *const s = project->pro_api_data;
  int res = jsmn_parse(&parser, s, sdslen((char *)s), json_tokens,
                       sdslen(project->pro_api_data));
  if (res <= 0 || json_tokens[0].type != JSMN_OBJECT) {
    fprintf(stderr, "%s:%d:Malformed JSON for project: id=%lld\n", __FILE__,
            __LINE__, project->pro_id);
    return;
  }

  for (i64 i = 1; i < res; i++) {
    jsmntok_t *const tok = &json_tokens[i];
    if (tok->type != JSMN_STRING) continue;

    if (json_eq(s, tok, "name", LEN0("name")) == 0) {
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
}

static void project_parse_pipelines_json(project_t *project,
                                         pipeline_t **pipelines) {
  assert(pipelines);
  buf_clear(json_tokens);

  jsmn_parser parser;
  jsmn_init(&parser);

  const char *const s = project->pro_api_data;
  int res = jsmn_parse(&parser, s, sdslen((char *)s), json_tokens,
                       sdslen(project->pro_api_data));
  if (res <= 0 || json_tokens[0].type != JSMN_ARRAY) {
    fprintf(stderr,
            "%s:%d:Malformed JSON for project pipelines: id=%lld res=%d "
            "json=`%s`\n",
            __FILE__, __LINE__, project->pro_id, res, project->pro_api_data);
    return;
  }

  pipeline_t *pipeline = NULL;
  for (i64 i = 1; i < res; i++) {
    const jsmntok_t *const tok = &json_tokens[i];
    if (tok->type == JSMN_OBJECT) {
      buf_push(*pipelines, ((pipeline_t){0}));
      assert(*pipelines);
      pipeline = &((*pipelines)[buf_size(*pipelines) - 1]);
      pipeline_init(pipeline, project->pro_path_with_namespace);
      continue;
    }

    if (json_eq(project->pro_api_data, tok, "id", LEN0("id")) == 0) {
      const jsmntok_t *const t = &json_tokens[++i];
      const char *const value = project->pro_api_data + t->start;
      if (t->type != JSMN_PRIMITIVE) {
        fprintf(
            stderr,
            "%s:%d:Malformed JSON for project id: id=%lld res=%d json=`%s`\n",
            __FILE__, __LINE__, project->pro_id, res, project->pro_api_data);
        return;
      }

      pipeline->pip_id = strtoll(value, NULL, 10);
    }
    if (json_eq(project->pro_api_data, tok, "ref", LEN0("ref")) == 0) {
      const jsmntok_t *const t = &json_tokens[++i];
      const char *const value = project->pro_api_data + t->start;
      pipeline->pip_vcs_ref =
          sdscatlen(pipeline->pip_vcs_ref, value, t->end - t->start);
    }
    if (json_eq(project->pro_api_data, tok, "created_at", LEN0("created_at")) ==
        0) {
      const jsmntok_t *const t = &json_tokens[++i];
      const char *const value = project->pro_api_data + t->start;
      pipeline->pip_created_at =
          sdscatlen(pipeline->pip_created_at, value, t->end - t->start);
    }
    if (json_eq(project->pro_api_data, tok, "updated_at", LEN0("updated_at")) ==
        0) {
      const jsmntok_t *const t = &json_tokens[++i];
      const char *const value = project->pro_api_data + t->start;
      pipeline->pip_updated_at =
          sdscatlen(pipeline->pip_updated_at, value, t->end - t->start);
    }
    if (json_eq(project->pro_api_data, tok, "status", LEN0("status")) == 0) {
      const jsmntok_t *const t = &json_tokens[++i];
      const char *const value = project->pro_api_data + t->start;
      pipeline->pip_status =
          sdscatlen(pipeline->pip_status, value, t->end - t->start);
    }
    if (json_eq(project->pro_api_data, tok, "web_url", LEN0("web_url")) == 0) {
      const jsmntok_t *const t = &json_tokens[++i];
      const char *const value = project->pro_api_data + t->start;
      pipeline->pip_url =
          sdscatlen(pipeline->pip_url, value, t->end - t->start);
    }
  }
}

static size_t write_cb(char *data, size_t n, size_t l, void *userp) {
  i64 i = (i64)userp;
  project_t *project = &args.arg_projects[i];
  project->pro_api_data = sdscatlen(project->pro_api_data, data, n * l);

  return n * l;
}

static void project_fetch_queue(CURLM *cm, int i, args_t *args) {
  memset(url, 0, sizeof(url));

  if (args->arg_gitlab_token)
    snprintf(
        url, LEN0(url), "%s/api/v4/projects/%llu?simple=true&private_token=%s",
        args->arg_base_url, args->arg_project_ids[i], args->arg_gitlab_token);
  else
    snprintf(url, LEN0(url), "%s/api/v4/projects/%llu", args->arg_base_url,
             args->arg_project_ids[i]);

  CURL *eh = curl_easy_init();
  curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_cb);
  curl_easy_setopt(eh, CURLOPT_WRITEDATA, i);
  curl_easy_setopt(eh, CURLOPT_URL, url);
  curl_easy_setopt(eh, CURLOPT_PRIVATE, i);
  curl_multi_add_handle(cm, eh);
}

static void project_pipelines_fetch_queue(CURLM *cm, int i, args_t *args) {
  memset(url, 0, sizeof(url));

  if (args->arg_gitlab_token)
    snprintf(
        url, LEN0(url), "%s/api/v4/projects/%llu/pipelines?private_token=%s",
        args->arg_base_url, args->arg_project_ids[i], args->arg_gitlab_token);
  else
    snprintf(url, LEN0(url), "%s/api/v4/projects/%llu/pipelines",
             args->arg_base_url, args->arg_project_ids[i]);
  CURL *eh = curl_easy_init();
  curl_easy_setopt(eh, CURLOPT_WRITEFUNCTION, write_cb);
  curl_easy_setopt(eh, CURLOPT_URL, url);
  curl_easy_setopt(eh, CURLOPT_WRITEDATA, i);
  curl_easy_setopt(eh, CURLOPT_PRIVATE, i);
  curl_multi_add_handle(cm, eh);
}

static void api_fetch(CURLM *cm, args_t *args) {
  int still_alive = 1;
  int msgs_left = -1;
  do {
    curl_multi_perform(cm, &still_alive);

    CURLMsg *msg;
    while ((msg = curl_multi_info_read(cm, &msgs_left))) {
      CURL *e = msg->easy_handle;
      i64 project_i = 0;
      curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &project_i);

      if (msg->msg == CURLMSG_DONE) {
        // fprintf(stderr, "R: %d - %s\n", msg->data.result,
        //        curl_easy_strerror(msg->data.result));
        curl_multi_remove_handle(cm, e);
        curl_easy_cleanup(e);
      } else {
        project_t *project = &args->arg_projects[project_i];
        fprintf(stderr, "Failed to fetch from API: id=%lld err=%d\n",
                project->pro_id, msg->msg);
      }
    }
    if (still_alive) curl_multi_wait(cm, NULL, 0, 100, NULL);

  } while (still_alive);
}

static void projects_fetch(args_t *args) {
  for (u64 i = 0; i < buf_size(args->arg_project_ids); i++) {
    const i64 id = args->arg_project_ids[i];

    project_t project = {0};
    project_init(&project, id);
    buf_push(args->arg_projects, project);
    project_fetch_queue(cm, i, args);
  }
  api_fetch(cm, args);

  for (u64 i = 0; i < buf_size(args->arg_project_ids); i++) {
    project_t *project = &args->arg_projects[i];
    project_parse_json(project);
  }
}

static pipeline_t *pipelines_fetch(args_t *args) {
  for (u64 i = 0; i < buf_size(args->arg_projects); i++) {
    sdsclear(args->arg_projects[i].pro_api_data);
    project_pipelines_fetch_queue(cm, i, args);
  }

  api_fetch(cm, args);

  pipeline_t *pipelines = NULL;
  buf_grow(pipelines, 100);
  for (u64 i = 0; i < buf_size(args->arg_projects); i++) {
    project_t *project = &args->arg_projects[i];
    project_parse_pipelines_json(project, &pipelines);
  }
  return pipelines;
}

static void *fetch(void *v_args) {
  args_t *args = v_args;
  curl_global_init(CURL_GLOBAL_ALL);

  for (;;) {
    buf_trunc(json_tokens, 10 * 1024);  // 10 KiB
    pipeline_t *pipelines = pipelines_fetch(args);
    lstack_push(&args->arg_channel, pipelines);
    sleep(5);
  }
  return NULL;
}
