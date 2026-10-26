CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -Iinclude
TARGET  = sentinel
SRCS    = src/main.c src/canary.c src/fanotify.c src/sentinel.c src/backup.c src/notify.c

PREFIX   ?= /usr/local
BIN_DST   = $(PREFIX)/sbin/vfs-sentinel
UNIT_DST  = /etc/systemd/system/vfs-sentinel.service
CONF_DST  = /etc/default/vfs-sentinel

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

# sudo make install  -> installs binary + service (config kept if it already exists)
install: $(TARGET)
	install -Dm755 $(TARGET) $(BIN_DST)
	install -Dm644 systemd/vfs-sentinel.service $(UNIT_DST)
	@if [ ! -f $(CONF_DST) ]; then \
		install -Dm644 systemd/vfs-sentinel.default $(CONF_DST); \
		sed -i "s/CHANGE_ME/$${SUDO_USER:-root}/g" $(CONF_DST); \
		echo ">> Edit $(CONF_DST) to choose the folder to protect"; \
	fi
	systemctl daemon-reload
	@echo ">> Start it with: sudo systemctl enable --now vfs-sentinel"

uninstall:
	-systemctl disable --now vfs-sentinel
	rm -f $(BIN_DST) $(UNIT_DST)
	systemctl daemon-reload
	@echo ">> Config left in $(CONF_DST) (delete it manually if you want)"

clean:
	rm -f $(TARGET)

.PHONY: all install uninstall clean
