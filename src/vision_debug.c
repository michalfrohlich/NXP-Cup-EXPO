/*
 * Legacy root-level vision_debug module kept only so older project files still
 * compile on branches that also build src/app/vision_debug.c.
 *
 * The active implementation lives in src/app/vision_debug.c. This translation
 * unit intentionally exports no VisionDebug_* symbols to avoid duplicate
 * definitions at link time.
 */
typedef int vision_debug_legacy_stub_t;
