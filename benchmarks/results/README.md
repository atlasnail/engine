# Benchmark results

Gated benchmark records live here, one file per run plus its raw log/CSV:

```
YYYY-MM-DD-<bench>-<platform>[-<variant>].md
YYYY-MM-DD-<bench>-<platform>[-<variant>].csv
YYYY-MM-DD-<bench>-<platform>[-<variant>].log
```

Use `benchmarks/TEMPLATE.md`. Never edit an old record to "fix" a number; add a new run and reference the old one. These files are the project's performance memory across sessions and agents.

`<platform>` is `win-x64` or `linux-x64`; include the hardware ID in the record, not the filename.
