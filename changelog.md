# Changelog

## 2026-11-01

- Base Android integration on latest LSPosed/Dobby upstream commit
  `c1da0315d7a2069bde2ab432e45d4ab6df91237a`.
- Preserve occupied RW and PROT_NONE mappings during near-address allocation.
  Adapt the allocation regression test to the current OSMemory API.
