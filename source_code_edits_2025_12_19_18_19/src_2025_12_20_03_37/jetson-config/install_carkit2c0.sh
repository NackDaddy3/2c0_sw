#!/bin/bash

# CARKIT2C0 GMSL Installation Script for Jetpack 5.1.2
# This script automates the kernel customization process as documented in README.md

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

print_step() {
    echo -e "${BLUE}[STEP]${NC} $1"
}

# Function to display usage
usage() {
    cat << EOF
Usage: $0 [OPTIONS]

This script automates the CARKIT2C0 GMSL kernel customization for Jetpack 5.1.2.

OPTIONS:
    -l <l4t_path>       Path to L4T (Linux_for_Tegra) directory
    -p <patch_path>     Path to the patch file (default: ./carkit2c0-gmsl.patch)
    -s                  Skip flashing (build only)
    -f                  Force operation (skip confirmations)
    -h                  Show this help message

ENVIRONMENT VARIABLES:
    L4T                 Path to Linux_for_Tegra directory (alternative to -l)

EXAMPLES:
    $0 -l /home/user/nvidia/nvidia_sdk/JetPack_5.1.2_Linux_JETSON_AGX_ORIN_TARGETS/Linux_for_Tegra
    export L4T=/path/to/Linux_for_Tegra && $0
    $0 -l /path/to/L4T -s  # Build only, skip flashing

EOF
}

# Initialize variables
L4T_PATH=""
PATCH_PATH=""
SKIP_FLASH=false
FORCE=false

# Get script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
DEFAULT_PATCH_PATH="$SCRIPT_DIR/carkit2c0-gmsl.patch"
CONFIG_DIR="$SCRIPT_DIR/Linux_for_Tegra"

# Parse command line arguments
while getopts "l:p:sfh" opt; do
    case $opt in
        l)
            L4T_PATH="$OPTARG"
            ;;
        p)
            PATCH_PATH="$OPTARG"
            ;;
        s)
            SKIP_FLASH=true
            ;;
        f)
            FORCE=true
            ;;
        h)
            usage
            exit 0
            ;;
        \?)
            print_error "Invalid option: -$OPTARG"
            usage
            exit 1
            ;;
    esac
done

# Check for L4T path
if [[ -z "$L4T_PATH" ]]; then
    if [[ -n "$L4T" ]]; then
        L4T_PATH="$L4T"
    else
        print_error "L4T path not specified. Use -l option or set L4T environment variable."
        usage
        exit 1
    fi
fi

# Set default patch path if not specified
if [[ -z "$PATCH_PATH" ]]; then
    PATCH_PATH="$DEFAULT_PATCH_PATH"
fi

# Function to check prerequisites
check_prerequisites() {
    print_step "Checking prerequisites..."
    
    # Check if running on WSL/Ubuntu
    if ! grep -qi ubuntu /etc/os-release 2>/dev/null; then
        print_warning "This script is designed for Ubuntu. Current OS may not be supported."
    fi
    
    # Check if L4T directory exists
    if [[ ! -d "$L4T_PATH" ]]; then
        print_error "L4T directory not found: $L4T_PATH"
        exit 1
    fi
    
    # Check if patch file exists
    if [[ ! -f "$PATCH_PATH" ]]; then
        print_error "Patch file not found: $PATCH_PATH"
        exit 1
    fi
    
    # Check if configuration directory exists
    if [[ ! -d "$CONFIG_DIR" ]]; then
        print_error "Configuration directory not found: $CONFIG_DIR"
        exit 1
    fi
    
    # Check required directories in L4T
    local required_dirs=(
        "$L4T_PATH/source/public"
        "$L4T_PATH/bootloader"
        "$L4T_PATH/kernel"
    )
    
    for dir in "${required_dirs[@]}"; do
        if [[ ! -d "$dir" ]]; then
            print_error "Required L4T directory not found: $dir"
            exit 1
        fi
    done
    
    # Check for required tools
    local required_tools=("git" "patch" "make" "gcc")
    for tool in "${required_tools[@]}"; do
        if ! command -v "$tool" &> /dev/null; then
            print_error "Required tool not found: $tool"
            print_info "Install missing tools with: sudo apt-get install build-essential git"
            exit 1
        fi
    done
    
    print_info "Prerequisites check completed successfully"
}

# Function to install required packages
install_packages() {
    print_step "Installing required packages..."
    
    local packages=("flex" "bison" "libssl-dev")
    local missing_packages=()
    
    # Check which packages are missing
    for pkg in "${packages[@]}"; do
        if ! dpkg-query -W -f='${Status}' "$pkg" 2>/dev/null | grep -q "install ok installed"; then
            missing_packages+=("$pkg")
        fi
    done
    
    if [[ ${#missing_packages[@]} -eq 0 ]]; then
        print_info "All required packages are already installed"
        return
    fi
    
    print_info "Installing missing packages: ${missing_packages[*]}"
    if ! sudo apt-get update; then
        print_error "Failed to update package lists"
        exit 1
    fi
    
    if ! sudo apt-get install -y "${missing_packages[@]}"; then
        print_error "Failed to install required packages"
        exit 1
    fi
    
    print_info "Required packages installed successfully"
}

# Function to copy configuration files
copy_config_files() {
    print_step "Copying configuration files..."
    
    # Define source and destination paths
    local pinmux_src="$CONFIG_DIR/bootloader/t186ref/BCT/tegra234-mb1-bct-pinmux-p3701-0000-a04.dtsi"
    local pinmux_dst="$L4T_PATH/bootloader/t186ref/BCT/tegra234-mb1-bct-pinmux-p3701-0000-a04.dtsi"
    
    local gpio_src="$CONFIG_DIR/bootloader/tegra234-mb1-bct-gpio-p3701-0000-a04.dtsi"
    local gpio_dst="$L4T_PATH/bootloader/tegra234-mb1-bct-gpio-p3701-0000-a04.dtsi"
    
    # Check if source files exist
    if [[ ! -f "$pinmux_src" ]]; then
        print_error "Pinmux configuration file not found: $pinmux_src"
        exit 1
    fi
    
    if [[ ! -f "$gpio_src" ]]; then
        print_error "GPIO configuration file not found: $gpio_src"
        exit 1
    fi
    
    # Create destination directories if they don't exist
    mkdir -p "$(dirname "$pinmux_dst")"
    mkdir -p "$(dirname "$gpio_dst")"
    
    # Backup existing files if they exist
    if [[ -f "$pinmux_dst" ]]; then
        print_info "Backing up existing pinmux file"
        cp "$pinmux_dst" "$pinmux_dst.backup.$(date +%Y%m%d_%H%M%S)"
    fi
    
    if [[ -f "$gpio_dst" ]]; then
        print_info "Backing up existing GPIO file"
        cp "$gpio_dst" "$gpio_dst.backup.$(date +%Y%m%d_%H%M%S)"
    fi
    
    # Copy configuration files
    if ! cp "$pinmux_src" "$pinmux_dst"; then
        print_error "Failed to copy pinmux configuration file"
        exit 1
    fi
    print_info "Copied pinmux configuration file"
    
    if ! cp "$gpio_src" "$gpio_dst"; then
        print_error "Failed to copy GPIO configuration file"
        exit 1
    fi
    print_info "Copied GPIO configuration file"
    
    print_info "Configuration files copied successfully"
}

# Function to setup git configuration if needed
setup_git() {
    print_step "Setting up git configuration..."
    
    cd "$L4T_PATH/source/public"
    
    # Check if git user is configured globally or locally
    if ! git config user.name &>/dev/null && ! git config --global user.name &>/dev/null; then
        print_warning "Git user.name not configured"
        git config user.name "CARKIT2C0 Build"
        print_info "Set git user.name to 'CARKIT2C0 Build'"
    fi
    
    if ! git config user.email &>/dev/null && ! git config --global user.email &>/dev/null; then
        print_warning "Git user.email not configured"
        git config user.email "build@carkit2c0.local"
        print_info "Set git user.email to 'build@carkit2c0.local'"
    fi
    
    print_info "Git configuration ready"
}

# Function to apply kernel patch
apply_patch() {
    print_step "Applying kernel patch..."
    
    cd "$L4T_PATH/source/public"
    
    # Initialize git repository if it doesn't exist
    if [[ ! -d ".git" ]]; then
        print_info "Initializing git repository"
        git init
    fi
    
    # Setup git configuration
    setup_git
    
    # Check if there are any uncommitted changes
    if ! git diff-index --quiet HEAD 2>/dev/null; then
        print_warning "Uncommitted changes detected"
        if [[ "$FORCE" != true ]]; then
            read -p "Continue anyway? (y/N): " -n 1 -r
            echo
            if [[ ! $REPLY =~ ^[Yy]$ ]]; then
                print_info "Operation cancelled"
                exit 0
            fi
        fi
    fi
    
    # Add all files if this is the first commit
    if ! git rev-parse HEAD &>/dev/null; then
        print_info "Adding all files to git"
        git add .
        if git diff --cached --quiet; then
            print_warning "No files to commit"
        else
            print_info "Creating initial commit"
            git commit -m "Initial commit before CARKIT2C0 patch"
        fi
    fi
    
    # Apply the patch
    print_info "Applying patch: $PATCH_PATH"
    if ! patch -p1 < "$PATCH_PATH"; then
        print_error "Failed to apply patch"
        print_info "You may need to resolve conflicts manually"
        exit 1
    fi
    
    # Verify patch was applied
    print_info "Verifying patch application"
    if ! git status --porcelain | grep -q .; then
        print_error "No changes detected after applying patch - patch may was empty or have failed silently "
        exit 1
    fi
    
    print_info "Kernel patch applied successfully"
}

# Function to build kernel
build_kernel() {
    print_step "Building kernel..."
    
    cd "$L4T_PATH/source/public"
    
    # Set up build environment
    local kernel_out="$PWD/kernel_out"
    export CROSS_COMPILE=aarch64-linux-gnu-
    
    print_warning "Kernel build can take 30+ minutes depending on your system."
    print_info "Output directory: $kernel_out"
    
    if [[ "$FORCE" != true ]]; then
        read -p "Start kernel build now? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            print_info "Kernel build cancelled"
            exit 0
        fi
    fi
    
    print_info "Starting kernel build (this may take a while)..."
    
    # Build kernel
    if ! ./nvbuild.sh -o "$kernel_out"; then
        print_error "Kernel build failed"
        exit 1
    fi
    
    print_info "Kernel build completed successfully"
    
    # Verify build outputs exist
    local image_file="$kernel_out/arch/arm64/boot/Image"
    local dtb_file="$kernel_out/arch/arm64/boot/dts/nvidia/tegra234-p3701-0005-p3737-0000.dtb"
    
    if [[ ! -f "$image_file" ]]; then
        print_error "Kernel Image not found: $image_file"
        exit 1
    fi
    
    if [[ ! -f "$dtb_file" ]]; then
        print_error "Device tree blob not found: $dtb_file"
        exit 1
    fi
    
    print_info "Build outputs verified"
}

# Function to copy build outputs
copy_build_outputs() {
    print_step "Copying build outputs..."
    
    local kernel_out="$L4T_PATH/source/public/kernel_out"
    local image_src="$kernel_out/arch/arm64/boot/Image"
    local dtb_src="$kernel_out/arch/arm64/boot/dts/nvidia/tegra234-p3701-0005-p3737-0000.dtb"
    
    local image_dst="$L4T_PATH/kernel/Image"
    local dtb_dst="$L4T_PATH/kernel/dtb/"
    
    # Create destination directory
    mkdir -p "$dtb_dst"
    
    # Backup existing files
    if [[ -f "$image_dst" ]]; then
        print_info "Backing up existing kernel Image"
        cp "$image_dst" "$image_dst.backup.$(date +%Y%m%d_%H%M%S)"
    fi
    
    local dtb_basename="$(basename "$dtb_src")"
    if [[ -f "$dtb_dst/$dtb_basename" ]]; then
        print_info "Backing up existing device tree blob"
        cp "$dtb_dst/$dtb_basename" "$dtb_dst/$dtb_basename.backup.$(date +%Y%m%d_%H%M%S)"
    fi
    
    # Copy files
    if ! cp "$image_src" "$image_dst"; then
        print_error "Failed to copy kernel Image"
        exit 1
    fi
    print_info "Copied kernel Image"
    
    if ! cp "$dtb_src" "$dtb_dst/"; then
        print_error "Failed to copy device tree blob"
        exit 1
    fi
    print_info "Copied device tree blob"
    
    print_info "Build outputs copied successfully"
}

# Function to flash the device
flash_device() {
    if [[ "$SKIP_FLASH" == true ]]; then
        print_warning "Skipping flash step as requested"
        return
    fi
    
    print_step "Flashing device..."
    
    cd "$L4T_PATH"
    
    if [[ "$FORCE" != true ]]; then
        print_warning "This will flash the Jetson device. Make sure it's connected and in recovery mode."
        read -p "Continue with flashing? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            print_info "Flashing skipped"
            SKIP_FLASH=true
            return
        fi
    fi
    
    print_info "Starting flash process..."
    if ! sudo ./flash.sh jetson-agx-orin-devkit mmcblk0p1; then
        print_error "Flashing failed"
        exit 1
    fi
    
    print_info "Device flashed successfully"
}

# Function to display summary
display_summary() {
    print_step "Installation Summary"
    echo
    echo "CARKIT2C0 GMSL kernel customization completed successfully!"
    echo
    echo "What was done:"
    echo "  ✓ Copied configuration files (pinmux and GPIO)"
    echo "  ✓ Applied kernel patch"
    echo "  ✓ Built custom kernel"
    echo "  ✓ Copied build outputs"
    if [[ "$SKIP_FLASH" != true ]]; then
        echo "  ✓ Flashed device"
    else
        echo "  - Skipped device flashing"
    fi
    echo
    echo "Next steps:"
    if [[ "$SKIP_FLASH" == true ]]; then
        echo "  - Flash the device manually: cd $L4T_PATH && sudo ./flash.sh jetson-agx-orin-devkit mmcblk0p1"
    fi
    echo "  - Verify customizations are active by checking relevant system files and GPIO states"
    echo "  - Test GMSL functionality"
    echo
}

# Main execution
main() {
    print_info "Starting CARKIT2C0 GMSL installation script"
    print_info "L4T Path: $L4T_PATH"
    print_info "Patch Path: $PATCH_PATH"
    print_info "Configuration Path: $CONFIG_DIR"
    echo
    
    if [[ "$FORCE" != true ]]; then
        read -p "Continue with installation? (y/N): " -n 1 -r
        echo
        if [[ ! $REPLY =~ ^[Yy]$ ]]; then
            print_info "Installation cancelled"
            exit 0
        fi
    fi
    
    check_prerequisites
    install_packages
    copy_config_files
    apply_patch
    build_kernel
    copy_build_outputs
    flash_device
    display_summary
    
    print_info "Installation completed successfully!"
}

# Trap to handle script interruption
trap 'print_error "Script interrupted"; exit 1' INT TERM

# Run main function
main "$@"