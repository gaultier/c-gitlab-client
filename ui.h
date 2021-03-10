#include "deps/termbox/src/termbox.c"
#include "deps/termbox/src/termbox.h"
#include "deps/termbox/src/utf8.c"

static void ui_init() {
  tb_init();
  tb_set_cursor(TB_HIDE_CURSOR, TB_HIDE_CURSOR);
  tb_select_input_mode(TB_INPUT_ESC);
}

static void ui_string_draw(const char* s, u64 len, int* x, int y, u16 fg,
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

static void ui_pipelines_header_draw(int* x, int max_width_ref,
                                     int max_width_status,
                                     int max_width_created,
                                     int max_width_updated) {
  // REF
  {
    const char header[] = "REF";
    ui_string_draw(header, LEN0(header), x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(2 + max_width_ref - LEN0(header), x, 0, TB_DEFAULT,
                  TB_DEFAULT);
  }

  // Created
  {
    const char header[] = "CREATED";
    ui_string_draw(header, LEN0(header), x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(2 + max_width_created - LEN0(header), x, 0, TB_DEFAULT,
                  TB_DEFAULT);
  }

  // Status
  {
    const char header[] = "UPDATED";
    ui_string_draw(header, LEN0(header), x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(2 + max_width_updated - LEN0(header), x, 0, TB_DEFAULT,
                  TB_DEFAULT);
  }

  // Status
  {
    const char header[] = "STATUS";
    ui_string_draw(header, LEN0(header), x, 0, TB_WHITE | TB_BOLD, TB_DEFAULT);
    ui_blank_draw(2 + max_width_status - LEN0(header), x, 0, TB_DEFAULT,
                  TB_DEFAULT);
  }
  ui_blank_draw(tb_width() - *x, x, 0, TB_DEFAULT, TB_DEFAULT);
}

static int ui_iso_date_to_short_time(const sds date, const struct tm* now,
                                     char res[10]) {
  struct tm time = {0};
  if (!strptime(date, "%FT%T", &time)) return sprintf(res, "?");

  struct tm diff = {.tm_year = now->tm_year - time.tm_year,
                    .tm_mon = now->tm_mon - time.tm_mon,
                    .tm_mday = now->tm_mday - time.tm_mday,
                    .tm_hour = now->tm_hour - time.tm_hour,
                    .tm_min = now->tm_hour - time.tm_hour,
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

static void ui_pipelines_draw(int* pipelines_count, int* pipeline_selected) {
  int max_width_ref = 0, max_width_status = 0, max_width_created = 9,
      max_width_updated = 9;
  {
    for (u64 i = 0; i < buf_size(projects); i++) {
      const project_t* const project = &projects[i];

      for (u64 j = 0; j < buf_size(project->pro_pipelines); j++) {
        const pipeline_t* const pipeline = &project->pro_pipelines[j];

        max_width_ref = MAX(max_width_ref, sdslen(pipeline->pip_vcs_ref));
        max_width_status = MAX(max_width_status, sdslen(pipeline->pip_status));

        *pipelines_count += 1;
      }
    }
  }

  int y = 0, x = 0, k = 0;
  ui_pipelines_header_draw(&x, max_width_ref, max_width_status,
                           max_width_created, max_width_updated);
  y++;

  time_t now_epoch = time(NULL);
  struct tm now;
  gmtime_r(&now_epoch, &now);

  for (u64 i = 0; i < buf_size(projects); i++) {
    const project_t* const project = &projects[i];

    for (u64 j = 0; j < buf_size(project->pro_pipelines); j++) {
      const pipeline_t* const pipeline = &project->pro_pipelines[j];
      x = 0;

      int fg = TB_BLUE, bg = TB_DEFAULT;
      if (*pipeline_selected == k) {
        fg = TB_WHITE;
        bg = TB_BLUE;
      }

      ui_string_draw(pipeline->pip_vcs_ref, sdslen(pipeline->pip_vcs_ref), &x,
                     y, fg, bg);
      ui_blank_draw(2 + max_width_ref - sdslen(pipeline->pip_vcs_ref), &x, y,
                    fg, bg);

      char res[10] = "";
      int width =
          ui_iso_date_to_short_time(pipeline->pip_created_at, &now, res);
      ui_string_draw(res, width, &x, y, fg, bg);
      ui_blank_draw(2 + LEN0(res) - width, &x, y, fg, bg);

      memset(res, 0, LEN0(res));
      width = ui_iso_date_to_short_time(pipeline->pip_updated_at, &now, res);
      ui_string_draw(res, width, &x, y, fg, bg);
      ui_blank_draw(2 + LEN0(res) - width, &x, y, fg, bg);

      char status[40] = "";
      memcpy(status, pipeline->pip_status,
             MIN(LEN0(status), sdslen(pipeline->pip_status)));
      ui_string_draw(status, max_width_status, &x, y, fg, bg);
      ui_blank_draw(2, &x, y, fg, bg);

      y++;
      k++;
    }
  }
}

static void ui_draw() {
  int pipelines_count = 0, pipeline_selected = 0;
  tb_clear();
  ui_pipelines_draw(&pipelines_count, &pipeline_selected);
  tb_present();

  struct tb_event event;
  while (tb_poll_event(&event)) {
    switch (event.type) {
      case TB_EVENT_RESIZE:
        tb_clear();
        ui_pipelines_draw(&pipelines_count, &pipeline_selected);
        tb_present();
        break;

      case TB_EVENT_KEY:
        if (event.key == TB_KEY_ARROW_DOWN &&
            pipeline_selected < pipelines_count) {
          pipeline_selected++;
          tb_clear();
          ui_pipelines_draw(&pipelines_count, &pipeline_selected);
          tb_present();
        } else if (event.key == TB_KEY_ARROW_UP && pipeline_selected > 0) {
          pipeline_selected--;
          tb_clear();
          ui_pipelines_draw(&pipelines_count, &pipeline_selected);
          tb_present();
        } else if (event.key == TB_KEY_ESC || event.key == TB_KEY_CTRL_C ||
                   event.key == TB_KEY_CTRL_D) {
          tb_shutdown();
          return;
        }
        break;
      default:
        break;
    }
  }
}

