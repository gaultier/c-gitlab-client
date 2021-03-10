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

static void ui_pipelines_header_draw(int* x, int max_width_id,
                                     int max_width_status, int max_width_url) {
  // Id
  {
    const char header[] = "Id";
    ui_string_draw(header, LEN0(header), x, 0, TB_WHITE | TB_BOLD, TB_YELLOW);
    ui_blank_draw(1 + max_width_id - LEN0(header), x, 0, TB_DEFAULT, TB_YELLOW);
  }

  // Status
  {
    const char header[] = "Status";
    ui_string_draw(header, LEN0(header), x, 0, TB_WHITE | TB_BOLD, TB_YELLOW);
    ui_blank_draw(1 + max_width_status - LEN0(header), x, 0, TB_DEFAULT,
                  TB_YELLOW);
  }

  // URL
  {
    const char header[] = "Url";
    ui_string_draw(header, LEN0(header), x, 0, TB_WHITE | TB_BOLD, TB_YELLOW);
    ui_blank_draw(max_width_url - LEN0(header), x, 0, TB_DEFAULT, TB_YELLOW);
  }

  ui_blank_draw(tb_width() - *x, x, 0, TB_DEFAULT, TB_YELLOW);
}

static void ui_pipelines_draw() {
  int max_width_id = 0, max_width_status = 0, max_width_url = 0;
  {
    for (u64 i = 0; i < buf_size(projects); i++) {
      const project_t* const project = &projects[i];

      for (u64 j = 0; j < buf_size(project->pro_pipelines); j++) {
        const pipeline_t* const pipeline = &project->pro_pipelines[j];

        char id[27] = "";
        const int width = sprintf(id, "%lld", pipeline->pip_id);
        max_width_id = MAX(max_width_id, width);

        max_width_status = MAX(max_width_status, sdslen(pipeline->pip_status));
        max_width_url = MAX(max_width_url, sdslen(pipeline->pip_url));
      }
    }
  }

  int y = 0, x = 0;
  ui_pipelines_header_draw(&x, max_width_id, max_width_status, max_width_url);
  y++;

  for (u64 i = 0; i < buf_size(projects); i++) {
    const project_t* const project = &projects[i];

    for (u64 j = 0; j < buf_size(project->pro_pipelines); j++) {
      const pipeline_t* const pipeline = &project->pro_pipelines[j];
      x = 0;

      char id[27] = "";
      snprintf(id, LEN0(id), "%lld", pipeline->pip_id);
      ui_string_draw(id, max_width_id, &x, y, TB_RED, TB_DEFAULT);
      ui_blank_draw(1, &x, y, TB_DEFAULT, TB_DEFAULT);

      char status[40] = "";
      memcpy(status, pipeline->pip_status,
             MIN(LEN0(status), sdslen(pipeline->pip_status)));
      ui_string_draw(status, max_width_status, &x, y, TB_RED, TB_DEFAULT);
      ui_blank_draw(1, &x, y, TB_DEFAULT, TB_DEFAULT);

      char url[500] = "";
      memcpy(url, pipeline->pip_url, MIN(LEN0(url), sdslen(pipeline->pip_url)));
      ui_string_draw(url, max_width_url, &x, y, TB_RED, TB_DEFAULT);
      y++;
    }
  }
}

static void ui_draw() {
  tb_clear();
  ui_pipelines_draw();
  tb_present();

  struct tb_event event;
  while (tb_poll_event(&event)) {
    switch (event.type) {
      case TB_EVENT_RESIZE:
        tb_clear();
        ui_pipelines_draw();
        tb_present();
        break;

      case TB_EVENT_KEY:
        tb_shutdown();
        return;
      default:
        break;
    }
  }
}

