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

.PHONY: bikeshed_fs
bikeshed_fs: build/debugfs_commands.debugfs
	dd if=/dev/zero of=bikeshed_fs bs=1K count=$(RAMDISK_SIZE_KiB)
	mke2fs -O^resize_inode,^sparse_super -m0 -F -L bikeshed bikeshed_fs
	ln -f bikeshed_fs build/bikeshed_fs
	make -C build/ populate_ramdisk

# Run the OS in qemu
qemu: 
	CFLAGS=-DQEMU_SERIAL $(MAKE) usb.image
	qemu-system-x86_64 -cpu core2duo -drive file=usb.image,format=raw,cyls=200,heads=16,secs=63 -serial stdio -net user -net nic,model=i82559er

walter:	
	CFLAGS=-DQEMU_SERIAL $(MAKE) usb.image
	kvm -cpu core2duo -drive file=usb.image,format=raw,cyls=200,heads=16,secs=63 -serial /dev/pts/1 -monitor stdio -net user -net nic,model=i82559er -vga std

mit: 
	CFLAGS=-DQEMU_SERIAL $(MAKE) usb.image
	~/qemu/bin/qemu -m 1024 -cpu core2duo -drive file=usb.image,format=raw,cyls=200,heads=16,secs=63 -serial /dev/pts/1 -monitor stdio -net user -net nic,model=i82559er
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

realclean: clean
	$(RM) usb.image floppy.image
	$(RM) image.dat
	$(RM) build.image
	$(RM) bikeshed_fs
	$(RM) build/bikeshed_fs
