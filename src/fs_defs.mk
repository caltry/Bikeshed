# Location of our ramdisk in memory. We'll need to know both the physical
# and virtual addresses when we go above the 1MiB line.
RAMDISK_VIRT_LOCATION := 0xE0000000
RAMDISK_PHYS_LOCATION := 0x400000
RAMDISK_SIZE_KiB := 120
