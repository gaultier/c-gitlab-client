#include "deps/termbox/src/termbox.c"
#include "deps/termbox/src/termbox.h"
#include "deps/termbox/src/utf8.c"

static void ui_init() {
  tb_init();
  tb_set_cursor(TB_HIDE_CURSOR, TB_HIDE_CURSOR);
  tb_select_input_mode(TB_INPUT_ESC);
}

static void ui_pipelines_draw() {
  u64 y = 1, x = 0;
  const sds header = sdsnew("Id │ Status │ Url");
  for (u64 k = 0; k < sdslen(header); k++)
    tb_change_cell(x++, 0, header[k], TB_WHITE, TB_DEFAULT);

  for (u64 i = 0; i < buf_size(projects); i++) {
    const project_t* const project = &projects[i];

    for (u64 j = 0; j < buf_size(project->pro_pipelines); j++) {
      const pipeline_t* const pipeline = &project->pro_pipelines[j];
      x = 0;

      const sds row =
          sdscatprintf(sdsempty(), "%lld │ %s │ %s", pipeline->pip_id,
                       pipeline->pip_status, pipeline->pip_url);
      for (u64 k = 0; k < sdslen(row); k++)
        tb_change_cell(x++, y, row[k], TB_RED, TB_DEFAULT);

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

