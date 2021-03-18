#pragma once

#include "common.h"
#include "deps/termbox/src/termbox.c"
#include "deps/termbox/src/termbox.h"
#include "deps/termbox/src/utf8.c"

#ifdef __APPLE__
#define OPEN_CMD "open"
#else
#define OPEN_CMD "xdg-open"
#endif

static const int ui_margin = 1;

typedef struct {
  int tab_max_width_cols[9];
  int tab_y, tab_h, tab_selected;
  pipeline_t* tab_pipelines;
  project_t* tab_projects;
} table_t;

table_t table;

static void table_init() {
  table = (table_t){.tab_max_width_cols = {9, 9, 9, 9, 9, 9, 9, 9, 9},
                    .tab_h = tb_height() - 1};  // -1 for header
  buf_grow(table.tab_projects, 100);
  buf_grow(table.tab_pipelines, 100);
}

static void table_scroll_top() { table.tab_y = table.tab_selected = 0; }

static int pipeline_compare(const void* a, const void* b) {
  const pipeline_t* const pa = a;
  const pipeline_t* const pb = b;

  return pb->pip_updated_at_time - pa->pip_updated_at_time;
}

static void table_sort() {
  qsort(table.tab_pipelines, buf_size(table.tab_pipelines), sizeof(pipeline_t),
        pipeline_compare);
}

static void table_scroll_bottom() {
  if (buf_size(table.tab_pipelines) == 0) return;

  table.tab_y = buf_size(table.tab_pipelines) - table.tab_h;
  CLAMP(table.tab_y, 0, (int)buf_size(table.tab_pipelines) - 1);

  table.tab_selected = (int)buf_size(table.tab_pipelines) - 1;
}

static void table_resize() {
  table.tab_h = tb_height() - 1;
  table.tab_y = table.tab_selected - table.tab_h - 1;
  CLAMP(table.tab_y, 0, (int)buf_size(table.tab_pipelines) - 1);
  CLAMP(table.tab_y, 0, table.tab_h - 1);
}

static void pipeline_merge(pipeline_t* before, pipeline_t* after) {
  if (sdslen(before->pip_project_path_with_namespace)) {
    after->pip_project_path_with_namespace =
        before->pip_project_path_with_namespace;
  }

  if (before->pip_started_at_time)
    after->pip_started_at_time = before->pip_started_at_time;

  if (sdslen(before->pip_started_at))
    after->pip_started_at = sdsdup(before->pip_started_at);

  if (before->pip_finished_at_time)
    after->pip_finished_at_time = before->pip_finished_at_time;

  if (sdslen(before->pip_finished_at))
    after->pip_finished_at = sdsdup(before->pip_finished_at);

  if (sdslen(before->pip_duration))
    after->pip_duration = sdsdup(before->pip_duration);

  if (before->pip_duration_second)
    after->pip_duration_second = before->pip_duration_second;
}

static void table_add_or_update_pipeline(pipeline_t* pipeline) {
  for (int i = 0; i < (int)buf_size(table.tab_pipelines); i++) {
    if (table.tab_pipelines[i].pip_id == pipeline->pip_id) {
      pipeline_merge(&table.tab_pipelines[i], pipeline);
      /* FIXME pipeline_release(&table.tab_pipelines[i]); */
      table.tab_pipelines[i] = *pipeline;
      return;
    }
  }
  buf_push(table.tab_pipelines, *pipeline);
}

static void table_calc_size() {
  for (u64 i = 0; i < buf_size(table.tab_pipelines); i++) {
    const pipeline_t* const pipeline = &table.tab_pipelines[i];

    int col = 0;
    table.tab_max_width_cols[col] =
        MAX(table.tab_max_width_cols[col], (int)sdslen(pipeline->pip_id_s));
    col++;

    table.tab_max_width_cols[col] =
        MAX(table.tab_max_width_cols[col],
            (int)sdslen(pipeline->pip_project_path_with_namespace));
    col++;

    table.tab_max_width_cols[col] =
        MAX(table.tab_max_width_cols[col], (int)sdslen(pipeline->pip_vcs_ref));
    col++;

    col += 3;  // skip created_at, updated_at, duration

    table.tab_max_width_cols[col] =
        MAX(table.tab_max_width_cols[col], (int)sdslen(pipeline->pip_status));
  }
}

static void ui_init() {
  tb_init();
  tb_set_cursor(TB_HIDE_CURSOR, TB_HIDE_CURSOR);
  tb_select_input_mode(TB_INPUT_ESC);
}

static void ui_string_draw(const char* s, int len, int* x, int y, u16 fg,
                           u16 bg) {
  for (int i = 0; i < len; i++) {
    tb_change_cell(*x, y, s[i], fg, bg);
    *x += 1;
  }
}

static void ui_blank_draw(int count, int* x, int y, u16 fg, u16 bg) {
  for (int i = 0; i < count; i++) {
    tb_change_cell(*x, y, 0, fg, bg);
    *x += 1;
  }
}

static void table_header_draw() {
  int col = 0, x = 0;
  {
    const char header[] = "ID";
    ui_string_draw(header, LEN0(header), &x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(ui_margin + table.tab_max_width_cols[col++] - LEN0(header),
                  &x, 0, TB_DEFAULT, TB_DEFAULT);
  }
  {
    const char header[] = "PROJECT";
    ui_string_draw(header, LEN0(header), &x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(ui_margin + table.tab_max_width_cols[col++] - LEN0(header),
                  &x, 0, TB_DEFAULT, TB_DEFAULT);
  }
  {
    const char header[] = "REF";
    ui_string_draw(header, LEN0(header), &x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(ui_margin + table.tab_max_width_cols[col++] - LEN0(header),
                  &x, 0, TB_DEFAULT, TB_DEFAULT);
  }
  {
    const char header[] = "CREATED";
    ui_string_draw(header, LEN0(header), &x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(ui_margin + table.tab_max_width_cols[col++] - LEN0(header),
                  &x, 0, TB_DEFAULT, TB_DEFAULT);
  }
  {
    const char header[] = "UPDATED";
    ui_string_draw(header, LEN0(header), &x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(ui_margin + table.tab_max_width_cols[col++] - LEN0(header),
                  &x, 0, TB_DEFAULT, TB_DEFAULT);
  }
  {
    const char header[] = "DURATION";
    ui_string_draw(header, LEN0(header), &x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(ui_margin + table.tab_max_width_cols[col++] - LEN0(header),
                  &x, 0, TB_DEFAULT, TB_DEFAULT);
  }
  {
    const char header[] = "STATUS";
    ui_string_draw(header, LEN0(header), &x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(ui_margin + table.tab_max_width_cols[col++] - LEN0(header),
                  &x, 0, TB_DEFAULT, TB_DEFAULT);
  }
  ui_blank_draw(tb_width() - x, &x, 0, TB_DEFAULT, TB_DEFAULT);
}

static void table_draw() {
  table_sort();
  table_header_draw();
  CHECK(table.tab_h, >, 0, "%d");
  CHECK(table.tab_y, >=, 0, "%d");
  CHECK(table.tab_selected, >=, 0, "%d");

  time_t now = time(NULL);

  for (int i = 0; i < table.tab_h; i++) {
    int p = i + table.tab_y, y = i + 1, x = 0, col = 0;

    CHECK(p, >=, 0, "%d");
    if (p >= (int)buf_size(table.tab_pipelines)) return;  // Finished
    const pipeline_t* const pipeline = &table.tab_pipelines[p];

    int fg = TB_BLUE, bg = TB_DEFAULT;
    if (table.tab_selected == p) {
      fg = TB_WHITE;
      bg = TB_BLUE;
    }

    {
      ui_string_draw(pipeline->pip_id_s, sdslen(pipeline->pip_id_s), &x, y, fg,
                     bg);
      ui_blank_draw(ui_margin + table.tab_max_width_cols[col++] -
                        sdslen(pipeline->pip_id_s),
                    &x, y, fg, bg);
    }
    {
      ui_string_draw(pipeline->pip_project_path_with_namespace,
                     sdslen(pipeline->pip_project_path_with_namespace), &x, y,
                     fg, bg);
      ui_blank_draw(ui_margin + table.tab_max_width_cols[col++] -
                        sdslen(pipeline->pip_project_path_with_namespace),
                    &x, y, fg, bg);
    }
    {
      ui_string_draw(pipeline->pip_vcs_ref, sdslen(pipeline->pip_vcs_ref), &x,
                     y, fg, bg);
      ui_blank_draw(ui_margin + table.tab_max_width_cols[col++] -
                        sdslen(pipeline->pip_vcs_ref),
                    &x, y, fg, bg);
    }
    char res[10] = "";
    int width = 0;
    {
      memset(res, 0, LEN0(res));
      u64 diff = difftime(now, pipeline->pip_created_at_time);
      width = common_duration_second_to_short(res, LEN0(res), diff);
      ui_string_draw(res, width, &x, y, fg, bg);
      ui_blank_draw(ui_margin + table.tab_max_width_cols[col++] - width, &x, y,
                    fg, bg);
    }
    {
      memset(res, 0, LEN0(res));
      u64 diff = difftime(now, pipeline->pip_updated_at_time);
      width = common_duration_second_to_short(res, LEN0(res), diff);
      ui_string_draw(res, width, &x, y, fg, bg);
      ui_blank_draw(ui_margin + table.tab_max_width_cols[col++] - width, &x, y,
                    fg, bg);
    }
    {
      ui_string_draw(pipeline->pip_duration, sdslen(pipeline->pip_duration), &x,
                     y, fg, bg);
      ui_blank_draw(ui_margin + table.tab_max_width_cols[col++] -
                        sdslen(pipeline->pip_duration),
                    &x, y, fg, bg);
    }
    {
      char status[40] = "";
      memcpy(status, pipeline->pip_status,
             MIN(LEN0(status), sdslen(pipeline->pip_status)));
      if (sdslen(pipeline->pip_status) == LEN0("success") &&
          strcmp(pipeline->pip_status, "success") == 0) {
        fg = TB_GREEN;
      } else if (sdslen(pipeline->pip_status) == LEN0("failed") &&
                 strcmp(pipeline->pip_status, "failed") == 0) {
        fg = TB_RED;
      } else if (sdslen(pipeline->pip_status) == LEN0("canceled") &&
                 strcmp(pipeline->pip_status, "canceled") == 0) {
        fg = TB_MAGENTA;
      }
      ui_string_draw(status, table.tab_max_width_cols[col++], &x, y, fg, bg);
      ui_blank_draw(ui_margin, &x, y, fg, bg);
    }
  }
}

static void table_update_pipelines_with_projects_info() {
  for (u64 i = 0; i < buf_size(table.tab_pipelines); i++) {
    pipeline_t* pipeline = &table.tab_pipelines[i];
    for (u64 j = 0; j < buf_size(table.tab_projects); j++) {
      project_t* project = &table.tab_projects[j];
      if (pipeline->pip_project_id == project->pro_id) {
        pipeline->pip_project_path_with_namespace =
            project->pro_path_with_namespace;
      }
    }
  }
}

static void table_pull_entities(args_t* args) {
  entity_t* entity = NULL;
  while ((entity = lstack_pop(&args->arg_channel))) {
    if (entity->ent_kind == EK_PROJECT) {
      buf_push(table.tab_projects, entity->ent_e.ent_project);
    } else if (entity->ent_kind == EK_PIPELINE) {
      table_add_or_update_pipeline(&entity->ent_e.ent_pipeline);
    } else
      assert(0 && "Unreachable");

    sdsfree(entity->ent_fetch_data);
  }
  table_update_pipelines_with_projects_info();
  table_calc_size();
}

static void ui_run(args_t* args) {
  table_init();

  while (1) {
    struct tb_event event = {0};
    tb_peek_event(&event, 500);
    table_pull_entities(args);

    switch (event.type) {
      case TB_EVENT_RESIZE:
        tb_clear();
        table_resize();
        table_draw();
        tb_present();
        break;

      case TB_EVENT_KEY:
        if (event.key == TB_KEY_ARROW_DOWN) {
          if (table.tab_selected < (int)buf_size(table.tab_pipelines) - 1)
            table.tab_selected++;
          if (table.tab_selected == (table.tab_y + table.tab_h) &&
              table.tab_y + table.tab_h < (int)buf_size(table.tab_pipelines))
            table.tab_y++;
        } else if (event.key == TB_KEY_ARROW_UP) {
          if (table.tab_selected > 0) table.tab_selected--;
          if (table.tab_selected == table.tab_y && table.tab_y > 0)
            table.tab_y--;
        } else if (event.key == TB_KEY_ESC || event.key == TB_KEY_CTRL_C ||
                   event.key == TB_KEY_CTRL_D) {
          tb_shutdown();
          exit(0);
        } else if (event.ch == 'G') {
          table_scroll_bottom();
        } else if (event.ch == 'g') {
          table_scroll_top();
        } else if ((event.ch == 'o' || event.ch == 'O') &&
                   sdslen(table.tab_pipelines[table.tab_selected].pip_url)) {
          sds cmd = sdscatfmt(sdsempty(), OPEN_CMD " %s",
                              table.tab_pipelines[table.tab_selected].pip_url);
          system(cmd);
          sdsfree(cmd);
        }
        tb_clear();
        table_resize();
        table_draw();
        tb_present();
        break;
      default:
        tb_clear();
        table_resize();
        table_draw();
        tb_present();
        break;
    }
  }
}

