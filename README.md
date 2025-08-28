# UniversalSpoof---Zygisk-Module-for-Android
Dont work (Tested only android 14, dont work)

A comprehensive Zygisk module designed to protect your privacy by spoofing device identifiers on Android systems. This module hooks into various system services to provide虚假 device information to applications.
🛡️ Features

    IMEI Spoofing: Replaces real IMEI numbers with customizable虚假 values

    Android ID Masking: Spoofs the device's Android ID

    Serial Number Protection: Replaces hardware serial numbers

    Multi-Layer Hooking: Targets multiple system services for comprehensive coverage

    Selective Application: Can be configured to target specific applications only

📋 Supported Identifiers
Identifier	Original API	Spoofed Value
IMEI	TelephonyManager.getDeviceId()	Customizable虚假 IMEI
IMEI (Slot-specific)	TelephonyManager.getImei(int)	Customizable虚假 IMEI
Android ID	Settings.Secure.getString(android_id)	FAKE_ANDROID_ID_1234567890
Serial Number	Build.getSerial()	FAKE_SERIAL_9876543210
🚀 Installation

    Prerequisites:

        Magisk v24.0 or later

        Zygisk enabled in Magisk settings

        Android 10 or higher

    Installation Steps:
    bash

    # Clone the repository
    git clone https://github.com/yourusername/UniversalSpoof.git

    # Build the module
    cd UniversalSpoof
    ./gradlew build

    # Find the compiled module
    # Location: app/build/intermediates/cxx/Release/.../libuniversalspoof.so

    Magisk Installation:

        Extract the compiled .so file

        Place in Magisk module structure

        Install via Magisk app or recovery

⚙️ Configuration
Customizing Spoofed Values

Edit the following constants in universal_spoof.cpp:
cpp

const char* UniversalSpoofModule::FAKE_ANDROID_ID = "YOUR_CUSTOM_ANDROID_ID";
const char* UniversalSpoofModule::FAKE_SERIAL = "YOUR_CUSTOM_SERIAL";
const char* UniversalSpoofModule::FAKE_IMEI = "YOUR_CUSTOM_IMEI";

Target Specific Applications

Modify the preAppSpecialize method to target specific apps:
cpp

void preAppSpecialize(AppSpecializeArgs *args) override {
    const char* app_name = env->GetStringUTFChars(args->nice_name, nullptr);
    
    if (strstr(app_name, "com.target.app") != nullptr) {
        hookAndroidID();
        hookSerial();
        hookTelephonyManager();
    }
    
    env->ReleaseStringUTFChars(args->nice_name, app_name);
}

🔧 Building from Source
Requirements

    Android NDK 25.2.9519653 or later

    CMake 3.22.1 or later

    Android Studio 2022.2.1 or later

Build Commands
bash

# Debug build
./gradlew assembleDebug

# Release build
./gradlew assembleRelease

# Clean build
./gradlew clean build

📊 Logging and Debugging

To monitor module activity:
bash

# View real-time logs
adb logcat -s "UniversalSpoof" -v time

# Check for errors
adb logcat -s "UniversalSpoof" -s "AndroidRuntime" -s "System.err"

# Verify module loading
adb logcat -s "Magisk" -s "zygisk" | grep -i "universal"

🎯 Targeted System Services

The module hooks into the following Android services:

    android.telephony.TelephonyManager

        getDeviceId()

        getDeviceId(int)

        getImei(int)

        getDeviceId(String, String)

        getDeviceIdList()

    android.telephony.SubscriptionManager

        getDeviceId(int)

    android.provider.Settings.Secure

        getString(android_id)

    android.os.Build

        getSerial()

    com.android.internal.telephony.ITelephonyRegistry

        Internal telephony services

⚠️ Limitations & Considerations
Known Limitations

    Android 14+: Increased restrictions make some hooks less effective

    Samsung OneUI: Custom framework modifications may affect functionality

    System Apps: Some system applications may use privileged APIs that bypass hooks

    Hardware-based IDs: Some identifiers are protected by hardware-level security

Legal and Ethical Considerations

    ⚠️ Important: Use this module responsibly and only on devices you own. Spoofing device identifiers may violate:

        Terms of Service of some applications

        Local laws and regulations in some regions

        Platform developer policies

This module is intended for:

    Privacy protection research

    Security testing on owned devices

    Educational purposes

🤝 Contributing

We welcome contributions! Please feel free to:

    Fork the repository

    Create a feature branch (git checkout -b feature/amazing-feature)

    Commit your changes (git commit -m 'Add amazing feature')

    Push to the branch (git push origin feature/amazing-feature)

    Open a Pull Request

Development Guidelines

    Follow Android NDK best practices

    Maintain compatibility with latest Zygisk API

    Add comprehensive logging for debugging

    Test on multiple Android versions

📝 License

This project is licensed under the MIT License - see the LICENSE.md file for details.
🙏 Acknowledgments

    John Wu (@topjohnwu) for Magisk and Zygisk

    The Android security research community

    All contributors and testers

🆘 Support

If you encounter issues:

    Check the Troubleshooting Guide

    Search existing Issues

    Create a new issue with:

        Android version and device model

        Magisk version

        Relevant logcat output

        Steps to reproduce

🔗 Related Projects

    Magisk - The Magic Mask for Android

    Zygisk - Zygisk framework documentation

    XPrivacyLua - Privacy manager for Android

Disclaimer: This module is provided for educational and research purposes only. The developers are not responsible for how this software is used. Always comply with local laws and regulations.
