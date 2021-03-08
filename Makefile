LDFLAGS = -lcurl -lncurses

HEADERS := $(wildcard *.h)
gitlab_client: gitlab_client.c $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -Wextra $< -o $@
