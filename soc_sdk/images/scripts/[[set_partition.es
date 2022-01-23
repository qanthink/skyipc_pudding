# <- this is for comment / total file size must be less than 4KB
mtdparts del CIS
setenv mtdparts $(mtdparts),0x500000(KERNEL),0x500000(RECOVERY),0x600000(rootfs),0x60000(MISC),-(UBI)
saveenv
nand erase.part UBI
ubi part UBI
ubi create miservice 0xA00000
 ubi create customer 0x5000000

% <- this is end of file symbol
