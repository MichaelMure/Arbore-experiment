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
 * Method:    SetLoggedFlags
 * Signature: (Ljava/lang/String;Z)V
 */
JNIEXPORT void JNICALL Java_libArbore_util_Log_SetLoggedFlags
  (JNIEnv * env, jobject javaObj, jstring s, jboolean to_syslog)
{

}

/*
 * Class:     libArbore_util_Log
 * Method:    LoggedFlags
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_libArbore_util_Log_LoggedFlags
  (JNIEnv *, jobject)
{

}

/*
 * Class:     libArbore_util_Log
 * Method:    ToSyslog
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_libArbore_util_Log_ToSyslog
  (JNIEnv *, jobject)
{

}

/*
 * Class:     libArbore_util_Log
 * Method:    initCppSide
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_libArbore_util_Log_initCppSide
  (JNIEnv * env, jobject javaObj)
{
	//unhand(javaObj)->logPtr_ = (long) &pf_log;
	//javaObj->logPtr_ = (long) &pf_log;
}
