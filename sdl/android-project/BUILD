cd sdl/android-project

zcat .../SDL2-2.0.*.tar.gz | tar xvf -
zcat .../SDL2_image-2.0.*.tar.gz | tar xvf -
zcat .../SDL2_ttf-2.0.*.tar.gz | tar xvf -
zcat .../SDL2_mixer-2.0.*.tar.gz | tar xvf -
mkdir libxml2
cd libxml2
	unzip .../platform_external_libxml2-master.zip

cd app/jni
	ln -s ../../SDL2-2.0.* SDL2
	ln -s ../../SDL2_image-2.0.* SDL2_image
	ln -s ../../SDL2_ttf-2.0.* SDL2_ttf
	ln -s ../../SDL2_mixer-2.0.* SDL2_mixer
	vi SDL2_mixer/Android.mk
		LOCAL_CFLAGS := -DMUSIC_WAV
	mkdir xml2
		ln -s ../../../libxml2/include

mkdir app/libs
cd app/libs
	cp -a .../crystax-ndk-10.*.*/sources/crystax/libs/{armeabi-v7a,x86} .

vi local.properties
	sdk.dir=.../android-sdk
	ndk.dir=.../crystax-sdk-10.*.*

./gradlew build
