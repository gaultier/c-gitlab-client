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

static void ui_pipelines_draw() {
  int y = 0, x = 0;
  const char header[] = "Id                        Status        Url";
  ui_string_draw(header, sizeof(header) - 1, &x, y, TB_WHITE | TB_BOLD,
                 TB_YELLOW);
  for (int k = sizeof(header) - 1; k <= tb_width(); k++)
    tb_change_cell(x++, 0, 0, TB_DEFAULT, TB_YELLOW);
  y++;

  for (u64 i = 0; i < buf_size(projects); i++) {
    const project_t* const project = &projects[i];

    for (u64 j = 0; j < buf_size(project->pro_pipelines); j++) {
      const pipeline_t* const pipeline = &project->pro_pipelines[j];
      x = 0;

      char id[27] = "";
      snprintf(id, sizeof(id) - 1, "%lld", pipeline->pip_id);
      ui_string_draw(id, sizeof(id) - 1, &x, y, TB_RED, TB_DEFAULT);

      char status[15] = "";
      memcpy(status, pipeline->pip_status,
             MIN(sizeof(status) - 1, sdslen(pipeline->pip_status)));
      ui_string_draw(status, sizeof(status) - 1, &x, y, TB_RED, TB_DEFAULT);

      char url[40] = "";
      memcpy(url, pipeline->pip_url,
             MIN(sizeof(url) - 1, sdslen(pipeline->pip_url)));
      ui_string_draw(url, sizeof(url) - 1, &x, y, TB_RED, TB_DEFAULT);
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

