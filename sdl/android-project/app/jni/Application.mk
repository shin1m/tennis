NDK_TOOLCHAIN_VERSION := 5

# Uncomment this if you're using STL in your project
# See CPLUSPLUS-SUPPORT.html in the NDK documentation for more information
# APP_STL := stlport_static
APP_STL := gnustl_static 

#APP_ABI := armeabi-v7a arm64-v8a x86 x86_64
APP_ABI := armeabi-v7a x86

# Min runtime API level
APP_PLATFORM=android-14
