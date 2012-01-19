/*
 * Copyright(C) 2012 Michael Muré
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "libArbore_util_Log.h"
#include <util/pf_log.h>

/*
 * Class:     libArbore_util_Log
 * Method:    N_print
 * Signature: (JILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_libArbore_util_Log_N_1print
  (JNIEnv *env, jobject, jlong instance, jint level, jstring s)
{
	Log* log = (Log*) instance;
	const char *nativeString = env->GetStringUTFChars(s, 0);

	(*log)[level] << nativeString;

	env->ReleaseStringUTFChars(s, nativeString);
}

/*
 * Class:     libArbore_util_Log
 * Method:    N_SetLoggedFlags
 * Signature: (JLjava/lang/String;Z)V
 */
JNIEXPORT void JNICALL Java_libArbore_util_Log_N_1SetLoggedFlags
  (JNIEnv *env, jobject, jlong instance, jstring s, jboolean to_syslog)
{
	Log* log = (Log*) instance;
	const char *nativeString = env->GetStringUTFChars(s, 0);

	log->SetLoggedFlags(std::string(nativeString), to_syslog);

	env->ReleaseStringUTFChars(s, nativeString);
}

/*
 * Class:     libArbore_util_Log
 * Method:    N_ToSyslog
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_libArbore_util_Log_N_1ToSyslog
  (JNIEnv *, jobject, jlong instance)
{
	Log* log = (Log*) instance;
	return log->ToSyslog();
}

/*
 * Class:     libArbore_util_Log
 * Method:    initCppSide
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_libArbore_util_Log_initCppSide
  (JNIEnv *, jobject)
{
	return (jlong) &pf_log;
}

/*
 * Class:     libArbore_util_Log
 * Method:    destroyCppSide
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_libArbore_util_Log_destroyCppSide
  (JNIEnv *, jobject, jlong)
{
 /* nothing to destroy here */
}
