/*
 * Copyright(C) 2012 Benoît Saccomano
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

 #include "libArbore_network_Host.h"
 #include <net/host.h>

/*
 * Class:     libArbore_network_Host
 * Method:    N_toString
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_libArbore_network_Host_N_1toString
  (JNIEnv *env, jobject, jlong instance)
	{
		Host* host = (Host*) instance;
		std::string str = host->GetAddr().GetStr();
		return env->NewStringUTF(str.c_str());
	}

/*
 * Class:     libArbore_network_Host
 * Method:    initCppSide
 * Signature: ()J
 */
JNIEXPORT jlong JNICALL Java_libArbore_network_Host_initCppSide
  (JNIEnv *, jobject)
	{
		return (jlong) new Host;
	}

/*
 * Class:     libArbore_network_Host
 * Method:    destroyCppSide
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_libArbore_network_Host_destroyCppSide
  (JNIEnv *, jobject, jlong instance)
	{
		 Host* host = (Host*) instance;
		 delete host;
	}
