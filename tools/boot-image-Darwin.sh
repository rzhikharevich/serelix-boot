#!/bin/sh

set -e

if ! [ -e boot.img ]; then
	dd if=/dev/zero of=boot.img bs=42m count=1
fi

DEVICE=`hdiutil attach -nomount boot.img | head -n1 | awk '{print $1}'`

VOLUME_NAME=SRLXBOOT
MEDIA_ROOT=/Volumes/$VOLUME_NAME

if [ $? != 0 ]; then
	echo "Failed to attach boot.img."
	exit 1
fi

if ! [ -e $DEVICE ]; then
	echo "Successfully attached boot.img, but couldn't locate the device node."
	exit 1
fi

if ! diskutil mount ${DEVICE}s1; then
	diskutil eraseDisk fat32 $VOLUME_NAME $DEVICE
	if ! diskutil mount ${DEVICE}s1; then
		echo "Failed to format boot.img."
		exit 1
	fi
fi

echo 'Copying files...'

echo ' * /EFI/BOOT/BOOTX64.EFI'
mkdir -p $MEDIA_ROOT/EFI/BOOT
cp BOOTX64.EFI $MEDIA_ROOT/EFI/BOOT

mkdir -p $MEDIA_ROOT/srlxboot

echo ' * /srlxboot/alotware.awf'
cp src/alotware.awf $MEDIA_ROOT/srlxboot

echo ' * /srlxboot/boot.cfg'
cp boot.cfg $MEDIA_ROOT/srlxboot

diskutil umountDisk $DEVICE
hdiutil detach $DEVICE
