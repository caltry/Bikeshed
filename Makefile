include src/fs_defs.mk

BUILDIMAGE = build/BuildImage
FANCYCAT   = build/FancyCat

#
# Targets for remaking bootable image of the program
#
# Default target:  usb.image
#

usb.image: src/boot/bootstrap.b src/prog.b $(BUILDIMAGE) $(FANCYCAT) bikeshed_fs
	$(BUILDIMAGE) -d usb -o build.image -b src/boot/bootstrap.b dummy 0x10000
	$(FANCYCAT) 0x100000 src/prog.b 0x200000 src/real.b $(RAMDISK_PHYS_LOCATION) bikeshed_fs
	/bin/cat build.image image.dat > usb.image

floppy.image: src/boot/bootstrap.b src/prog.b $(BUILDIMAGE) $(FANCYCAT) bikeshed_fs
	$(BUILDIMAGE) -d floppy -o build.image -b src/boot/bootstrap.b dummy 0x10000
	$(FANCYCAT) 0x100000 src/prog.b 0x200000 src/real.b $(RAMDISK_PHYS_LOCATION) bikeshed_fs
	/bin/cat build.image image.dat > floppy.image

#
# Additional dependencies to make sure that bootstrap.b and prog.b are made
#

src/boot/bootstrap.b: build_src
src/prog.b: build_src

.PHONY: build_src
build_src:
	$(MAKE) -C src

build/%: FORCE
	make -C $(dir $@) $(notdir $@)
FORCE:

#
# Targets for copying bootable image onto boot devices
#

floppy:	floppy.image
	dd if=floppy.image of=/dev/fd0

usb:	usb.image
	dd if=usb.image of=/local/devices/disk

bikeshed_fs:
	dd if=/dev/zero of=bikeshed_fs bs=1K count=$(RAMDISK_SIZE_KiB)
	mke2fs -O^resize_inode,^sparse_super -m0 -F -L bikeshed bikeshed_fs

# Run the OS in qemu
qemu:	usb.image
	qemu-system-x86_64 usb.image -serial stdio

walter:	usb.image
	kvm usb.image -serial /dev/pts/1 -monitor stdio
#
# Special rule for creating the modification and offset programs
#
# These are required because we don't want to use the same options
# as for the standalone binaries.
#

$(BUILDIMAGE):
	$(MAKE) -C build

clean:
	$(MAKE) -C src clean
	$(MAKE) -C build clean
	$(RM) bikeshed_fs

realclean: clean
	$(RM) usb.image floppy.image
	$(RM) image.dat
	$(RM) build.image
