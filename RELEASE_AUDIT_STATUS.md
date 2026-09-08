# Dungeons & Dolphins 4.19.1 optimization audit status

Checkpoint date: 2026-09-09

This tree is the current 4.19.1 optimized source checkpoint. The eleven FAP manifests report 4.19.1 and the host release gates pass after the `.inc` elimination and runtime-lifetime refactor.

## Current optimization state

- The former `dnd_app_shared.inc` implementation fragment and three text-including wrapper `.c` files are removed.
- `dnd_app_core.c` is compiled as a normal translation unit for Hub, Combat and Grants with explicit per-FAP build defines and dedicated entry/header files.
- The regenerated 32-bit common `DndDolphinsApp` layout is **3,416 B**, down from **4,676 B** (1,260 B / 26.9%).
- Optional measured runtimes are **48 B Catalog**, **64 B collection cache/index**, **76 B roll/dice**, **24 B Grant Review**, **248 B Combat**, and **308 B profile browser**.
- Catalog, profile, editor/input-hook, autosave, roll, Grant Review, collection-cache and Combat working objects follow first-use/last-use ownership rather than being permanently embedded in the common app state.
- Catalog teardown now frees both its streamed page and descriptor; Back snapshots catalog return state before releasing the screen-owned runtime.
- DNDCombat uses the 3,416 B base plus 248 B Combat and 76 B Roll runtimes (3,740 B before dynamic indexes/framework allocations). DNDGrants uses the 3,416 B base plus its 24 B Grant Review runtime during review work.
- The current optimized x86_64 entry-rooted Hub proxy is **115,542 B text+rodata**, 1,576 B (~1.35%) below the immediately preceding 117,118 B proxy. This is a host ownership proxy, not an ARM target-size claim.

## Validation

`python3 tests/host/run_tests.py` passes the catalog/grant audit, all eleven manifest links, all eleven lifecycle smoke tests with balanced project allocations, DNDolphins ownership exclusion, ASan/UBSan regression executables, large collection/Combat paging, Inventory bags/Bag Mover, Spellbook, Bestiary, Journal and Adventure rollback tests.

`python3 tests/host/layout32.py` regenerates the current 32-bit state measurements from the packaged source.

A fresh uFBT/clang-format pass cannot be claimed in this execution environment because neither executable is installed. Existing source style is preserved. Real Flipper/ARM stack high-water testing is still required before reducing the current stack reservations.
