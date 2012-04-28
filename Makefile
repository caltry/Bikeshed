
BUILDIMAGE = build/BuildImage

#
# Targets for remaking bootable image of the program
#
# Default target:  usb.image
#

usb.image: src/boot/bootstrap.b src/prog.b bikeshed_fs $(BUILDIMAGE)
	$(BUILDIMAGE) -d usb -o usb.image -b src/boot/bootstrap.b src/prog.b 0x10000 bikeshed_fs 0x80000

floppy.image: src/boot/bootstrap.b src/prog.b bikeshed_fs $(BUILDIMAGE)
	$(BUILDIMAGE) -d floppy -o floppy.image -b src/boot/bootstrap.b src/prog.b 0x10000 bikeshed_fs 0x80000

#
# Additional dependencies to make sure that bootstrap.b and prog.b are made
#

src/boot/bootstrap.b: build_src
src/prog.b: build_src

.PHONY: build_src
build_src:
	$(MAKE) -C src

#
# Targets for copying bootable image onto boot devices
#

floppy:	floppy.image
	dd if=floppy.image of=/dev/fd0

usb:	usb.image
	dd if=usb.image of=/local/devices/disk

bikeshed_fs:
	dd if=/dev/zero of=bikeshed_fs bs=1K count=120
	mke2fs -O^resize_inode,^sparse_super -m0 -F -L bikeshed bikeshed_fs

# Run the OS in qemu
qemu:	usb.image
	qemu-system-x86_64 usb.image -serial stdio

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
