#NDK目录 r17c是最后一个支持gcc的ndk
export NDK_HOME=~/Godinsec_Project/NDK/android-ndk-r15c
#生成交叉编译链工具
toolchain=${NDK_HOME}/build/tools/make-standalone-toolchain.sh
#生成交叉编译链保存在当前目录子文件夹android-toolchain
install_root=`pwd`/android-toolchain

#生成32位库最低支持到android4.3，64位库最低支持到android5.0 最新版的ffmpeg，x264需要最低 android-23 就是 android 6.0 因为cabs()等函数。
platforms=(
  "android-23"
  "android-23"
  "android-23"
  "android-23"
  "android-23"
)
#支持以下5种cpu框架
archs=(
  "arm"
  "arm"
  "arm64"
  "x86"
  "x86_64"
)
#cpu型号
abis=(
  "armeabi"
  "armeabi-v7a"
  "arm64-v8a"
  "x86"
  "x86_64"
)
echo $NDK_HOME
echo "安装在目录:$install_root"

num=${#abis[@]}

for ((i=0; i<num; i++))
do
   bash $toolchain --arch=${archs[i]} --platform=${platforms[i]} --abis=${abis[i]} --install-dir=$install_root/${abis[i]}
done
