# Renut at Sonic 4's addresses (XDK 2.0.11626, D3D9LTCG)

S4ep1-nx's Renut is TFU-nx's (katamari-nx's 321 hooks at TFU's addresses, TFU's device layout, XDK 2.0.6534). Moving it
to Sonic 4 takes two maps: TFU's hooked functions to Sonic 4's, and TFU's device offsets to Sonic 4's.

References (game data, used only here, never committed): TFU's default.xex (Mark's, SHA-256 `c28a5607...83f6316a`,
translated with TFU-nx's own manifest and config) and LoS1's (SHA-256 `0287bbd8...`, LoS1-nx's manifest and config).
LoS1 has Sonic 4's XDK build (11626) but plain D3D9; Sonic 4 links D3D9LTCG, so some small functions are reshaped or
inlined into the game's code.

## Functions (2026-11-14): 277 of TFU's 316 paired

| How | Count | Evidence |
|---|---|---|
| State tables | 242 rows | Sonic 4's render table `0x824D6498` (101 records) and sampler table `0x824D6958` (20) pair row for row with TFU's `0x82F43938` / `0x82F43DF8` by their default column (`tools/renut_state_tables.py` with TFU's tables; `renut_map/state_tables.txt`) |
| Call-site propagation | the rest | `tools/s4_d3d_propagate.py` (d3d_propagate.py with Sonic 4's tables and range): from the tables, unique shapes and the seeds below, every aligned `bl` pairs its targets; 2058 pairs, 40 conflicts dropped (`renut_map/tfu_to_s4.json`) |
| Seeds | 45 | `renut_map/seeds.txt`: d3d_match >= 0.85 with a 0.1 lead (TFU or LoS1, `match_tfu.txt`, `match_los1.txt`), and by kernel imports: Swap `sub_821C0E58` (the only VdSwap caller), Present `sub_8249E2F8` (the same callees as TFU's Present; LTCG moved it out of the D3D block), Resolve `sub_821C1C80` and SetScissorRect `sub_821B5B28` (Present's callees), InitializePresentationParameters `sub_821CF318` (VdQueryVideoMode), SetRingBufferParameters `sub_821C6788` (VdInitializeRingBuffer), the gamma reader `sub_821CADB0`, VdRetrainEDRAM's caller `sub_821D33B8` |

Hooks TFU left unhooked that Sonic 4 can have (LoS1 matches at 1.00, same XDK): BeginVertices `sub_821BF968`,
DrawVerticesUP `sub_821BFE00`, XGRegisterPixelShader `sub_821B99F8`.

Still open (39, `renut_map/cov.txt`): InsertCallback, AddRef, Acquire/ReleaseThreadOwnership, Direct3D_CreateDevice
and XenonGpu_Initialize (TFU's device init `sub_822E1950` is inlined into Sonic 4's `sub_821CF800`, which calls
VdInitializeEngines directly), DrawIndexedTessellatedVertices, Clear, Set{Vertex,Pixel}Shader, the lock and unlock
family, surface-level getters, CreateIndexBuffer, texture size helpers, and the small setters LTCG likely inlined
(SetStreamSource, SetIndices, SetVertexDeclaration, SetViewport, the B constants, UnsetAll, ResetAllState).

## Device layout: Sonic 4 = LoS1 (confirmed)

`tools/renut_field_diff.py` over 446 paired Direct3D functions (TFU to Sonic 4, `renut_map/field_diff.txt`), checked
name for name against LoS1-nx's `d3d.h` constants (LoS1's layout, verified in that game): all 14 shared fields agree.

| TFU offsets | Sonic 4 | Fields (votes / LoS1 name check) |
|---|---|---|
| 10880 .. 10943 | the same | creation thread, fences, flags (equal in TFU's and LoS1's d3d.h) |
| 11820 | +172 (11992) | vertex declaration (LoS1) |
| 11848 .. 12752 | +176 | 30+ fields; index buffer, render targets, depth, streams, strides, textures, viewport, scissor, shaders, predication, snapshot size (LoS1) |
| 12992 .. 13260 | +176 to +180 | tiling block and KickOff (13232 -> 13408, LoS1); 12992 -> 13172 and 13256 -> 13436 at +180: to check field by field |
| 13376 .. 13444 | +204 / +208 | VerticesRingEnd 13444 -> 13652 (LoS1) |
| 13500 .. 15004 | +212 | command buffer, presentation interval, present fence tag, critical sections, gamma ramp (LoS1) |
| 16560 .. 16728 | +216 | swap count, side buffer (LoS1) |
| 21532 .. 21600 | +216 to +224 | to check field by field |
| 22300 | +1900 (24200) | behavior flags (LoS1) |
| 22400 (allocation) | 24320 (0x5F00) | LoS1 |

Open: 11824 and 11832 (between the +172 and +176 fields), the tiling block's +180 fields, 21532..21600, TFU's -1 words
at 21700 and gamma pair 22272/22276.
