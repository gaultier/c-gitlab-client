#include <menu.h>

MENU* menu;
WINDOW* window;
ITEM** items;

static void ui_init() {
  initscr();
  start_color();
  cbreak();
  noecho();
  keypad(stdscr, TRUE);
  init_pair(1, COLOR_RED, COLOR_BLACK);
}

static void ui_print_in_middle(WINDOW* win, int starty, int startx, int width,

                               char* string, chtype color) {
  int length, x, y;
  float temp;

  if (win == NULL) win = stdscr;
  getyx(win, y, x);
  if (startx != 0) x = startx;
  if (starty != 0) y = starty;
  if (width == 0) width = 80;

  length = strlen(string);
  temp = (width - length) / 2;
  x = startx + (int)temp;
  wattron(win, color);
  mvwprintw(win, y, x, "%s", string);
  wattroff(win, color);
  refresh();
}

static void ui_draw() {
  u64 pipeline_count = 0;
  for (u64 i = 0; i < buf_size(projects); i++) {
    const project_t* const project = &projects[i];

    for (u64 j = 0; j < buf_size(project->pro_pipelines); j++) {
      pipeline_count++;
    }
  }
  items = calloc(pipeline_count, sizeof(void*));
  u64 k = 0;
  for (u64 i = 0; i < buf_size(projects); i++) {
    const project_t* const project = &projects[i];

    for (u64 j = 0; j < buf_size(project->pro_pipelines); j++) {
      const pipeline_t* const pipeline = &project->pro_pipelines[j];
      items[k++] = new_item(sdscatprintf(sdsempty(), "%lld", pipeline->pip_id),
                            pipeline->pip_status);
    }
  }

  menu = new_menu(items);
  /* set_menu_format(menu, 10, 1); */
  window = newwin(0, 0, 0, 0);
  keypad(window, TRUE);
  set_menu_win(menu, window);
  set_menu_sub(menu, derwin(window, 0, 0, 1, 1));
  set_menu_mark(menu, ">");

  box(window, 0, 0);
  /* ui_print_in_middle(window, 1, 0, 40, "Pipelines", COLOR_PAIR(1)); */
  /* mvwaddch(window, 2, 0, ACS_LTEE); */
  /* mvwhline(window, 2, 1, ACS_HLINE, 38); */
  /* mvwaddch(window, 2, 39, ACS_RTEE); */
  /* mvprintw(LINES - 2, 0, "F1 to exit"); */
  refresh();

  post_menu(menu);
  wrefresh(window);

  int c;
  while ((c = wgetch(window)) != KEY_F(1)) {
    switch (c) {
      case KEY_DOWN:
        menu_driver(menu, REQ_NEXT_ITEM);
        break;
      case KEY_UP:
        menu_driver(menu, REQ_PREV_ITEM);
        break;
    }
    wrefresh(window);
  }
}

