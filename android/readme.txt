
apt-get update
apt install -y openjdk-8-jdk ant android-sdk-platform-tools-common wget unzip make
# libsdl2-dev
# android-sdk
cd /tmp
wget https://dl.google.com/android/repository/android-ndk-r10e-linux-x86_64.zip

wget https://www.libsdl.org/release/SDL2-2.0.10.zip
unzip SDL2-2.0.10.zip 
mv SDL2-2.0.10 /usr/src/SDL2

mkdir /usr/lib/android-sdk
mkdir /usr/lib/android-sdk/cmdline-tools
export ANDROID_HOME="/usr/lib/android-sdk"
PATH="$ANDROID_HOME/cmdline-tools/tools/bin:$PATH"
unzip /src/android/commandlinetools-linux-6514223_latest.zip 

sdkmanager "build-tools;25.0.2"  
sdkmanager ndk-bundle
sdkmanager platform-tools

yes | sdkmanager --licenses


# export ANDROID_NDK_HOME="/tmp/android-ndk-r10e"  


# PATH="/tmp/android-ndk-r10e:$PATH"


cd "/usr/src/SDL2/build-scripts"
./androidbuild.sh org.libsdl.testgles ../test/testgles.c
cd /usr/src/SDL2/build/org.libsdl.testgles
./gradlew installDebug


cd /usr/src/SDL2/build-scripts
./androidbuild.sh be.kyuran.crocods /src/main/*.c /src/crocods-core/*.c /src/crocods-core/iniparser/*.c /src/main/*.h /src/crocods-core/*.h /src/crocods-core/iniparser/*.h /src/main/headers/*.h
cd /usr/src/SDL2/build/be.kyuran.crocods
./gradlew installDebug




#######

Debugging

socat tcp-listen:5556,reuseaddr,fork tcp:192.168.1.87:5555
