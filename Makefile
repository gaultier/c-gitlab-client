LDFLAGS = -lcurl

HEADERS := $(wildcard *.h)
gitlab_client: main.c $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -Wextra -Wvla -Wpedantic -g -flto -march=native $< -o $@ -O0 -fsanitize=address
