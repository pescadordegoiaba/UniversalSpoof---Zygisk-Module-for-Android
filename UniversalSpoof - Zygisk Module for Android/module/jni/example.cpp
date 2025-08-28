#include <jni.h>
#include <android/log.h>
#include <string.h>
#include "zygisk.hpp"

#define LOG_TAG "SpoofIDs"
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, __VA_ARGS__)

using namespace zygisk;

class SpoofIDsModule : public ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
        LOGD("Módulo carregado");
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        // Hooking deve ser feito no preAppSpecialize
        hookAndroidID();
        hookSerial();
    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        // Apenas para logging após a especialização
        LOGD("Processo especializado - spoofing ativo");
    }

private:
    Api *api;
    JNIEnv *env;

    // Ponteiros para as funções originais
    static jstring (*orig_getString)(JNIEnv *, jobject, jobject, jstring);
    static jstring (*orig_getSerial)(JNIEnv *, jobject);

    // Hook para Settings.Secure.getString(ContentResolver, String)
    static jstring getString_hook(JNIEnv *env, jobject thiz, jobject cr, jstring name) {
        const char *nameC = env->GetStringUTFChars(name, nullptr);
        jstring ret = nullptr;

        if (nameC != nullptr && strcmp(nameC, "android_id") == 0) {
            ret = env->NewStringUTF("FAKE_ANDROID_ID_1234567890");
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
        return env->NewStringUTF("FAKE_SERIAL_9876543210");
        // Nota: Não chamamos a original pois queremos sempre spoofar
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

        // Salva o ponteiro para a função original
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

        // Salva o ponteiro para a função original
        orig_getSerial = reinterpret_cast<decltype(orig_getSerial)>(methods[0].fnPtr);

        if (orig_getSerial != nullptr) {
            LOGD("Hook de Build.getSerial registrado com sucesso");
        } else {
            LOGD("Falha ao registrar hook de Build.getSerial");
        }
    }
};

// Inicializa os ponteiros estáticos para as funções originais
jstring (*SpoofIDsModule::orig_getString)(JNIEnv *, jobject, jobject, jstring) = nullptr;
jstring (*SpoofIDsModule::orig_getSerial)(JNIEnv *, jobject) = nullptr;

REGISTER_ZYGISK_MODULE(SpoofIDsModule)