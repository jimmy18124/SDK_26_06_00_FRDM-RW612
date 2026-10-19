# MCUXpresso with OpenThread Library Integration Guide

> Complete guide for integrating OpenThread library into MCUXpresso IDE projects

## Table of Contents

- [Prerequisites](#prerequisites)
- [Step 1: Download and Extract SDK Package](#step-1-download-and-extract-sdk-package)
- [Step 2: Set Up ot-nxp Environment](#step-2-set-up-ot-nxp-environment)
- [Step 3: Build OpenThread CLI Library](#step-3-build-openthread-cli-library)
- [Step 4: Update MCUXpresso Project XML Manifest](#step-4-update-mcuxpresso-project-xml-manifest)
- [Step 5: Enable OpenThread in Project Configuration](#step-5-enable-openthread-in-project-configuration)
- [Step 6: Import and Build in MCUXpresso IDE](#step-6-import-and-build-in-mcuxpresso-ide)
- [Step 7: Flash and Verify](#step-7-flash-and-verify)
- [Quick Reference: Platform Differences](#quick-reference-platform-differences)

---

## Prerequisites

### Software Requirements

- **MCUXpresso IDE**
- **Python 3.6+** (for automation scripts)
- **Git** (for cloning repositories)
- **ARM GCC Toolchain** (arm-none-eabi-gcc)

### Platform-Specific Setup

#### Windows Users

**Recommended: Use Git Bash** for a Unix-like command-line environment:
- Download Git for Windows: [https://git-scm.com/download/win](https://git-scm.com/download/win)
- Git Bash is included with installation
- Benefits: Use same commands as Linux, forward slashes in paths

**Enable Git Long Path Support** (run once):

```bash
git config --global core.longpaths true
```

#### Linux Users

No additional setup required. Use your standard terminal.

### Hardware Requirements

Refer to the [Step 7: Flash and Verify](#step-7-flash-and-verify) section for your hardware platform (e.g., RW612, MCXW71).

---

## Step 1: Download and Extract SDK Package

1. Download the MCUXpresso SDK zip package for your target platform from the [MCUXpresso SDK Builder](https://mcuxpresso.nxp.com/).

2. Extract the package to a local directory:

   **Linux**:
   ```bash
   unzip <sdk_package>.zip -d <sdk_root>
   ```

   **Windows**: Use File Explorer or a tool such as 7-Zip to extract the zip to `<sdk_root>`.

3. Navigate to the `mcuxpresso_support` directory inside the extracted SDK:

   ```bash
   cd <sdk_root>/boards/<board>/coex_examples/third_party/mcuxpresso_support
   ```

4. Clone the ot-nxp repository into this directory:

   ```bash
   git clone https://github.com/NXP/ot-nxp.git -b <ot_github_release_tag> ot-nxp
   ```

   > **Note**: The target directory must be named `ot-nxp`. All subsequent steps rely on this relative path.

---

## Step 2: Set Up ot-nxp Environment

### 2.1 Initialize ot-nxp Submodules

Navigate into the cloned ot-nxp directory:

```bash
cd <sdk_root>/boards/<board>/coex_examples/third_party/mcuxpresso_support/ot-nxp
```

Initialize submodules:

```bash
git submodule update --init
```

### 2.2 Update NXP SDK

Run the SDK update script:

```bash
python third_party/nxp_matter_support/scripts/update_nxp_sdk.py --platform common
```

> **Note**: SDK download location differs between platforms due to Windows' 260-character path limitation:
> - **Windows**: `boards/<board>/coex_examples/third_party/mcuxpresso_support/sdk/` (same level as `ot-nxp/`, to avoid long paths)
> - **Linux**: `boards/<board>/coex_examples/third_party/mcuxpresso_support/ot-nxp/third_party/nxp_matter_support/github_sdk/sdk_next/repo/mcuxsdk/`

If `update_nxp_sdk.py` fails, manually run `west update`:

**Windows**:
```bash
cd <sdk_root>/boards/<board>/coex_examples/third_party/mcuxpresso_support/sdk/repo/mcuxsdk_manifest
west update
```

**Linux**:
```bash
cd <sdk_root>/boards/<board>/coex_examples/third_party/mcuxpresso_support/ot-nxp/third_party/nxp_matter_support/github_sdk/sdk_next/repo/mcuxsdk_manifest
west update
```

### 2.3 Set Up Environment

**Windows (Git Bash)**:

```bash
cd <sdk_root>/boards/<board>/coex_examples/third_party/mcuxpresso_support/ot-nxp

# Source environment setup (Git Bash supports source command)
source ../sdk/repo/mcuxsdk/mcux-env.sh

# Export SDK configuration
west mcuxsdk-export
```

**Linux**:

```bash
cd <sdk_root>/boards/<board>/coex_examples/third_party/mcuxpresso_support/ot-nxp

# Source environment setup
source ./third_party/nxp_matter_support/github_sdk/sdk_next/repo/mcuxsdk/mcux-env.sh

# Export SDK configuration
west mcuxsdk-export
```

**Note for Windows CMD users**: If using Windows Command Prompt instead of Git Bash, use `mcux-env.cmd` instead of `source ./mcux-env.sh`.

---

## Step 3: Build OpenThread CLI Library

### 3.1 Set ARM GCC Toolchain Path

**Both Windows (Git Bash) and Linux**:
```bash
export ARMGCC_DIR=<path-to-armgcc>
```

**Windows CMD alternative**:
```cmd
set ARMGCC_DIR=<path-to-armgcc>
```

### 3.2 Build for Your Platform

Navigate to ot-nxp directory:

```bash
cd <sdk_root>/boards/<board>/coex_examples/third_party/mcuxpresso_support/ot-nxp
```

**Example: Build for RW612 Platform**

```bash
./script/build_rw612 ot_cli -DOT_NXP_BUILD_APP_AS_LIB=ON -DBOARD_APP_UART_INSTANCE=0 -DOT_NXP_DISABLE_TCP=ON -DOT_NXP_LWIP_IPERF=ON -DOT_APP_CLI_FREERTOS_IPERF=ON -DOT_APP_BR_FREERTOS=OFF -DOT_NXP_ENABLE_WPA_SUPP_MBEDTLS=OFF -DCMAKE_BUILD_TYPE=Debug -DCOEX_RTOS_MAX_PRIO=<value_for_configMAX_PRIORITIES>
```

### 3.3 Troubleshooting: Build Fails on Windows (Long Path Issue)

**Symptom**: Build fails with "subcommand failed" or path-related errors on Windows.

**Solution**: Use virtual drive mapping to shorten paths:

```bash
# Create virtual drive mapping directly to mcuxpresso_support (Git Bash)
cmd //c "subst W: <sdk_root>\\boards\\<board>\\coex_examples\\coex_wifi_edgefast\\third_party\\mcuxpresso_support"

# Verify mapping
cmd //c "subst"

# Navigate to shortened path
cd /w/ot-nxp

# Rebuild
./script/build_rw612 ot_cli <options>
```

> **Note**: `W:` is a temporary virtual drive (session-based, no files are copied). It disappears after reboot and can be removed manually with `cmd //c "subst W: /D"`. Removing it does **not** affect your original files.

### 3.4 Expected Build Output

After successful build in `build_<platform>/<platform>_ot_cli/`:

- **OpenThread Libraries**: `lib/*.a` files
- **Compilation Database**: `compile_commands.json` (required for Step 4)
- **Build System Files**: `build.ninja`, CMake configuration

---

## Step 4: Update MCUXpresso Project XML Manifest

### 4.1 Script Options

| Option | Description | Example |
|--------|-------------|----------|
| `--app <app_name>` | Specify application name (required) | `--app coex_wifi_edgefast` |
| `--dry-run` | Preview changes without modifying files | `--dry-run` |
| `--verbose` | Show detailed progress information | `--verbose` |

### 4.2 Run the Automation Script

Navigate to mcuxpresso_support directory:

```bash
cd <sdk_root>/boards/<board>/coex_examples/third_party/mcuxpresso_support
```

Update XML manifest for your application:

```bash
python add_openthread_to_mcuxpresso.py --app <your_application_name>
```

**What this script does**:
1. Analyzes OpenThread header dependencies from `compile_commands.json`
2. Adds include paths to the XML manifest
3. Adds source groups with header files
4. Creates backup of original XML file
5. Generates detailed change report

**Output Files**:
- Updated XML: `boards/<board>/<example_type>/<app_name>/*.xml`
- Backup: `*.original_backup_<timestamp>.xml`
- Report: `XML_CHANGE_REPORT_<timestamp>.md`

To restore the original XML file if needed:

```bash
# Find the backup
ls -lt *.original_backup_*.xml | head -1

# Restore
cp <backup_file> <original_xml_file>
```

---

## Step 5: Enable OpenThread in Project Configuration

Navigate to your application directory:

```bash
cd <sdk_root>/boards/<board>/coex_examples/<app_name>
```

Edit `app_mcuxpresso_config.h`:

Set `CONFIG_OT` to 1:

```c
#define CONFIG_OT       1  // Enable OpenThread
```

---

## Step 6: Import and Build in MCUXpresso IDE

### 6.1 Install SDK into MCUXpresso IDE

> **Note**: The SDK prepared through Steps 1–5 is now ready to be installed into MCUXpresso IDE.

1. Open MCUXpresso IDE
2. Navigate to the **Installed SDKs** tab (bottom panel)
3. Drag-and-drop your SDK archive or SDK folder into the **Installed SDKs** panel
4. MCUXpresso IDE will install/register the SDK

### 6.2 Import Project from Installed SDK

1. Click "**Import SDK example(s)...**" in the Quickstart Panel (bottom left)
2. Select your installed SDK from the dropdown list
3. Navigate through the project tree:
   - Expand `<board>` (e.g., `frdmrw612`)
   - Expand `<example_type>` (e.g., `coex_examples`)
   - Select `<app_name>` (e.g., `coex_wifi_edgefast`)
4. Click **Next**, then **Finish**

### 6.3 Build Project

1. Right-click on the imported project in Project Explorer
2. Select "Build Configurations" → "Set Active" → "Debug" or "Release"
3. Click "Build Project" or press Ctrl+B
4. Verify the build succeeded: check that the build log contains no `undefined reference` errors related to OpenThread, and that the map file includes OpenThread symbols.

**Expected Build Output**:
- Binary file: `<build-config>/<app_name>.axf`
- Map file with OpenThread symbols
- No linking errors related to OpenThread library

---

## Step 7: Flash and Verify

### RW612 Platform

#### Hardware Setup

- **Board**: FRDM-RW612
- **USB Connection**: Type-C cable to J7 (MCU-Link port)
- **Serial Settings**:
  - Baud rate: 115200
  - Data bits: 8
  - Parity: None
  - Stop bits: 1
  - Flow control: None

#### Flash the Binary and Verify

**Flash the Binary**:

```bash
J-Link> loadbin <path>/frdmrw612_coex_wifi_edgefast.bin, 0x08000000
```

**Verify coex_wifi_edgefast Example**:

After flashing, open a serial terminal (115200 8N1) and verify:

1. **Check Initial Output**:

```
        Coex APP

SHELL build: ========================================
========================================
     Initialize RW612 Module
========================================
Copyright  2020  NXP
========================================

WiFi shell initialization
========================================
BLE shell initialization
@bt> ========================================

@Coex>
```

2. **Verify WiFi Functionality**:

```bash
@Coex> wifi.wlan-version
WLAN Driver Version   : v1.3.r48.p10
WLAN Firmware Version : rw610w-V2, IMU, FP99, 18.99.6.p6, PVE_FIX 1

@Coex> wifi.wlan-scan
Scan scheduled...
2 networks found:
1C:6A:7A:87:FF:BF  "NXP" Infra
        channel: 161
        rssi: -74 dBm
        security: WPA2
```

3. **Verify BLE Functionality**:

```bash
@Coex> bt.init
Bluetooth initialized
Settings Loaded

@Coex> bt.scan on
Bluetooth active scan enabled
[DEVICE]: 44:6D:F5:85:DC:5F (random), RSSI -64
```

4. **Verify OpenThread** (if CONFIG_OT=1):

**Note**: OpenThread CLI commands are input via the UART port specified by `BOARD_APP_UART_INSTANCE` during OT library build.

```bash
> ifconfig up
Done
> thread start
Done
> state
leader
Done
```

---

## Quick Reference: Platform Differences

### Directory Structure Comparison

**Windows**:
```
boards/<board>/coex_examples/
├── third_party/
│   └── mcuxpresso_support/
│       ├── add_openthread_to_mcuxpresso.py
│       ├── README.md
│       ├── ot-nxp/                    ← Cloned from GitHub
│       │   ├── build_rw612/           ← Generated after Step 3 build
│       │   ├── script/build_rw612
│       │   └── third_party/nxp_matter_support/
│       └── sdk/                       ← Downloaded by Step 2.2 (Windows only)
│           └── repo/mcuxsdk/          ← Actual SDK root
└── coex_wifi_edgefast/                ← Application directory
    └── *.xml
```

**Linux**:
```
boards/<board>/coex_examples/
├── third_party/
│   └── mcuxpresso_support/
│       ├── add_openthread_to_mcuxpresso.py
│       ├── README.md
│       └── ot-nxp/                              ← Cloned from GitHub
│           ├── build_rw612/                     ← Generated after Step 3 build
│           ├── script/build_rw612
│           └── third_party/nxp_matter_support/
│               └── github_sdk/sdk_next/
│                   └── repo/mcuxsdk/            ← Downloaded by Step 2.2
└── coex_wifi_edgefast/                          ← Application directory
    └── *.xml
```
```
---
