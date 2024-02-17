# Android Rooting Attack Lab
- Android devices do not allow their owners to have the root privilege on the device. The process of gaining the root privilege on Android devices is called `rooting`. Being able to root Android devices is a very useful skill for security experts

## Background Knowledge
### Background Knowledge About Rooting
- there are many reasons why people want to get root privilege. For example, uninstalling some pre-installed system apps that are useless most of the time, there are several ways to root an Android device
- *modifying Android from inside*
    - the first rooting approach is doing it from inside Android. Exploit vulnerabilities inside the kernel or daemons running with the root privilege.
    - In the past, there are many rooting exploits, such as `RageAgainstTheCage`, which took advantage of `RLIMIT_NPROC` max, the value specifying how many processes a given UID can run. This exploit first uses `adb shell` to run a shell on the device via the `adb` daemon. The UID of the shell will be 2000. The exploits then forks new processes until the number of processes have reached the limit and fork call will fail. At this point, the exploit kills the `adb` daemon, and then reconnect to it using `adb shell`. This causes the system to restart the `adb` daemon. When `adb` is started, it always has the root privilege, but it will drop the privilege to UID 2000 using `setuid(2000)`. Unfortunately, the UID 2000 has already used up its process quota, so the call will fail. The `adb` daemon fails to handle the failure correctly: instead of exiting, it keeps running with the root privilege. As a result, the `adb shell` command will give users a root shell. This vulnerability has been fixed after Android 2.2
    
- *modifying Android from outside*
    - directly modify Android's system files from outside. If a second OS is installed on this device, and we gain the root privilege for this OS, we can mount the partition used by Android system and modify any files. Most Android devices do have the second OS installed, called *recovery OS*, it is used for recovery purposes as well as updating OS
    - however, the recovery OS also has access control. Recovery OS is typically placed on devices by vendors, who use the OS to update Android, but not wanting users from running arbitrary commands. To enforece that, recovery OSes do not give users a shell prompt, preventing users from running arbitrary commands. Instead, they take a package provided from outside (either from users or downloaded from the Internet), the package contains the commands and files needed for updating the Android OS, this mechanism is called *Over-The-Air (OTA)* update. The package is called OTA package, which has a standard file structure
    - most recovery OSes only accept the packages made by the vendors to ensure that any update is approved by the vendors. This is enforced using digital signatures from the vendors. This protection becomes a roadblock for rooting, and we need to find ways to bypass this access control

- *reinstall recovery OS*
    - instead of bypassing the access control of the recovery OS, it is easier to replace it with another recovery OS that does not have such access controls. This new recovery OS is called custom recovery OS, which will not verify the signature, so we can provide our own OTA packages to it
    - however, there is another access control preventing us from replacing the recovery OS, and this time it is the bootloader
    - *Bootloader* is a low level code that loads an OS or some other system software after the computer is powered on. When a bootloader is "locked", it will simply load one of the OSes that is already installed on the device, leaving no chances for users to modify any of the pre-installed OSes. If a bootloader can be unlocked, we can add another option to install custom OS and replacing the existing ones, this process is referred to as *flashing custom OS*
    - most manufacturers do provide ways for users to unlock the bootloader, provided that by doing so, users will lose all of their data, as well as the warranties

### Background Knowledge About OTA
- OTA is a standard technique for devices to update Android OS. Since rooting also needs to update Android OS, OTA becomes a popular choice
- OTA package is just a zip file and its structure is as follows. Of particular interest to this lab is the `META-INF` folder, which includes signature and certificates of the package along with two very important files named `update-binary` and `updater-script`
    - `META-INF/com/google/android/update-binary`: this binary is executed by the recovery OS to apply OTA updates, it loads and executes `updater-script`
    - `META-INF/com/google/android/updater-script`: an installation script which is interpreted by `update-binary`. It is written using a script language called `Edify`, which describes the required action to be performed to apply updates
    ```
    Real OTA Package
    |------ META-INF
    |       |------ MANIFEST.MF
    |       |------ CERT.RSA
    |       |------ CERT.SF
    |       |------ com/
    |               |------ google/
    |                       |------ android/
    |                               |------ update-binary
    |                               |------ updater-script
    |------ arm/    --+
    |------ x86/    --|    all of these folders are optional, vendors 
    |------ system/ --|--> put necessary directory files which to be 
    |------ boot/   --|    used/copied/extracted during the update
    |------ ...     --+
    ```
- after signature verification on the OTA package, the recovery OS extracts the `update-binary` executable from the OTA package to the `/tmp` directory and runs it by passing three arguments as follows
    ```
    updatte-binary version output package
    *version: the version of recovery API
    *output: the command pipe that update-binary uses to communicate with recovery
    *package: the path of the OTA package
    
    An example: update-binary 3 stdout /sdcard/ota.zip
    ```
- On a successful execution of `updater-script`, the recovery OS copies the execution logs to the `/cache/recovery/` directory and reboots into the Android OS. Execution logs can be accessible from the Android OS after rebooting. This is how android system is updated by using OTA package

## Lab Setup
- we assume that the bootloader can be unlocked, and the stock recovery OS can be replaced. Replacing recovery OS in the VM is quite difficult, we use Ubuntu 16.04 to emulate what users can do using a custom recovery OS
- the Android VM in the lab is already rooted, the VM build is actually a `userdebug` build, and several doors were built in to the VM to allow access, real Android devices use `user` build, which do not have these doors. In this lab, we are not allowed to use these doors to get root access

## Task 1: Build A Simple OTA Package
- build a simple OTA package from scratch, here are our goals
    1. How to inject a program into the Android OS from the recovery OS
    2. How to get our injected program to run automatically, and with the root privilege
    3. How to write a program that can give us the root shell
- in this task, we focus on the first and second goal
- for sake of simplicity, try to create a dummy file in `/system` foler on Android VM, which requires the root privilege: `echo hello > /system/dummy`

- *step 1: write the update script*
    - `update-binary` file can be a binary executable, or just a simple script file (this requires the recovery OS to have the binary executable, e.g., `bash`, to execute the script file), we will use a shell script
    - purpose for `update-binary`
        1. inject `dummy.sh` program into the Android OS (we need to figure out where to place it, and how to set up its permissions)
        2. change the Android OS configuration file, so our `dummy.sh` program can be automatically executed with the root privilege when Android boots up, to achieve this, we will use one approach related to Linux (in the next task, we will use a different approach related to Android framework)
    - Android is built on top of the Linux OS, when it boots up, its underlying Linux boots up first, which conducts system initialization, including starting essential daemon processes. The booting procedure, using the root privilege, runs a file called `/system/etc/init.sh` for part of the initialization. So, if we can insert a command into `init.sh` file, we can run our `dummy.sh` with the root privilege
    - we need to do this through `update-binary` file, we will use the `sed` command, which is stream editor for filtering and transforming text. Find the "return 0" statement inside `init.sh`, and insert a command beofre that
        - `sed -i "/return 0/ i /android/system/xbin/dummy.sh" /android/system/etc/init.sh`
        - `-i`: edit files in place
        - `/return 0/`: match the line that has the content `return 0`
        - `i`: insert before the matching line
        - `/android/system/xbin/dummy.sh`: the content to be inserted
        - `/android/system/etc/init.sh`: the target file
- *step 2: build the OTA package*
    - put our files in their corresponding folders according to the file structure, we don't need to create files that are not needed for our task (e.g., the signature and optional files)
    - zip all files: `zip -r my_ota.zip ./`
    - we can use `unzip -l` to display the file structure
- *step 3: run the OTA package*
    - after building the OTA package, we can provide it to the recovery OS, which will run it automatically. However, this is how it works with a real recovery OS, in our lab setup, we use Ubuntu as our recovery OS, but it does not have the needed recovery functionality. Therefore, we have to emulate the recovery functionality
    - first, reboot the android VM, and hold left shift, then we can see 3 options, select `Ubuntu` to get in the recovery OS
    - on our Ubuntu 16.04 VM, `scp my_ota.zip seed@10.0.2.78:/tmp` (`10.0.2.78` is the IP address of the recovery OS)
    - on the recovery OS, we have to manually unpack the OTA package, go to folder `/tmp/my_ota/META-INF/com/google/android` folder to find the `update-binary` file, and run it
    - boot up the Android OS, and check whether `/system/dummy` exists or not
    - *important*: the shell program on Android is `/system/bin/sh`, if we use `#!/bin/bash` in `dummy.sh`, `init.sh` will not be able to run it

## Task 2: Inject Code via `app_process`
- in task `1`, we modify the `init.sh` file to get our injected program to run automatically with the root privilege. This initialization script is used by the underlying Linux OS. Once the Linux part is initialized, Android OS will bootstrap its runtime that is built on top of Linux. We would like to execute our injected program during this bootstrapping process
- when Android runtime bootstraps, it always runs a program called `app_process`, using the root privilege. This starts the `Zygote` daemon, whose mission is to launch applications, i.e., `Zygote` daemon is the parent of all app processes
- modify `app_process`, so in addition to launch the `Zygote` daemon, it also runs something of our choice to create a file `/system/dummy2`
- the following sample code is a wrapper for the original `app_process`, we will rename the original `app_process` binary to `app_process_original`, and call our wrapper program `app_process`, so it will first create `/system/dummy2`, and then run the original `app_process`
    ```
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>

    extern char** environ;

    int main(int argc, char** argv) {
        // Write the dummy file
        FILE* f = fopen("/system/dummy2", "w");
        if(f == NULL) {
            printf("Permission Denied\n");
            exit(EXIT_FAILURE);
        }
        fclose(f);

        // Launch the original binary
        char* cmd = "/system/bin/app_process_original";
        execve(cmd, argv, environ);

        // execve() returns only if it fails
        return EXIT_FAILURE;
    }
    ```
- *step 1: compile the code*
    - compile the code on SEED Ubuntu VM, because neither of the recovery OS or Android OS has the native code development environment installed. The *Native Development Kit (NDK)* has been installed on SEED Ubuntu VM. NDK is a set of tools that allow us to compile C and C++ code for Android OS, this type of code, called native code, can either be a stand-alone native program, or invoked by Java code in Android apps via *JNI (Java Native Interface)*. Our wrapper `app_process` program is a stand-alone native program, which needs to be compiled using NDK
    - to use NDK, we need to create 2 files, `Application.mk` and `Android.mk`, and place them in the same folder as the source code
        - The Application.mk file
            ```
            APP_ABI := x86
            APP_PLATFORM := android-22
            APP_STL := stlport_static
            APP_BUILD_SCRIPT := Android.mk
            ```
        - The Android.mk file
            ```
            LOCAL_PATH := $(call my-dir)
            include $(CLEAR_VARS)
            LOCAL_MODULE := my_app_process
            LOCAL_SRC_FILES := my_app_process.c
            include $(BUILD_EXECUTABLE)
            ```
    - run the following command to compile the code, if the compilation succeeds, we can find the binary file in the `./libs/x86` folder
        ```
        export NDK_PROJECT_PATH=.
        ndk-build NDK_APPLICATION_MK=./Application.mk
        ```
    - result
        ```
        $ ./compile.sh 
        Compile x86    : my_app_process <= my_app_process.c
        Executable     : my_app_process
        Install        : my_app_process => libs/x86/my_app_process
        ```

- *step 2: write the update script and build OTA package*
    - just like in task `1`, create the OST package, then unzip it in the recovery OS, and run `update-binary`
        1. copy the compiled binary code to the corresponding location (`/system/bin/`) inside Android
        2. rename the original `app_process` to `app_process_original`, and rename our binary as `app_process` (specifically, `app_process64`, as our Anroid VM is 64-bit)

## Task 3: Implement `SimpleSU` for Getting Root Shell
- now we know how to inject our code into the Android system and gain the root privilege, but we have not completely achieved our ultimate goal. We want to get a shell that runs with the root privilege, such a shell is called *root shell*
- the problem of the approaches in task `1` and `2` is that if we start a interactive shell (which will not terminate unless user type an exit command), this will stop the system booting process, so the OS will never be able to complete its booting sequence
- if we were on a typical Linux system, we can easily solve the problem by using the `chmod` command to turn on the Set-UID bit of any shell program (e.g., `/bin/bash`) that is owned by the root. Unfortunately, for security reasons, Android has removed the Set-UID mechanism from its underlying Linux OS since version 4.3 (API Level 18)
- antoher approach is to start a root daemon during the booting process, and then use this daemon to help users get a root shell. This is the approach used by some of the popular rooting OTA packages, such as `SuperSU` developed by Chainfire. We will write such a daemon
- the idea is simple, when users want to get a root shell, they run a client program, which sends a request to the root daemon, then the daemon starts a shell process, and give it to the client. The tricky part is how to let the user control the shell process that is created by the daemon
- users need to be able to control the standard input and output devices of the shell process, however, the shell process inherits its standard I/O devices from its parent process owned by root. We can find a way to let the client program to control these devices, or we can do it in a different way by giving the client program's IP devices to the shell process, so they also become the I/O devices of the shell program
- we need to have 2 essential pieces of knowledge
    1. how to send the standard I/O devices (file descriptors) to another process
    2. once a process receives the file descriptors, how it can use them as its IO devices

### Background
- *file descriptors*
    - each process in Linux systems typically has 3 associated I/O devices: standard input (`STDIN`), standard output (`STDOUT`), and standard error devices (`STDERR`)
    - processes access these devices through the standard POSIX application programming interface that uses file descriptors: `0`: `STDIN`; `1`: `STDOUT`; `2`: `STDERR`, in this task, we need to pass file descriptors from one process to another
    - file descriptors can be passed either via inheritance or explicit sending, when a parent process creates a child process using `fork()`, all the partent's file descriptors are inherited by the child process. Beyond this stage, if parent process wants to share a new file descriptor with its children, or if two unrelated processes want to share the same file descriptor, they have to explicitly send the fd, which can be achieved using the Unix Domain Socket
    - fd can be redirected. The system call `dup2(int dest, int src)` can redirect the `src` fd to the `dest` one, so the fd entry at index `src` actually points to the entry at `dest`
- *creating new process*
    - `pid_t pid=fork()`, in parent process `pid!=0` (it will be parent process ID), in child process `pid==0`

- *passing the file descriptor*
    - each of the client and server has its own standard I/O FDs `0`, `1`, `2`, denote them as `C_IN`, `C_OUT`, `C_ERR`, `S_IN`, `S_OUT`, `S_ERR`
    - how the client and the server work together to help the client get the root privilege
        1. the client connects to the server using the Unix Domain Socket
        2. Upon receiving the request, the server forks a child process and runs a root shell. The child process inherits all the standard I/O FDs from the parent
        3. the client sends its FDs 0, 1, and 2 to the server's child process using the Unix Domain Socket, they will be saved in the table indexes 4, 5, and 6, respectively
        4. the child process redirects its FDs 0, 1, 2 to the FDs received from the client, resulting in FDs 4, 5, and 6 being used as the standard input, output, and error devices. Now, the client and the server's child process share the same I/O devices
### The Task
- the source code is provided, compile them using NDK, and build an OTA package
    - use the approach in task `1`, place `/system/xbin/mydaemon` before `return 0` in `/system/etc/init.sh`
    - after getting into the Android OS, run `/system/xbin/mysu`
        ```
        $ /system/xbin/mysu
        ...
        start to connect to daemon
        sending file descriptor
        STDIN 0
        STDOUT 1
        STDERR 2
        2
        ...
        # id
        uid=0(root) gid=0(root) groups=0(root) context=u:r:init:s0
        ```
- check the standard I/O devices of the client and the shell process (the file descriptors are in `/proc/<PID>/fd/`)
    ```
    # ps | grep my
    root    1071  1     5064  364   0 0000000000 S /system/xbin/mydaemon
    root    1084  1071  0     0     0 0000000000 Z myademon
    root    1508  1071  0     0     0 0000000000 Z myademon
    root    1855  1071  0     0     0 0000000000 Z myademon
    u0_a36  3143  3096  5064  1888  0 0000000000 S ./mysu
    ```
    ```
    # ps | grep sh
    ...
    root    3812  1071  0     0     0 0000000000 S /system/bin/sh
    ...
    ```
    - we can see a root shells generated by `mydaemon` process (`PID 1071`). Check `ls -la /proc/3812/fd`, and the user program's FDs `ls -la /proc/3143/fd`, we can find that both of teir standard IO devices FDs (0, 1, and 2) are pointed to `/dev/pts/0`
- actions
    - server launches the original `app_process` binary: in `main()` function
        ```
        argv[0] = APP_PROCESS;
        execve(argv[0], argv, environ);
        ```
    - client sends its FDs: in `connect_daemon()` function
        ```
        send_fd(socket, STDIN_FILENO);      //STDIN_FILENO = 0
        send_fd(socket, STDOUT_FILENO);     //STDOUT_FILENO = 1
        send_fd(socket, STDERR_FILENO);     //STDERR_FILENO = 2
        ```
    - server forks to a child process: in `main()` function
        ```
        pid_t pid = fork();
        if (pid == 0) {
            //initialize the daemon if not running
            if (!detect_daemon()) run_daemon(argv);
        }
        ```
    - child process receives client's FDs: in `child_process()` function
        ```
        int client_in = recv_fd(socket);
        int client_out = recv_fd(socket);
        int client_err = recv_fd(socket);
        ```
    - child process redirects its standard I/O FDs: in `child_process()` function
        ```
        dup2(client_in, STDIN_FILENO);      //STDIN_FILENO = 0
        dup2(client_out, STDOUT_FILENO);    //STDOUT_FILENO = 1
        dup2(client_err, STDERR_FILENO);    //STDERR_FILENO = 2
        ```
    - child process launches a root shell: in `child_process()` function
        ```
        char* env[] = {SHELL_ENV, PATH_ENV, NULL};
        char* shell[] = {DEFAULT_SHELL, NULL};
        execve(shell[0], shell, env);
        ```
