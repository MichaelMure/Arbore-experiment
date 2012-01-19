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

#include "libArbore_util_Key.h"
#include <util/key.h>
#include <string>
#include <ctime>

/*
 * Class:     libArbore_util_Key
 * Method:    N_toString
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_libArbore_util_Key_N_1toString
  (JNIEnv *env, jobject, jlong instance)
{
	Key* key = (Key*) instance;
	std::string str = key->GetStr();
	return env->NewStringUTF(str.c_str());
}

/*
 * Class:     libArbore_util_Key
 * Method:    N_GetRandomKey
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_libArbore_util_Key_N_1GetRandomKey
  (JNIEnv *, jclass)
{
	Key k = Key::GetRandomKey();
	return (jlong) new Key(k);
}

/*
 * Class:     libArbore_util_Key
 * Method:    initCppSide
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_libArbore_util_Key_initCppSide
  (JNIEnv *, jobject)
{
	return (jlong) new Key();
}

/*
 * Class:     libArbore_util_Key
 * Method:    destroyCppSide
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_libArbore_util_Key_destroyCppSide
  (JNIEnv *, jobject, jlong instance)
{
	Key* key = (Key*) instance;
	delete key;
}

/*
 * Class:     libArbore_util_Key
 * Method:    InitRandomNumberGenerator
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_libArbore_util_Key_InitRandomNumberGenerator
  (JNIEnv *, jclass)
{
	srand(time(NULL));
}
