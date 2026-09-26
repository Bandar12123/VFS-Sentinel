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
