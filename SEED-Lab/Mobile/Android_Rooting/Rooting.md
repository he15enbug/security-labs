# Android Rooting Attack Lab
- Android devices do not allow their owners to have the root privilege on the device. The process of gaining the root privilege on Android devices is called `rooting`. Being able to root Android devices is a very useful skill for security experts

## Background Knowledge
### Background Knowledge About Rooting
- there are many reasons why people want to get root privilege. For example, uninstalling some pre-installed system apps that are useless most of the time, there are several ways to root an Android device
- *modifying Android from inside*: the first rooting approach is doing it from inside Android. Exploit vulnerabilities inside the kernel or daemons running with the root privilege.
    - In the past, there are many rooting exploits, such as `RageAgainstTheCage`, which took advantage of `RLIMIT_NPROC` max, the value specifying how many processes a given UID can run. This exploit first uses `adb shell` to run a shell on the device via the `adb` daemon. The UID of the shell will be 2000. The exploits then forks new processes until the number of processes have reached the limit and fork call will fail. At this point, the exploit kills the `adb` daemon, and then reconnect to it using `adb shell`. This causes the system to restart the `adb` daemon. When `adb` is started, it always has the root privilege, but it will drop the privilege to UID 2000 using `setuid(2000)`. Unfortunately, the UID 2000 has already used up its process quota, so the call will fail. The `adb` daemon fails to handle the failure correctly: instead of exiting, it keeps running with the root privilege. As a result, the `adb shell` command will give users a root shell. This vulnerability has been fixed after Android 2.2
    
    - *modifying Android from outside*: directly modify Android's system files from outside. If a second OS is installed on this device, and we gain the root privilege for this OS, we can mount the partition used by Android system and modify any files. Most Android devices do have the second OS installed, called *recoveryOS*, it is used for recovery purposes as well as updating OS

### Background Knowledge About OTA
- OTA is a standard technique for devices to update Android OS. Since rooting also needs to update Android OS, OTA becomes a popular choice
- OTA package is just a zip file and its structure is as follows
    ```
    
    ```