# <- this is for comment / total file size must be less than 4KB
tftp 0x21000000 rootfs.sqfs
nand erase.part rootfs
nand write.e 0x21000000 rootfs ${filesize}
% <- this is end of file symbol
