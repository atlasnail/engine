# QA — decisions needed from you

Most decisions are made and recorded (see `docs/decisions/`). These are the ones
that need your judgment. Edit the **Answer** line under each question, save the
file, and ask me to review it.

Recommended answers are marked. Everything works without answering them today;
they matter before the project is shared or released.

---

## Q1. License

The repository currently has no `LICENSE` file. That is deliberate: choosing a
license is your call, and it affects what others may do with the engine and with
games built on it.

- [ ] **MIT** (recommended): short, permissive, the common choice for small
      engines and libraries. Anyone may use it commercially; you keep copyright.
- [ ] **Apache-2.0**: permissive like MIT, plus an explicit patent grant and
      trademark language. Longer and more corporate-friendly.
- [ ] **Zlib**: permissive and very short; common in game tooling.
- [ ] Decide later (keeps the repo private-ish in practice; not recommended once
      the repo is public).

**Answer:**

---

## Q2. Project and engine name

"engine" is currently the folder name and working title; namespaces and targets
use `eng`. A real name affects the repository name, docs, package names, include
paths (`eng/...`), and later the SDK layout.

- [ ] **Keep `engine` / `eng` for now** (recommended until 1.0): renaming later
      is a mechanical refactor plus a note in the changelog.
- [ ] Pick a name now (write it below); I will apply it consistently.

**Answer:**

---

## Q3. How to verify Linux

Windows is verified locally. Linux is configured in CI and presets but has never
been compiled on this machine (no WSL, Docker or Linux toolchain installed).

- [ ] **Push and let GitHub Actions verify** (recommended): the CI workflow
      builds and tests on Ubuntu 24.04, including ASan/UBSan.
- [ ] Install WSL2 + Ubuntu locally; I can build and test without CI.
- [ ] Use a separate Linux machine/VM.

**Answer:**

---

## Q4. How often should I pause for your review?

You asked me to keep working and inform you afterwards. This only matters for
taste calls where a wrong guess would be expensive.

- [ ] **Keep going autonomously; only ask for licenses, names, money or
      irreversible actions** (recommended).
- [ ] Check in at every milestone boundary.
- [ ] Check in more often.

**Answer:**

---

## Notes

- Anything not listed here I will decide, record in `docs/status.md` (and an ADR
  if it is hard to reverse), and report in my summary.
- Unanswered questions do not block M1 (window, loop, input) from starting.
