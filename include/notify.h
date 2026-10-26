#ifndef NOTIFY_H
#define NOTIFY_H

/* Logging + desktop notifications.
 * Logs go to syslog (view with: journalctl -u vfs-sentinel -f).
 * Desktop notifications are sent to the user named in the
 * SENTINEL_NOTIFY_USER environment variable (or the sudo user). */

void notify_init(void);
void notify_close(void);

/* level: "low" | "normal" | "critical" */
void notify_send(const char *level, const char *title, const char *body);

#endif
