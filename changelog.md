# Changelog

## 2026-11-01

- Port the BYgram Android Dobby 1.2 fix onto its original source revision:
  correct near-page selection and preserve existing mappings during allocation.
- Add an allocation regression check and reproduction instructions; pass on
  Android API 35/36/37 x86_64 with 4 KiB and 16 KiB pages.
