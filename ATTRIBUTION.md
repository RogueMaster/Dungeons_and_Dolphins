# Rules attribution

This work includes material from the System Reference Document 5.2.1 ("SRD 5.2.1") by Wizards of the Coast LLC, available at https://www.dndbeyond.com/srd. The SRD 5.2.1 is licensed under the Creative Commons Attribution 4.0 International License, available at https://creativecommons.org/licenses/by/4.0/legalcode.

Pocket d20 implements compact tracking and dice calculations inspired by the SRD; it does not include the complete SRD text or a complete rules database.

## Persistence references reviewed

The storage design was informed by established Flipper Zero application patterns, including:

- FlipperTasks: https://github.com/MadLadSquad/FlipperTasks
- FlipNote: https://github.com/morty517/flipnote
- Flipper Zero Note Application: https://github.com/AdrianN001/Flipper-Zero-Note-Application

Pocket d20's save implementation is original to this project. It uses the native Flipper Storage API with a temporary file, backup rotation, schema version, payload validation, and checksum.

