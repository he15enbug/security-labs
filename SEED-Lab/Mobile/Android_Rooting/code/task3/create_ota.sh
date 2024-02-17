#!/bin/bash

mkdir -p my_ota/META-INF/com/google/android

# create update-binary and write code modify init.sh
touch my_ota/META-INF/com/google/android/update-binary
cp mysu/libs/x86/mysu my_ota/META-INF/com/google/android/mysu
cp mydaemon/libs/x86/mydaemon my_ota/META-INF/com/google/android/mydaemon

echo "#!""/bin/bash" >> my_ota/META-INF/com/google/android/update-binary
echo "" >> my_ota/META-INF/com/google/android/update-binary

# add code in init.sh to run mydaemon
echo "cp mysu /android/system/xbin/mysu" >> my_ota/META-INF/com/google/android/update-binary
echo "cp mydaemon /android/system/xbin/mydaemon" >> my_ota/META-INF/com/google/android/update-binary
echo "sed -i \"/return 0/ i /system/xbin/mydaemon\" /android/system/etc/init.sh" >> my_ota/META-INF/com/google/android/update-binary

chmod a+x my_ota/META-INF/com/google/android/update-binary

# zip everything
cd my_ota
zip -r my_ota.zip ./
