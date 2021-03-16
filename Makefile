LDFLAGS = -lcurl

HEADERS := $(wildcard *.h)
gitlab_client: main.c $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -Wextra -Wvla -Wpedantic -fno-omit-frame-pointer -g -flto -march=native $< -o $@ -O0 -fsanitize=address
