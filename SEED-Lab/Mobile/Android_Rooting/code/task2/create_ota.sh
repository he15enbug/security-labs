#!/bin/bash

mkdir -p my_ota/META-INF/com/google/android
touch my_ota/META-INF/com/google/android/updater-script

# create update-binary and write code modify init.sh
touch my_ota/META-INF/com/google/android/update-binary
cp libs/x86/my_app_process my_ota/META-INF/com/google/android/my_app_process

echo "#!""/bin/bash" >> my_ota/META-INF/com/google/android/update-binary
echo "" >> my_ota/META-INF/com/google/android/update-binary

# replace the original app_process64
echo "mv /android/system/bin/app_process64 /android/system/bin/app_process_original" >> my_ota/META-INF/com/google/android/update-binary
echo "cp my_app_process /android/system/bin/app_process64" >> my_ota/META-INF/com/google/android/update-binary

# chmod a+x /android/system/bin/app_process64 is not necessary
# echo "chmod a+x /android/system/bin/app_process64" >> my_ota/META-INF/com/google/android/update-binary

chmod a+x my_ota/META-INF/com/google/android/update-binary

# zip everything
cd my_ota
zip -r my_ota.zip ./
