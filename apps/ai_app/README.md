### `ai_app`

Native **cFS application** that ports the core math from Karpathy’s `microgpt.py` into **static-memory C** suitable for constrained targets.

This app currently provides:

- **Static-pool scalar autograd** (`Value`-style nodes, topological backward) for future on-orbit fine-tuning.
- **Inference-focused tensor ops** (RMSNorm, softmax, linear).
- **A MicroGPT-style forward step** with a static key/value cache.
- **cFS SB boilerplate** that subscribes to HK topics and normalizes raw payload bytes into a fixed feature vector (placeholder mapping).

### Description

`ai_app` is intended to consume housekeeping telemetry from the Software Bus, transform it into a fixed-size feature vector, and (once weights are provided) run a lightweight transformer forward pass to produce inference outputs.

The repository also includes host-side unit tests that validate the math against Python “golden” values.

### Command & Telemetry

**Subscriptions**

- Subscription topics are defined in [`fsw/src/ai_app_mission_subscribe.c`](fsw/src/ai_app_mission_subscribe.c) via numeric SB values from [`fsw/inc/ai_app_mission_ids.h`](fsw/inc/ai_app_mission_ids.h).
  - `AI_APP_SB_VALUE_CFE_ES_HK_TLM` defaults to `0x0803`
  - `AI_APP_SB_VALUE_SENSORS_HK_TLM` defaults to `0x0820` (placeholder)

**Message handling**

- The app receives buffers in [`fsw/src/ai_app_app.c`](fsw/src/ai_app_app.c), extracts the payload, and calls:
  - `AI_APP_NormalizePayloadToFeatures()` in [`fsw/src/ai_app_telemetry.c`](fsw/src/ai_app_telemetry.c)

**Important**

- The current mapping is intentionally generic: it scales the first N payload bytes to \([0,1]\). Missions should replace this with a field-aware extraction of the specific HK structs being subscribed to.
- No AI-specific command MIDs, output telemetry packets, or SB output messages are defined yet.

### Dependencies

**Runtime (cFS)**

- cFE core services and headers (uses `cfe.h`, SB, EVS, ES APIs).

**Host unit tests**

- `gcc` (or compatible C compiler) and `libm` (`-lm`).
- No cFE is required for the host unit tests; a minimal UtAssert-compatible shim is provided under `unit-test/`.

### Build instructions

#### Host unit tests (recommended first)

Run from this directory:

```bash
make -C unit-test check
```

This builds and runs `unit-test/ai_app_host_ut`, which executes:

- autograd goldens (from `scripts/golden/autograd_golden.py`)
- tensor goldens (from `scripts/golden/tensor_golden.py`)
- GPT forward goldens (from `scripts/golden/gpt_forward_golden.py`)
- subscription-list sanity checks

#### Full cFS bundle integration (app build)

`ai_app` is a standard cFS app built via CMake:

- App CMake: [`CMakeLists.txt`](CMakeLists.txt)
- Mission config list: [`mission_build.cmake`](mission_build.cmake)
- Platform config list: [`arch_build.cmake`](arch_build.cmake)

To include the app in a typical bundle mission build, add it to your target list, e.g. in `sample_defs/targets.cmake`:

- `list(APPEND MISSION_GLOBAL_APPLIST ai_app)`

### Model Architecture

The forward pass follows the structure in `microgpt.py` (GPT-2-like, simplified):

- **Token embedding** + **position embedding**
- **RMSNorm**
- Repeated for `n_layer`:
  - **Multi-head causal attention** with a **static KV cache** (up to `block_size`)
  - Residual connection
  - **RMSNorm**
  - **MLP**: Linear → ReLU → Linear
  - Residual connection
- Final **LM head** linear layer to logits

Implementation entry point:

- `AI_APP_GptForwardStep()` in [`fsw/src/ai_app_gpt.c`](fsw/src/ai_app_gpt.c)

Default “toy” dimensions (for initial bring-up / tests) are defined in:

- [`config/default_ai_app_mission_cfg.h`](config/default_ai_app_mission_cfg.h)

### Integration Notes

#### Model Weight Management (cFS Table Services)

`ai_app` manages GPT weights via **cFE Table Services** using a single weights table:

- **Table type**: `AI_APP_WeightsTable_t` (see `fsw/inc/ai_app_tbl.h`)
- **Table name**: `AI_APP.WEIGHTS`
- **Default image**: compiled-in (`fsw/src/ai_app_tbl.c`) for bring-up; missions should upload a `.tbl` image for real weights.

**Safety checks (validation callback)**

On every load/update, the table validation callback rejects the image and emits a **CRITICAL** event with a specific reason if any check fails:

- **Header integrity**: magic + version + expected dimensions (vocab/embd/block/head/layer)
- **CRC32**: table images provided from the ground must include a non-zero CRC32 and it must match the table bytes.
  The compiled default image may omit CRC with `Crc32==0` and `MissionVersion` beginning with `DEFAULT_`.
- **NaN/Inf scan**: rejects any NaN/Inf (reason includes location like `wte[3]` or `attn_wq[L0][12]`).
- **Range checks**: rejects weights outside `[AI_APP_WT_MIN, AI_APP_WT_MAX]` (reason includes index/value).

**Runtime access pattern (NASA-style Get–Process–Release)**

During inference, the app follows a deterministic cycle:

- Call `CFE_TBL_Manage()` at a safe sync point in the main loop
- `CFE_TBL_GetAddress()` to lock the current buffer
- Build a pointer-based `AI_APP_GptWeights` view into the locked table
- Run `AI_APP_GptForwardStep()` and immediately `CFE_TBL_ReleaseAddress()`

**Generating and loading `.tbl` images**

Typical workflow:

- Generate weights from your training pipeline and emit a binary table image matching `AI_APP_WeightsTable_t`
- Compute/populate the table CRC32 (and/or use cFS tooling such as `elf2cfetbl` / `tblCRCTool` in the bundle)
- Load/activate the table via standard **cFE table commands** (load/validate/activate)

**On-orbit weight updates**

- Upload a new table file to the spacecraft
- Issue cFE Table Services commands to **load** and **activate** the updated `AI_APP.WEIGHTS` table
- If validation fails, `ai_app` will emit a **CRITICAL** event indicating the specific reason (CRC mismatch, NaN/Inf index, out-of-range index/value, or dimension mismatch)

#### MsgIds are mission-specific

The default SB values in `ai_app_mission_ids.h` are placeholders intended for lab-style builds. Replace these with your mission’s actual MsgId routing.

#### Memory model

The math core is designed to avoid runtime heap allocation:

- autograd uses a static node pool (`AI_APP_MAX_NODES` default 8192)
- GPT forward uses compile-time bounded arrays for KV cache and scratch buffers

