
# Kernel Customization for Jetpack 5.1.2 for the CARKIT2C0 with GMSL

## Overview

This directory contains everything needed to customize the Jetpack 5.1.2 kernel for the CARKIT2C0 with GMSL support. You can choose between two approaches:

- **Automated Installation** (Recommended): Use the provided shell script for a guided, error-checked installation
- **Manual Installation**: Follow the step-by-step instructions below

## Prerequisites
- WSL installation running Ubuntu-20.04
- Patch file for kernel modifications
- Pinmux configuration file
- GPIO configuration file

## (Prerequisite) Host (WSL) Software Setup
https://confluencewikiprod.intra.infineon.com/spaces/ASRSSET/pages/1317870076/NVIDIA+Jetson+AGX+Orin+Documentation+for+CTRX#NVIDIAJetsonAGXOrinDocumentationforCTRX-Host(WSL)SoftwareSetupwsl_setup

## Automated Installation (Recommended)

The `install_carkit2c0.sh` script automates the entire process with error checking, progress tracking, and backup creation.

### Quick Start
```bash
# Set your L4T path and run the script
export L4T=/path/to/JetPack/Linux_for_Tegra
./install_carkit2c0.sh

# Or specify the path directly
./install_carkit2c0.sh -l /path/to/JetPack/Linux_for_Tegra
```

### Script Options
```bash
Usage: ./install_carkit2c0.sh [OPTIONS]

OPTIONS:
    -l <l4t_path>       Path to L4T (Linux_for_Tegra) directory
    -p <patch_path>     Path to the patch file (default: ./carkit2c0-gmsl.patch)
    -s                  Skip flashing (build only)
    -f                  Force operation (skip confirmations)
    -h                  Show help message

EXAMPLES:
    ./install_carkit2c0.sh -l /home/user/nvidia/nvidia_sdk/JetPack_5.1.2_Linux_JETSON_AGX_ORIN_TARGETS/Linux_for_Tegra
    ./install_carkit2c0.sh -l /path/to/L4T -s  # Build only, skip flashing
    ./install_carkit2c0.sh -l /path/to/L4T -f  # Force mode, skip confirmations
```

### What the Script Does
- ✅ Validates all prerequisites and dependencies
- ✅ Installs required packages (flex, bison, libssl-dev)
- ✅ Copies configuration files with automatic backup
- ✅ Sets up git configuration if needed
- ✅ Applies the kernel patch safely
- ✅ Builds the custom kernel (with confirmation prompt)
- ✅ Copies build outputs to correct locations
- ✅ Flashes the device (optional)
- ✅ Provides detailed progress and error reporting


## Manual Installation

If you prefer to execute the steps manually or need to troubleshoot issues, follow these detailed steps:

### 1. Copy Configuration Files
Copy the configuration files into the bootloader
```bash
# Copy pinmux file to bootloader directory
cp tegra234-mb1-bct-pinmux-p3701-0000-a04.dtsi $L4T/bootloader/t186ref/BCT/tegra234-mb1-bct-pinmux-p3701-0000-a04.dtsi

# Copy GPIO configuration file
cp tegra234-mb1-bct-gpio-p3701-0000-a04.dtsi $L4T/bootloader/tegra234-mb1-bct-gpio-p3701-0000-a04.dtsi
```

### 2. Apply Kernel Patch
```bash
cd $L4T/source/public/
git init
git add .
git commit -m "Initial commit" # Optional

# Apply the patch
patch -p1 < /path/to/your/patch/carkit2c0-gmsl.patch

# Verify patch was applied successfully
git status
```

### 3. Build and Flash
```bash
# Install missing packages
sudo apt-get install flex bison libssl-dev

# Build kernel with new configuration
cd $L4T/source/public/
./nvbuild.sh -o $PWD/kernel_out

# Copy the generated files
cp $L4T/source/public/kernel_out/arch/arm64/boot/dts/nvidia/tegra234-p3701-0005-p3737-0000.d
tb $L4T/kernel/dtb/
cp $L4T/source/public/kernel_out/arch/arm64/boot/Image $L4T/kernel/Image

# Flash the updated kernel and bootloader
cd $L4T/
sudo ./flash.sh jetson-agx-orin-devkit mmcblk0p1
```

## Verification
After flashing, verify the customizations are active by checking relevant system files and GPIO states.
