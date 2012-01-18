/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class libArbore_util_Log */

#ifndef _Included_libArbore_util_Log
#define _Included_libArbore_util_Log
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     libArbore_util_Log
 * Method:    N_SetLoggedFlags
 * Signature: (JLjava/lang/String;Z)V
 */
JNIEXPORT void JNICALL Java_libArbore_util_Log_N_1SetLoggedFlags
  (JNIEnv *, jobject, jlong, jstring, jboolean);

/*
 * Class:     libArbore_util_Log
 * Method:    N_LoggedFlags
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_libArbore_util_Log_N_1LoggedFlags
  (JNIEnv *, jobject, jlong);

/*
 * Class:     libArbore_util_Log
 * Method:    N_ToSyslog
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_libArbore_util_Log_N_1ToSyslog
  (JNIEnv *, jobject, jlong);

/*
 * Class:     libArbore_util_Log
 * Method:    N_print
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_libArbore_util_Log_N_1print
  (JNIEnv *, jobject, jlong, jstring);

/*
 * Class:     libArbore_util_Log
 * Method:    initCppSide
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_libArbore_util_Log_initCppSide
  (JNIEnv *, jobject);

/*
 * Class:     libArbore_util_Log
 * Method:    destroyCppSide
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_libArbore_util_Log_destroyCppSide
  (JNIEnv *, jobject, jlong);

#ifdef __cplusplus
}
#endif
#endif
