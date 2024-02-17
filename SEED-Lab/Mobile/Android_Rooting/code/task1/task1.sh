#!/bin/bash

mkdir -p my_ota/META-INF
touch my_ota/META-INF/MANIFEST.MF
# touch my_ota/CERT.RSA
# touch my_ota/CERT.SF
mkdir -p my_ota/META-INF/com/google/android
touch my_ota/META-INF/com/google/android/updater-script

# create update-binary and write code modify init.sh
touch my_ota/META-INF/com/google/android/update-binary
echo "#!""/bin/bash" >> my_ota/META-INF/com/google/android/update-binary
echo "" >> my_ota/META-INF/com/google/android/update-binary

echo "cp dummy.sh /android/system/xbin/dummy.sh" >> my_ota/META-INF/com/google/android/update-binary
echo "chmod a+x /android/system/xbin/dummy.sh" >> my_ota/META-INF/com/google/android/update-binary

echo "sed -i \"/return 0/ i /system/xbin/dummy.sh\" /android/system/etc/init.sh" >> my_ota/META-INF/com/google/android/update-binary
chmod a+x my_ota/META-INF/com/google/android/update-binary

# create dummy.sh, which will create a file /system/dummy
touch my_ota/META-INF/com/google/android/dummy.sh
# in Android OS, the shell program is in /system/bin/
echo "#!""/system/bin/sh" >> my_ota/META-INF/com/google/android/dummy.sh
echo "" >> my_ota/META-INF/com/google/android/dummy.sh
echo "echo hello > /system/dummy" >> my_ota/META-INF/com/google/android/dummy.sh

# zip everything
cd my_ota
zip -r my_ota.zip ./
