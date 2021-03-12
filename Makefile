LDFLAGS = -lcurl -lmenu -lncurses -L/usr/local/Cellar/ncurses/6.2/lib/

HEADERS := $(wildcard *.h)
gitlab_client: main.c $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -Wextra -g $< -o $@ -fsanitize=address -D TEST
