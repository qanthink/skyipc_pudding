############################ 说明 ###############################
#
#	作用：主要用于配置编译的toolChain
#
#################################################################

#设置toolchain
#SET(tools "uclibc")
#SET(tools "glibc4.8.3")
SET(tools "glibc9.1.0")

#设置编译器路径
if(${tools} STREQUAL "uclibc")########设置uclibc的路径  
message("ToolChain: uclibc")#打印信息

SET(ToolPath "/home/linux/tools/toolchain/arm-buildroot-linux-uclibcgnueabihf-4.9.4-uclibc-1.0.31/bin")
SET(CMAKE_C_COMPILER  	${ToolPath}/arm-buildroot-linux-uclibcgnueabihf-gcc)
SET(CMAKE_CXX_COMPILER 	${ToolPath}/arm-buildroot-linux-uclibcgnueabihf-g++)
SET(CMAKE_STRIP 		${ToolPath}/arm-buildroot-linux-uclibcgnueabihf-strip)

elseif(${tools} STREQUAL "glibc4.8.3")########设置glibc的路径
message("ToolChain: glibc4.8.3")#打印信息

SET(ToolPath "/home/linux/tools/toolchain/arm-linux-gnueabihf-4.8.3-201404/bin")
SET(CMAKE_C_COMPILER  	${ToolPath}/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER 	${ToolPath}/arm-linux-gnueabihf-g++)
SET(CMAKE_STRIP 		${ToolPath}/arm-linux-gnueabihf-strip)

elseif(${tools} STREQUAL "glibc9.1.0")########设置glibc的路径
message("ToolChain: glibc9.1.0")#打印信息

SET(ToolPath "/home/linux/tools/toolchain/gcc-sigmastar-9.1.0-2020.07-x86_64_arm-linux-gnueabihf/bin")
SET(CMAKE_C_COMPILER  	${ToolPath}/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER 	${ToolPath}/arm-linux-gnueabihf-g++)
SET(CMAKE_STRIP 		${ToolPath}/arm-linux-gnueabihf-strip)

endif()

