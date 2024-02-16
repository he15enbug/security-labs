# Android Repackaging Attack Lab
- repackaging attack is a very common type of attacks on Android devices. In such an attack, attackers modify a popular app downloaded from app markets, reverse engineer the app, add some malicious payloads, and then upload the modified app to app markets. User s can be easily fooled, because it is hard to notice the difference between the modified app and the original one
- what makes repackaging attack easy is that Android apps' binary code can be easily reverse engineered, due to the lack of protection on binaries

-*lab environment*:
    - the lab requires two virtual machines
        - SEED Android 7.1 VM
        - SEED Ubuntu 16.04 VM

## Task 1: Obtain An Android App (APK file) and Install It
- use the provided [`RepackagingLab.apk`](https://seedsecuritylabs.org/Labs_20.04/Mobile/Android_Repackaging/files/RepackagingLab.apk.zip)
- some other apps may not run on the VM (they will crash), one reason is that the app may have native code. Native code compiled for a real Android device has binary code for ARM processors, while our Android VM runs on x86 processors
- install the host app. We will do it using `adb` tool from the Ubuntu VM. First, we need to find the IP address of the Android VM. Run `ifconfig` in the Android Terminal app (I get `10.0.2.4`)
    ```
    // on Ubuntu VM
    $ adb connect 10.0.2.4
    * daemon not running. starting it now on port 5037 *
    * daemon started successfully *
    $ adb install RepackagingLab.apk
    2052 KB/s (1421095 bytes in 0.676s)
    Success
    ```
## Task 2: Disassemble Android App
- to launch the repackaging attack on an app, we need to modify the app
- it is not easy to modify the APK file, because the code in the APK file contains *Dalvik bytecode* (dex format), which is not meant for human to read, and we have to convert the bytecode into something that is human readable
- the most common human readable format for Dalvik bytecode is known as *Smali*. The names "Smali" and "Baksmali" are the Icelandic equivalents of "assembler" and "disassembler", respectively
- use a tools called *APKTool* to disassemble dex code (`classes.dex`) to smali code. APKTool is a very powerful reverse engineering tool for Android apps. It is useful to decode and rebuild Android apps, feed the entire APK file to the tool like the following:
    ```
    $ apktool d RepackagingLab.apk
    I: Using Apktool 2.2.2 on RepackagingLab.apk
    ...
    ```
- APK file is just a zip file, which contains `classes.dex` (compiled java source code, called Dalvik bytecode), `resources.arsc` (resource files), `AndroidManifest.xml`, etc. APKTool basically unzips the APK file, and decode its contents. For the resource files, not much needs to be done. For the Dalvik bytecode `classes.dex`, it is disassembled into smali code, APKTool places the output files into a created folder with a name that is the same as the name of the APK file. The typical folder structure of APK file after disassembly is shown as follows
    ```
    Andorid App Package
    |-------------- original
    |               |---------- META-INF
    |                           (data to ensure the integrity of 
    |                            the package and system security)
    |
    |-------------- res (resource files)
    |               |--------- anim
    |               |--------- color
    |               |--------- drawable
    |               |--------- layout
    |               |--------- values
    |
    |-------------- smali
    |               |---------- android 
    |               |           (android support library code decompiled)
    |               |---------- com
    |                           (app specific code decompiled in smali)
    |
    +-------------- AndroidManifest.xml
                    (information about app components,
                     name, version, access rights, also
                     references to library files and other)
    ```
- we will inject some malicious code into the `smali` folder, and then assemble everything together into a new APK package

## Task 3: Inject Malicious Code
- there are many ways to inject malicious code into the target app's smali code. One approach is to directly modify some existing smali file, so we can add malicious logic into it. Another approach, which is much easier, is to add a completely new component to the app, which is independent from the existing app, so it does not affect the app's behavior. Since each independent component can be placed in a separate smali file, using this approach, we just need to create a new smali file
- there are 4 types of components in Android apps, `activity`, `service`, `broadcast receiver`, and `content provider`. The first 2 components are most commonly used by apps, while the other 2 are not so common. We can add any of these components in the target app, but the most important problem for the attack is to find a way to trigger the malicious code without being noticed by users. Although solutions to the problem exists for all these components, the easiest one is broadcast receiver, which can be triggered by broadcasts sent by the system. For example, when the system time is set, a `TIME_SET` broadcast will be sent out; after the system reboots, a `BOOT_COMPLETED` broadcast will be sent out. We can write a broadcast receiver that listens to one of these broadcasts, so the malicious code will be automatically triggered by those events
- write a broadcast receiver in Java, and build an APK file, then run APKTool to disassemble the APK file to get the smali code for the broadcast receiver
- we need to register our broadcast receiver to the system, this is done by adding some information to the target app's `AndroidManifest.xml` file, which can also be found from the folder created by APKTool
    ```
    <manifest...>
        ...
        <uses-permission android:name="android.permission.READ_CONTACTS" />
        <uses-permission android:name="android.permission.WRITE_CONTACTS" />
        ...
        <application>
            ...
            <receiver android:name="com.MaliciousCode">
                <intent-filter>
                    <action android:name="android.intent.action.TIME_SET"/>
                </intent-filter>
            </receiver>
        </application>
    </manifest>
    ```

## Task 4: Repack Android App with Malicious Code
- after we have inserted the malicious smali code, we can reassemble everything together, and build a single APK file
- *step 1: rebuild APK*: `$ apktool b RepackagingLab`, this will generate a new APK file, by default, it will be saved in the `RepackagingLab/dist` directory
- *step 2: sign the APK file*: Android requires all apps to be digitally signed before they can be installed. This requires each APK to have a digital signature and a public key certificate. The certificate and signature helps Android to identify the author of an app. From security perspective, the certificate needs to be signed by a *Certificate Authority, CA*. Getting a certificate from an accepted CA is usually not free, so Android allows developers to sign their certificates using their own private key, i.e., the certificate is self signed. The purpose of such self-signed certificates is meant for apps to be albe to run on Android devices, not for security. Developers can put any name they want in the certificate, regardless of whether the name is legally owned by others or not. This entirely defeats the purpose of certificate and signature. Google Play Store does some name verification before accepting an app, but other third-party app markets do not always conduct such a verification. We will use a self-signed certificate, the entire process consists of 2 steps
    1. generate a public and private key pair using `keytool` command. The tool will prompt user for a password, which is used to protect the keystore. It also asks users to provide some additional information for the key. It then generates a pulibc/private key pair, and store that in a keystore file `mykey.keystore` (specified at the command line). The keystore can store multiple keys, each identified by an alias name
        - `$ keytool -alias Heisenbug -genkey -v -keystore mykey.keystore`
    2. use `jarsigner` to sign the APK file using the key generated in the previous step. The command will prompt the user to enter the password
        - `$ jarsigner -keystore mykey.keystore RepackagingLab.apk Heisenbug`

## Task 5: Install the Repackaged App and Trigger the Malicious Code
- install the Repackaged App on the Android VM, and test the attack
- on the Android VM, uninstall the app first, otherwise, we will not be able to install the repackaged app because of the signature mismatch
- on the Ubuntu VM: `$ adb install RepackagingLab.apk`
- enter the contacts list and a contact
- *important*
    - go to `Settings`, `Apps`, find `RepackagingLab`, and in `Permissions`, we need to allow the app to access contacts
    - we need to run this app once to register the broadcast receiver
- go to `Settings`, and set the time
- check the contacts list: all cleared

## Task 6: Using Repackaging Attack to Track Victim's Location
- perform antoher repackaging attack where the malicious code will steal the location information from a user's phone, essentially tracking the user's movement
- *step 1: setting up mock locations*
    - on Android device, we can get the location information from its GPS hardware. In Android VM, there isn't such hardware, but Android OS allow us to provide mock locations to applications. All we need to do is to write a mock location app, and then configure the Android OS to get locations from this app. The app has already been installed on the VM, it can simulate location traces in six different cities, simply select a city from the app

- *step 2: configuring DNS*
    - the malicious code in the repackaged app will send out the victim's coordinates to the attacker's server at `www.repackagingattacklab.com`, we are going to use SEED Ubuntu VM to host this server, we need to map the hostname to the Ubuntu VM's IP address, add an entry in `/system/etc/hosts` file on the Android VM: `10.0.2.15 www.repackagingattacklab.com`

- *step 3: repackaging and installing the victim app*
    - download 3 smali files `MaliciousCode.smali`, `SendData$1.smali`, `SendData.smali`, and put them in `smali/com/mobiseed/repackaging` folder in `RepackagingLab`
    - edit the `AndroidManifest.xml`
        ```
        <manifest...>
            ...
            <uses-permission android:name="android.permission.READ_CONTACTS" />
            <uses-permission android:name="android.permission.WRITE_CONTACTS" />
            <uses-permission android:name="android.permission.ACCESS_COARSE_LOCATION" />
            <uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
            <uses-permission android:name="android.permission.ACCESS_MOCK_LOCATION" />
            ...
            <application>
                ...
                <receiver android:name="com.MaliciousCode">
                    <intent-filter>
                        <action android:name="android.intent.action.TIME_SET"/>
                    </intent-filter>
                </receiver>
                <receiver android:name="com.mobiseed.repackaging.MaliciousCode">
                    <intent-filter>
                        <action android:name="android.intent.action.TIME_SET"/>
                    </intent-filter>
                </receiver>
            </application>
        </manifest>
        ```
    - assemble everything, then self-sign the apk file, uninstall the app on the Android VM, then, on the Ubuntu VM, run `adb install RepackagingLab.apk`
- *step 4: enabling the permission on the Android VM*
    - go to `Settings`, turn on all permissions for this app

- *step 5: triggering the attacking code*
    - run the app once to make it being registered, then run the mock location application and choose a location, then change the time on the Android VM to trigger the attack

- *step 6: tracking the victim*
    - once the malicious code is triggered, we can go to the Ubuntu VM, visit `http://www.repackagingattacklab.com` on the browser, we will be able to track the victim's location
