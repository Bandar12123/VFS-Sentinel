# VFS-Sentinel

## What is ransomware

Ransomware is a type of malicious software or malware. It encrypts a victim's data, after which the attacker demands a ransom.

## How this project works on this

1. **Canary file** — no one can open it except a suspect. The canary file discovers any ransomware, even if it's a new one.

2. **Immediate interception** — it stops any process before it opens the file, not after.

3. **Instant automated response** — access prohibited, kill the process, and it records details without human intervention.

4. **Backup** — save a copy from any file it tries to open, for safety.

5. **Whitelist** — tells the difference between trusted system and real threats.

---

📁 `include/canary.h` | `src/canary.c`

## canary.h/canary.c bait unit (important things in this project)

this file is responsible about main idea The foundation upon which the entire project rests create Unknown file A dummy file that no legitimate user should ever touch.

📄 `src/canary.c` — function: `create_canary_file(target_dir, out_canary_path)`

make unknown file (`000_canary_.docx`) and put it inside the protect folder and put inside it unknown byte like the header of word real (`PK\x03\x04`) this for any process try to check he think it's a real file, not empty file. `out_canary_path`, so the rest of the program knows where the bait is.

📄 `src/canary.c` — function: `is_canary_file(filepath)`

check the path of file and return 1 if that contain `000_canary_` this call the `fanotify.c` every time that open file because decide is this normal or evil trying

📄 `src/canary.c` — function: `cleanup_canary_file(canary_path)`

Automatically removes bait files when protection stops, leaving no clutter behind.

---

📁 `include/fanotify.h` | `src/fanotify.c`

## fanotify.h / fanotify.c
Monitoring and Interception Unit, these files talk with the Linux kernel.

📄 `src/fanotify.c` — function: `init_fanotify`

Open channel to contact between the program and kernel system (`fanotify_init`) and need permission `FAN_CLASS_CONTENT`. This makes the kernel stop for any process and wait the decision (allow or deny) before continue. This needs root permission, so we need `sudo`.

📄 `src/fanotify.c` — function: `mark_directory(fan_fd, dir_path)`

This tells the kernel to watch the folder, and anyone try to open it (`FAN_OPEN_PERM`) send notification before you allow it. The process `FAN_EVENT_ON_CHILD` monitors subdirectories, not just the main folder.

📄 `src/fanotify.c` — function: `get_filepath_from_fd(fd, ...)`

An internal helper function that converts a kernel-provided file descriptor (fd) into a readable file path by reading `/proc/self/fd/<fd>`.

📄 `src/fanotify.c` — function: `start_event_loop(fan_fd)`

Main Event Loop: The core function that runs indefinitely, listening for kernel events and processing each event:

1- make sure it was a safe event

2- ignore any request come from the program itself

3- bring the path and check is it the canary. If yes, bring program name that try to open it (`/proc/<pid>/exe`) and make sure it's not in whitelist. If suspicious, kill it and don't let it get inside. If no, they call `backup_file()` first, and this let the open

4- send the decision (`FAN_ALLOW`/`FAN_DENY`) back to kernel

---

📁 `include/backup.h` | `src/backup.c`

## backup.h / backup.c
backup unit

📄 `src/backup.c` — function: `backup_file(src_fd)`

One-Time File Backup: Takes an open file descriptor and creates a single backup copy per file using `O_EXCL` to prevent overwriting. It validates that the file is a regular file (`S_ISREG`) and under 20MB to prevent system freezes on large files. All backups are stored in a fixed folder (`/tmp/.vfs_sentinel_backups`), and each backup file is named after the file's inode number (e.g. `598018.bak`) instead of its original name, to avoid collisions between different files.

📄 `src/backup.c` — function: `fallback_copy(src_fd, dest_fd)`

Internal Fallback Copy: If the fast `copy_file_range` call fails (e.g., across different filesystems), it falls back to a traditional read/write loop using an 8KB buffer to guarantee backup creation under all conditions.

---

📁 `include/sentinel.h` | `src/sentinel.c`

## 4. sentinel.c / sentinel.h
The entry interface, connects all above modules into a single function, keeping `main.c` clean and minimal.

📄 `src/sentinel.c` — function: `handle_shutdown(sig)`

Turns on automatically on exit signals (`Ctrl+C` or `SIGTERM`) to clean up bait files and exit the program safely.

📄 `src/sentinel.c` — function: `sentinel_run(target_dir)`

1- Automatically creates a test directory (`demo_vault`) if no path is provided.

2- Registers the signal handler.

3- (`create_canary_file`)

4- Initializes fanotify and attaches it to the target directory (`init_fanotify` + `mark_directory`).

5- Enters the monitoring loop (`start_event_loop`), which runs non-stop until stopped by the user.

---

## How to run it / How to use it

### 1. Build

Note: the `Makefile` expects a `main.c` file in the project root (not inside `src/`). If you're setting up the project yourself, make sure `main.c` exists in the root folder with this inside it:

```c
#include "sentinel.h"

int main(int argc, char *argv[]) {
    return sentinel_run(argc >= 2 ? argv[1] : NULL);
}
```

```bash
make clean
make
```

### 2. Run — Demo Mode (no folder needed)

If you run it without any argument, it automatically creates a test folder (`demo_vault`) with a sample file inside, and starts protecting it:

```bash
sudo ./sentinel
```

### 3. Run — Protect a Real Folder

Pass any folder path you want to protect:

```bash
sudo ./sentinel /path/to/your/folder
```

`sudo` is required because `fanotify` needs root permission to intercept file access at the kernel level.

### 4. Test It

Open a normal file inside the protected folder — it should open fine, and a backup copy gets created automatically:

```bash
cat /path/to/your/folder/some_file.txt
```

Try opening the bait file directly — the process should be killed instantly:

```bash
cat /path/to/your/folder/000_canary_.docx
echo "exit code: $?"   # 137 means it was killed
```

### 5. Stop It
Ctrl+C

This cleans up the bait file automatically before exiting.