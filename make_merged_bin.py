"""Merge the c3_mini build into one binary that flashes at offset 0x0.

Usage: py make_merged_bin.py <path to tool-esptoolpy\\esptool.py>
"""
import pathlib
import subprocess
import sys

HERE = pathlib.Path(__file__).resolve().parent
BUILD = HERE / ".pio" / "build" / "c3_mini"
BOOT_APP0 = pathlib.Path.home() / ".platformio/packages/framework-arduinoespressif32/tools/partitions/boot_app0.bin"
OUTPUT = HERE / "dist" / "c3-mini-deepseek-peak-light.merged.bin"

OUTPUT.parent.mkdir(exist_ok=True)
subprocess.run(
    [
        sys.executable, sys.argv[1], "--chip", "esp32c3", "merge_bin",
        "-o", str(OUTPUT),
        "--flash_mode", "dio", "--flash_freq", "80m", "--flash_size", "4MB",
        "0x0", str(BUILD / "bootloader.bin"),
        "0x8000", str(BUILD / "partitions.bin"),
        "0xe000", str(BOOT_APP0),
        "0x10000", str(BUILD / "firmware.bin"),
    ],
    check=True,
)
print(OUTPUT, OUTPUT.stat().st_size)
