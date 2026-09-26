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
