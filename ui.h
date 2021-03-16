#pragma once

#include "common.h"
#include "deps/lstack/lstack.h"
#include "deps/termbox/src/termbox.c"
#include "deps/termbox/src/termbox.h"
#include "deps/termbox/src/utf8.c"

typedef struct {
  int tab_max_width_cols[5];
  int tab_y, tab_h, tab_selected;
  pipeline_t* tab_pipelines;
  project_t* tab_projects;
} table_t;

table_t table;

static void table_init() {
  table = (table_t){.tab_max_width_cols = {9, 9, 9, 9, 9},
                    .tab_h = tb_height() - 1};  // -1 for header
}

static void table_scroll_top(table_t* table) {
  table->tab_y = table->tab_selected = 0;
}

static void table_scroll_bottom(table_t* table) {
  table->tab_y = MAX(0, buf_size(table->tab_pipelines) - table->tab_h);
  table->tab_selected = buf_size(table->tab_pipelines) - 1;
}

static void table_resize(table_t* table) {
  table->tab_h = tb_height() - 1;
  table->tab_y = MAX(table->tab_y, table->tab_h);
}

static void table_add_or_update_pipeline(table_t* table, pipeline_t* pipeline) {
  for (int i = 0; i < (int)buf_size(table->tab_pipelines); i++) {
    if (table->tab_pipelines[i].pip_id == pipeline->pip_id) {
      /* pipeline_release(&table->tab_pipelines[i]); */
      if (sdslen(table->tab_pipelines[i].pip_project_path_with_namespace)) {
        pipeline->pip_project_path_with_namespace =
            table->tab_pipelines[i].pip_project_path_with_namespace;
      }
      table->tab_pipelines[i] = *pipeline;
      return;
    }
  }
  buf_push(table->tab_pipelines, *pipeline);
}

static void table_calc_size() {
  for (u64 i = 0; i < buf_size(table.tab_pipelines); i++) {
    pipeline_t* pipeline = &table.tab_pipelines[i];

    int col = 0;
    table.tab_max_width_cols[col] =
        MAX(table.tab_max_width_cols[col],
            (int)sdslen(pipeline->pip_project_path_with_namespace));
    col++;

    table.tab_max_width_cols[col] =
        MAX(table.tab_max_width_cols[col], (int)sdslen(pipeline->pip_vcs_ref));
    col++;

    col += 2;  // skip created, updated

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

static int ui_iso_date_to_short_time(const sds date, const struct tm* now,
                                     char res[10]) {
  struct tm time = {0};
  if (!strptime(date, "%FT%T", &time)) return sprintf(res, "?");

  struct tm diff = {.tm_year = now->tm_year - time.tm_year,
                    .tm_mon = now->tm_mon - time.tm_mon,
                    .tm_mday = now->tm_mday - time.tm_mday,
                    .tm_hour = now->tm_hour - time.tm_hour,
                    .tm_min = now->tm_min - time.tm_min,
                    .tm_sec = now->tm_sec - time.tm_sec};

  if (diff.tm_year > 100)
    return 0;
  else if (diff.tm_year > 0)
    return sprintf(res, "%dy", diff.tm_year);
  else if (diff.tm_mon > 0)
    return sprintf(res, "%dM", diff.tm_mon);
  else if (diff.tm_mday > 0)
    return sprintf(res, "%dd", diff.tm_mday);
  else if (diff.tm_hour > 0)
    return sprintf(res, "%dh", diff.tm_hour);
  else if (diff.tm_min > 0)
    return sprintf(res, "%dm", diff.tm_min);
  else if (diff.tm_sec > 0)
    return sprintf(res, "%ds", diff.tm_sec);
  else
    return sprintf(res, "now");
}

static void table_header_draw(table_t* table) {
  int col = 0, x = 0;
  // Project name
  {
    const char header[] = "PROJECT";
    ui_string_draw(header, LEN0(header), &x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(2 + table->tab_max_width_cols[col++] - LEN0(header), &x, 0,
                  TB_DEFAULT, TB_DEFAULT);
  }
  // Ref
  {
    const char header[] = "REF";
    ui_string_draw(header, LEN0(header), &x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(2 + table->tab_max_width_cols[col++] - LEN0(header), &x, 0,
                  TB_DEFAULT, TB_DEFAULT);
  }

  // Created
  {
    const char header[] = "CREATED";
    ui_string_draw(header, LEN0(header), &x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(2 + table->tab_max_width_cols[col++] - LEN0(header), &x, 0,
                  TB_DEFAULT, TB_DEFAULT);
  }

  // Status
  {
    const char header[] = "UPDATED";
    ui_string_draw(header, LEN0(header), &x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(2 + table->tab_max_width_cols[col++] - LEN0(header), &x, 0,
                  TB_DEFAULT, TB_DEFAULT);
  }

  // Status
  {
    const char header[] = "STATUS";
    ui_string_draw(header, LEN0(header), &x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(2 + table->tab_max_width_cols[col++] - LEN0(header), &x, 0,
                  TB_DEFAULT, TB_DEFAULT);
  }
  ui_blank_draw(tb_width() - x, &x, 0, TB_DEFAULT, TB_DEFAULT);
}

static void table_draw(table_t* table) {
  table_header_draw(table);

  time_t now_epoch = time(NULL);
  struct tm now;
  gmtime_r(&now_epoch, &now);

  for (int i = 0; i < table->tab_h; i++) {
    int p = i + table->tab_y, y = i + 1, x = 0, col = 0;

    if (p >= (int)buf_size(table->tab_pipelines)) return;
    const pipeline_t* const pipeline = &table->tab_pipelines[p];

    int fg = TB_BLUE, bg = TB_DEFAULT;
    if (table->tab_selected == p) {
      fg = TB_WHITE;
      bg = TB_BLUE;
    }

    ui_string_draw(pipeline->pip_project_path_with_namespace,
                   sdslen(pipeline->pip_project_path_with_namespace), &x, y, fg,
                   bg);
    ui_blank_draw(2 + table->tab_max_width_cols[col++] -
                      sdslen(pipeline->pip_project_path_with_namespace),
                  &x, y, fg, bg);

    ui_string_draw(pipeline->pip_vcs_ref, sdslen(pipeline->pip_vcs_ref), &x, y,
                   fg, bg);
    ui_blank_draw(
        2 + table->tab_max_width_cols[col++] - sdslen(pipeline->pip_vcs_ref),
        &x, y, fg, bg);

    char res[10] = "";
    int width = ui_iso_date_to_short_time(pipeline->pip_created_at, &now, res);
    ui_string_draw(res, width, &x, y, fg, bg);
    ui_blank_draw(2 + table->tab_max_width_cols[col++] - width, &x, y, fg, bg);

    memset(res, 0, LEN0(res));
    width = ui_iso_date_to_short_time(pipeline->pip_updated_at, &now, res);
    ui_string_draw(res, width, &x, y, fg, bg);
    ui_blank_draw(2 + table->tab_max_width_cols[col++] - width, &x, y, fg, bg);

    char status[40] = "";
    memcpy(status, pipeline->pip_status,
           MIN(LEN0(status), sdslen(pipeline->pip_status)));
    ui_string_draw(status, table->tab_max_width_cols[col++], &x, y, fg, bg);
    ui_blank_draw(2, &x, y, fg, bg);
  }
}

static void table_update_pipelines_on_project_received(project_t* project) {
  for (u64 i = 0; i < buf_size(table.tab_pipelines); i++) {
    pipeline_t* pipeline = &table.tab_pipelines[i];
    if (pipeline->pip_project_id == project->pro_id) {
      sdsfree(pipeline->pip_project_path_with_namespace);
      pipeline->pip_project_path_with_namespace =
          project->pro_path_with_namespace;
    }
  }
}

static void ui_run(args_t* args) {
  table_init();

  struct tb_event event;
  entity_t* entity = NULL;
  while (1) {
    tb_peek_event(&event, 500);
    while ((entity = lstack_pop(&args->arg_channel))) {
      if (entity->ent_kind == EK_PROJECT) {
        buf_push(table.tab_projects, entity->ent_e.ent_project);
        table_update_pipelines_on_project_received(
            &buf_last(table.tab_projects));
      } else if (entity->ent_kind == EK_PIPELINE) {
        table_add_or_update_pipeline(&table, &entity->ent_e.ent_pipeline);
      }
      entity_pop(entities, entity);
      sdsfree(entity->ent_fetch_data);
    }
    table_calc_size();

    switch (event.type) {
      case TB_EVENT_RESIZE:
        tb_clear();
        table_resize(&table);
        table_draw(&table);
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
          table_scroll_bottom(&table);
        } else if (event.ch == 'g') {
          table_scroll_top(&table);
        }
        tb_clear();
        table_draw(&table);
        tb_present();
        break;
      default:
        break;
    }
  }
}

