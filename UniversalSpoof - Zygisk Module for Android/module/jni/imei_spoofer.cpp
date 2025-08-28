#include <jni.h>
#include <android/log.h>
#include <string.h>
#include "zygisk.hpp"

#define LOG_TAG "UniversalSpoof"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using namespace zygisk;

class UniversalSpoofModule : public ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
        LOGD("Módulo Universal Spoof carregado");
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        // Hooking para Android ID e Serial
        hookAndroidID();
        hookSerial();

        // Hooking para IMEI
        hookTelephonyManager();
        hookSubscriptionManager();
        hookTelephonyRegistry();
    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        const char* nice_name = args->nice_name ? env->GetStringUTFChars(args->nice_name, nullptr) : "unknown";
        LOGD("Processo especializado - spoofing universal ativo para: %s", nice_name);
        if (args->nice_name) env->ReleaseStringUTFChars(args->nice_name, nice_name);
    }

private:
    Api *api;
    JNIEnv *env;

    // Ponteiros para as funções originais - Android ID & Serial
    static jstring (*orig_getString)(JNIEnv *, jobject, jobject, jstring);
    static jstring (*orig_getSerial)(JNIEnv *, jobject);

    // Ponteiros para funções originais - IMEI
    static jstring (*orig_getDeviceId)(JNIEnv *, jobject, jint);
    static jstring (*orig_getImei)(JNIEnv *, jobject, jint);
    static jstring (*orig_getDeviceIdWithFeature)(JNIEnv *, jobject, jstring, jstring);
    static jobjectArray (*orig_getDeviceIdList)(JNIEnv *, jobject);

    // Valores falsos
    static const char* FAKE_ANDROID_ID;
    static const char* FAKE_SERIAL;
    static const char* FAKE_IMEI;

    // ===== Hooks para Android ID & Serial =====

    // Hook para Settings.Secure.getString(ContentResolver, String)
    static jstring getString_hook(JNIEnv *env, jobject thiz, jobject cr, jstring name) {
        const char *nameC = env->GetStringUTFChars(name, nullptr);
        jstring ret = nullptr;

        if (nameC != nullptr && strcmp(nameC, "android_id") == 0) {
            ret = env->NewStringUTF(FAKE_ANDROID_ID);
            LOGD("Spoof aplicado em Settings.Secure.getString(android_id)");
            env->ReleaseStringUTFChars(name, nameC);
            return ret;
        }

        env->ReleaseStringUTFChars(name, nameC);
        // Para outras chaves, chama a função original
        return orig_getString(env, thiz, cr, name);
    }

    // Hook para Build.getSerial()
    static jstring getSerial_hook(JNIEnv *env, jobject thiz) {
        LOGD("Spoof aplicado em Build.getSerial()");
        return env->NewStringUTF(FAKE_SERIAL);
    }

    void hookAndroidID() {
        JNINativeMethod methods[] = {
                {
                        "getString",
                        "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;",
                        reinterpret_cast<void*>(getString_hook)
                }
        };

        api->hookJniNativeMethods(env, "android/provider/Settings$Secure", methods, 1);
        orig_getString = reinterpret_cast<decltype(orig_getString)>(methods[0].fnPtr);

        if (orig_getString != nullptr) {
            LOGD("Hook de Settings.Secure.getString registrado com sucesso");
        } else {
            LOGD("Falha ao registrar hook de Settings.Secure.getString");
        }
    }

    void hookSerial() {
        JNINativeMethod methods[] = {
                {
                        "getSerial",
                        "()Ljava/lang/String;",
                        reinterpret_cast<void*>(getSerial_hook)
                }
        };

        api->hookJniNativeMethods(env, "android/os/Build", methods, 1);
        orig_getSerial = reinterpret_cast<decltype(orig_getSerial)>(methods[0].fnPtr);

        if (orig_getSerial != nullptr) {
            LOGD("Hook de Build.getSerial registrado com sucesso");
        } else {
            LOGD("Falha ao registrar hook de Build.getSerial");
        }
    }

    // ===== Hooks para IMEI =====

    // Hook para TelephonyManager.getDeviceId()
    static jstring getDeviceId_hook(JNIEnv *env, jobject thiz, jint slotIndex) {
        LOGD("Spoofing TelephonyManager.getDeviceId() para slot: %d", slotIndex);
        return env->NewStringUTF(FAKE_IMEI);
    }

    // Hook para TelephonyManager.getImei()
    static jstring getImei_hook(JNIEnv *env, jobject thiz, jint slotIndex) {
        LOGD("Spoofing TelephonyManager.getImei() para slot: %d", slotIndex);
        return env->NewStringUTF(FAKE_IMEI);
    }

    // Hook para TelephonyManager.getDeviceId(String callingPackage, String featureId)
    static jstring getDeviceIdWithFeature_hook(JNIEnv *env, jobject thiz, jstring callingPackage, jstring featureId) {
        LOGD("Spoofing TelephonyManager.getDeviceId(with feature)");
        return env->NewStringUTF(FAKE_IMEI);
    }

    // Hook para TelephonyManager.getDeviceIdList()
    static jobjectArray getDeviceIdList_hook(JNIEnv *env, jobject thiz) {
        LOGD("Spoofing TelephonyManager.getDeviceIdList()");

        jclass stringClass = env->FindClass("java/lang/String");
        jobjectArray result = env->NewObjectArray(1, stringClass, nullptr);
        jstring fakeImei = env->NewStringUTF(FAKE_IMEI);
        env->SetObjectArrayElement(result, 0, fakeImei);

        return result;
    }

    void hookTelephonyManager() {
        // Hook para vários métodos do TelephonyManager
        JNINativeMethod methods[] = {
                {
                        "getDeviceId",
                        "()Ljava/lang/String;",
                        reinterpret_cast<void*>(+[](JNIEnv *env, jobject thiz) {
                            LOGD("Spoofing TelephonyManager.getDeviceId()");
                            return env->NewStringUTF(FAKE_IMEI);
                        })
                },
                {
                        "getDeviceId",
                        "(I)Ljava/lang/String;",
                        reinterpret_cast<void*>(getDeviceId_hook)
                },
                {
                        "getImei",
                        "(I)Ljava/lang/String;",
                        reinterpret_cast<void*>(getImei_hook)
                },
                {
                        "getDeviceId",
                        "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;",
                        reinterpret_cast<void*>(getDeviceIdWithFeature_hook)
                },
                {
                        "getDeviceIdList",
                        "()Ljava/util/List;",
                        reinterpret_cast<void*>(+[](JNIEnv *env, jobject thiz) {
                            LOGD("Spoofing TelephonyManager.getDeviceIdList() as List");
                            jclass arrayListClass = env->FindClass("java/util/ArrayList");
                            jmethodID constructor = env->GetMethodID(arrayListClass, "<init>", "()V");
                            jmethodID addMethod = env->GetMethodID(arrayListClass, "add", "(Ljava/lang/Object;)Z");

                            jobject list = env->NewObject(arrayListClass, constructor);
                            jstring fakeImei = env->NewStringUTF(FAKE_IMEI);
                            env->CallBooleanMethod(list, addMethod, fakeImei);

                            return list;
                        })
                }
        };

        api->hookJniNativeMethods(env, "android/telephony/TelephonyManager", methods, 5);
        orig_getDeviceId = reinterpret_cast<decltype(orig_getDeviceId)>(methods[1].fnPtr);
        orig_getImei = reinterpret_cast<decltype(orig_getImei)>(methods[2].fnPtr);

        LOGD("Hooks do TelephonyManager registrados");
    }

    void hookSubscriptionManager() {
        // Hook para SubscriptionManager.getDeviceId()
        JNINativeMethod methods[] = {
                {
                        "getDeviceId",
                        "(I)Ljava/lang/String;",
                        reinterpret_cast<void*>(getDeviceId_hook)
                }
        };

        api->hookJniNativeMethods(env, "android/telephony/SubscriptionManager", methods, 1);
        LOGD("Hooks do SubscriptionManager registrados");
    }

    void hookTelephonyRegistry() {
        // Hook para métodos do TelephonyRegistry que podem vazar IMEI
        JNINativeMethod methods[] = {
                {
                        "getDeviceId",
                        "(I)Ljava/lang/String;",
                        reinterpret_cast<void*>(getDeviceId_hook)
                }
        };

        api->hookJniNativeMethods(env, "com/android/internal/telephony/ITelephonyRegistry", methods, 1);
        LOGD("Hooks do TelephonyRegistry registrados");
    }
};

// Inicializa os ponteiros estáticos para as funções originais
jstring (*UniversalSpoofModule::orig_getString)(JNIEnv *, jobject, jobject, jstring) = nullptr;
jstring (*UniversalSpoofModule::orig_getSerial)(JNIEnv *, jobject) = nullptr;
jstring (*UniversalSpoofModule::orig_getDeviceId)(JNIEnv *, jobject, jint) = nullptr;
jstring (*UniversalSpoofModule::orig_getImei)(JNIEnv *, jobject, jint) = nullptr;
jstring (*UniversalSpoofModule::orig_getDeviceIdWithFeature)(JNIEnv *, jobject, jstring, jstring) = nullptr;
jobjectArray (*UniversalSpoofModule::orig_getDeviceIdList)(JNIEnv *, jobject) = nullptr;

// Inicializa os valores falsos
const char* UniversalSpoofModule::FAKE_ANDROID_ID = "FAKE_ANDROID_ID_1234567890";
const char* UniversalSpoofModule::FAKE_SERIAL = "FAKE_SERIAL_9876543210";
const char* UniversalSpoofModule::FAKE_IMEI = "355667788990011";

// Registra apenas UM módulo
REGISTER_ZYGISK_MODULE(UniversalSpoofModule)