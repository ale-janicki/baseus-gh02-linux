# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

**:date: ORDER**: Entries are organized in **descending chronological order** (newest first).

## [1.0.0] - 2026-11-08

### Added

- `gh02_fix` kernel module: for the Baseus AeQur GH02 dongle (`10d6:4802`) it replaces the 4-interface configuration descriptor Linux receives with the 6-interface one Windows receives, which exposes the working "usb audio game" output.
- DKMS packaging, so the module is rebuilt for every new kernel.
- `gh02-fix.service`: re-enumerates the dongle at boot if it came up before the module loaded.
- `install.sh` / `uninstall.sh`.
- README covering symptoms, the cause, install steps and PipeWire setup.

---

**Note**: This changelog is maintained manually and may not include all minor changes. For detailed commit history, please refer to the Git log.
