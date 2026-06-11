# LuaPilot

A standalone Lua 5.5 binary for Linux scripting and automation, written
in C++23. Embeds OpenSSL, SQLite, miniz, nlohmann/json, cpp-httplib,
and tomlplusplus statically — one binary, no system dependencies
beyond glibc.

Can be used in three modes:

1. **As a Lua interpreter** : `luapilot script.lua` or `luapilot folder/`
   (looks for `main.lua` inside).
2. **As a packager** : `luapilot --create-exe script.lua app` produces a
   self-contained executable with the script and its `require`d modules
   embedded as a ZIP appended to the binary.
3. **As a library of bindings** : Lua scripts get
   `luapilot.json`, `luapilot.http`, `luapilot.sqlite`,
   `luapilot.socket`, `luapilot.inotify`, `luapilot.workers`,
   `luapilot.user`, and more.

## Quick start

```sh
git clone https://github.com/Chipsterjulien/luapilot_standalone.git
cd luapilot_standalone
./build_local.sh        # downloads deps and compiles (~5 min first time)
./run_tests.sh          # offline harness — should print 827 PASS / 0 FAIL
./test/luapilot --help
```

The build script vendors and compiles all its dependencies. The only
prerequisites on your system are a C++23 compiler, CMake, `wget`, and
`unzip`.

## Documentation

- **English** : [`docs/en/README.md`](docs/en/README.md)
- **Français** : [`docs/fr/README.md`](docs/fr/README.md)

The docs are split per module under `docs/en/modules/` (and `docs/fr/`).
You can generate a single PDF manual with:

```sh
cd docs && make manual-en.pdf   # or manual-fr.pdf
```

Requires `pandoc` and a LaTeX engine (`texlive-xetex` is fine).

## Releases

Download prebuilt binaries from the
[releases page](https://github.com/Chipsterjulien/luapilot_standalone/releases).

## License

See [LICENSE](LICENSE).
