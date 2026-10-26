#include "sentinel.h"
#include "notify.h"

#include <pwd.h>
#include <syslog.h>
#include <sys/wait.h>

static char notify_user[64] = {0};
static uid_t notify_uid = (uid_t)-1;

void notify_init(void) {
    openlog("vfs-sentinel", LOG_PID | LOG_NDELAY, LOG_DAEMON);

    const char *user = getenv("SENTINEL_NOTIFY_USER");
    if (!user || !*user) user = getenv("SUDO_USER");   /* when run via sudo */
    if (!user || !*user) {
        syslog(LOG_WARNING, "no SENTINEL_NOTIFY_USER set: desktop notifications disabled");
        return;
    }

    struct passwd *pw = getpwnam(user);
    if (!pw) {
        syslog(LOG_WARNING, "notify user '%s' not found: desktop notifications disabled", user);
        return;
    }
    snprintf(notify_user, sizeof(notify_user), "%s", user);
    notify_uid = pw->pw_uid;
    syslog(LOG_INFO, "desktop notifications -> user %s (uid %u)", notify_user, (unsigned)notify_uid);
}

void notify_close(void) {
    closelog();
}

/* Runs notify-send as the desktop user, on that user's session bus.
 * Double fork so the daemon never blocks on, or leaves zombies from, the helper. */
static void desktop_notify(const char *level, const char *title, const char *body) {
    if (notify_uid == (uid_t)-1) return;

    pid_t pid = fork();
    if (pid < 0) return;
    if (pid > 0) {
        waitpid(pid, NULL, 0);          /* reaps the short-lived middle child */
        return;
    }

    if (fork() != 0) _exit(0);          /* middle child exits; grandchild is reparented */

    char bus[128];
    snprintf(bus, sizeof(bus), "DBUS_SESSION_BUS_ADDRESS=unix:path=/run/user/%u/bus", (unsigned)notify_uid);

    int devnull = open("/dev/null", O_RDWR);
    if (devnull >= 0) {
        dup2(devnull, STDIN_FILENO);
        dup2(devnull, STDOUT_FILENO);
        dup2(devnull, STDERR_FILENO);
    }

    execlp("runuser", "runuser", "-u", notify_user, "--",
           "env", bus,
           "notify-send", "-u", level, "-a", "VFS-Sentinel", "-i", "security-high",
           title, body, (char *)NULL);
    _exit(127);
}

void notify_send(const char *level, const char *title, const char *body) {
    int prio = LOG_INFO;
    if (strcmp(level, "critical") == 0) prio = LOG_ALERT;
    else if (strcmp(level, "low") == 0) prio = LOG_NOTICE;

    syslog(prio, "%s: %s", title, body);
    printf("[VFS-Sentinel] %s: %s\n", title, body);
    fflush(stdout);

    desktop_notify(level, title, body);
}
