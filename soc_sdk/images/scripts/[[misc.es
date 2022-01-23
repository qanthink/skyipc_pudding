# <- this is for comment / total file size must be less than 4KB
tftp 0x21000000 misc.lfs
nand erase.part MISC
nand write.e 0x21000000 MISC ${filesize}
% <- this is end of file symbol
