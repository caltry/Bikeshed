
BUILDIMAGE = host/build/BuildImage

#
# Targets for remaking bootable image of the program
#
# Default target:  usb.image
#

usb.image: target/boot/bootstrap.b target/prog.b BuildImage
	$(BUILDIMAGE) -d usb -o usb.image -b target/boot/bootstrap.b target/prog.b 0x10000

floppy.image: target/boot/bootstrap.b target/prog.b BuildImage
	$(BUILDIMAGE) -d floppy -o floppy.image -b target/boot/bootstrap.b target/prog.b 0x10000

#
# Additional dependencies to make sure that bootstrap.b and prog.b are made
#

target/boot/bootstrap.b: build_target
target/prog.b: build_target

.PHONY: build_target
build_target:
	$(MAKE) -C target

#
# Targets for copying bootable image onto boot devices
#

floppy:	floppy.image
	dd if=floppy.image of=/dev/fd0

usb:	usb.image
	dd if=usb.image of=/local/devices/disk

# Run the OS in qemu
qemu:	usb.image
	qemu-system-x86_64 usb.image -serial stdio

#
# Special rule for creating the modification and offset programs
#
# These are required because we don't want to use the same options
# as for the standalone binaries.
#

.PHONY: BuildImage
BuildImage:
	$(MAKE) -C host/build

clean:
	$(MAKE) -C target clean
	$(MAKE) -C host clean

realclean: clean
	$(RM) usb.image floppy.image
