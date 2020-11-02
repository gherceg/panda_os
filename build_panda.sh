./create_initrd first.txt first.txt second.txt second.txt
cd src
make clean
make
cd ..
cp src/kernel iso/boot/kernel
genisoimage -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -A panda -input-charset utf8 -quiet -boot-info-table -o panda.iso iso