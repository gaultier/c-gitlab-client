.PHONY: deps clean

LDFLAGS = deps/curl/lib/.libs/libcurl.a deps/mbedtls/lib/libmbedcrypto.a deps/mbedtls/lib/libmbedx509.a deps/mbedtls/lib/libmbedtls.a

HEADERS := $(wildcard *.h)
gitlab_client: main.c $(HEADERS)
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -Wextra -Wvla -Wpedantic -fno-omit-frame-pointer -g -flto -march=native $< -o $@ -O2 # -fsanitize=address

deps: 
	(cd deps/mbedtls && LDFLAGS='' make -j && ln -s library lib || true)
	(cd deps/curl && autoreconf -fi && LDFLAGS='' ./configure --enable-http --disable-alt-svc --disable-cookies --disable-file --disable-ftp --disable-gopher --disable-imap --disable-largefile --disable-ldap --disable-ldaps --disable-libgcc --disable-manual --disable-mime --disable-mqtt --disable-netrc --disable-pop3 --disable-progress-meter --disable-rtsp --disable-smb --disable-smtp --disable-sspi --disable-telnet --disable-tftp --disable-verbose --enable-optimize --disable-debug --without-hyper --without-zsh-functions-dir --without-fish-functions-dir --enable-static --disable-shared --disable-curldebug --without-nghttp2 --without-nghttp3 --without-brotli --without-zlib --without-libidn2 --without-zstd --with-mbedtls=`pwd`/../mbedtls --without-ssl && LDFLAGS='' make -j)

clean:
	rm -f gitlab_client
