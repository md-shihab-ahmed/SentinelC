# SentinelC

A lightweight, automated File Integrity Monitoring (FIM) and tamper-detection tool written in C for Windows systems.

[![Language](https://img.shields.io/badge/Language-C-00599C?style=flat-square&logo=c)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform](https://img.shields.io/badge/Platform-Windows-0078D6?style=flat-square&logo=windows)](https://learn.microsoft.com/en-us/windows/win32/)
[![License](https://img.shields.io/badge/License-MIT-green?style=flat-square)](LICENSE)

---

## 📌 Project Overview

**SentinelC** is an academic systems security project developed as part of the *Structured Programming Language Sessional* course in the Department of Cyber Security Engineering at the University of Frontier Technology, Bangladesh (UFTB).

The tool establishes a trusted state baseline of files within a monitored directory and audits them on demand to detect unauthorized alterations, newly created files, or deleted files.

---

## ⚙️ How It Works

SentinelC operates through a state-comparison pipeline:

```
[Target Directory] 
       │
       ▼
[Directory Scan (Win32 API)] ───► Extract: Name + Size (High/Low DWORD) + FNV-1a Hash
       │
       ▼
[State Evaluation]
 ├── Create Mode ──► Serialize binary snapshot into 'baseline.db' + record path in 'config.txt'
 └── Audit Mode  ──► Compare Current vs. Baseline:
                      ├── Same Name + Same Size + Same Hash ──► [MATCH]
                      ├── Same Name + Different Size/Hash   ──► [MODIFIED]
                      ├── In Monitored Dir but Not Baseline ──► [NEW FILE]
                      └── In Baseline but Not Monitored Dir ──► [DELETED]
                                │
                                ▼
                       Append to 'audit_log.txt'
```

### Hash Algorithm: FNV-1a (32-bit)
SentinelC implements the non-cryptographic **Fowler–Noll–Vo (FNV-1a)** 32-bit hash algorithm for fast, lightweight byte-level change detection:
- **Offset basis:** `2166136261u`
- **FNV prime:** `16777619u`

---

## ✨ Features

- **Binary Baseline Generation:** Saves up to 1,000 file metadata records into a compact binary format (`baseline.db`).
- **Tamper Detection:** Accurately detects byte-level changes in file content even when file size remains unchanged.
- **Audit Logging:** Automatically logs security events with action tags (`[MODIFIED]`, `[NEW FILE]`, `[DELETED]`) to `audit_log.txt`.
- **Configuration Persistence:** Remembers the monitored target directory path across sessions (`config.txt`).
- **Safe Baseline Reset:** Cleanly removes configuration and baseline records for re-initialization.

---

## 🚀 Getting Started

### Prerequisites
- Operating System: **Windows 10 / 11** (uses Win32 File API `FindFirstFileA` / `FindNextFileA`)
- Compiler: **GCC (MinGW)** or **MSVC (cl)**

### Compilation
Clone the repository and compile using GCC:
```bash
git clone https://github.com/md-shihab-ahmed/SentinelC.git
cd SentinelC
gcc -Wall -Wextra sentinel.c -o sentinel.exe
```

### Execution
Run the executable:
```bash
.\sentinel.exe
```

### Interactive Menu
```text
============================================
 SentinelC - File Integrity Monitoring Tool
============================================

1. Create Baseline
2. Run Audit
3. View Audit Log
4. Reset Baseline
5. Exit

Enter your choice: 
```

---

## 🧪 Verified Test Scenarios

The tool has been empirically validated against test suites:
- **Unmodified State:** Confirmed `[MATCH]` reported for identical files across runs.
- **Content Tampering:** Modified single characters in test files; correctly flagged as `[MODIFIED]`.
- **Unauthorized Injection:** Created new files in the folder; correctly flagged as `[NEW FILE]`.
- **File Deletion:** Removed monitored files; correctly detected and reported as `[DELETED]`.
- **Log Integrity:** Verified that `audit_log.txt` accurately records each anomaly.

---

## ⚠️ Security Context & Limitations

As an honest academic prototype, SentinelC has known design boundaries:
1. **Hash Cryptography:** FNV-1a is a fast change-detection hash, not a cryptographically collision-resistant hash (such as SHA-256). An adversary with knowledge of FNV-1a could craft hash collisions.
2. **Directory Scope:** The scan is non-recursive (monitors top-level directory items; ignores nested subdirectories).
3. **Execution Model:** Audits are user-triggered (polling/manual) rather than kernel-level event-driven real-time monitoring (`ReadDirectoryChangesW`).
4. **Platform Binding:** Bound to Windows Win32 API (`windows.h`).

---

## 🗺️ Roadmap

- [ ] Upgrade hashing engine to **SHA-256** using OpenSSL or WinCrypt API.
- [ ] Implement recursive directory traversal for nested folders.
- [ ] Add cross-platform POSIX support (`dirent.h` / `sys/stat.h`) for Linux.
- [ ] Implement automated real-time alerts using `ReadDirectoryChangesW`.

---

## 📄 License

This project is open-source under the [MIT License](LICENSE).
