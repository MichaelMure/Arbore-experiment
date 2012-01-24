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

 #include "libArbore_network_Host_List.h"
 #include <net/hosts_list.h>

/*
 * Class:     libArbore_network_Host_List
 * Method:    N_decodeHost
 * Signature: (JLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_libArbore_network_Host_1List_N_1decodeHost
  (JNIEnv *env, jobject, jlong instance, jstring hostname)
	{
		HostsList* host_list = (HostsList*) instance;
		const char *nativeString = env->GetStringUTFChars(hostname, 0);
		Host h = host_list->DecodeHost(nativeString);
		Host *copy = new Host(h);
		env->ReleaseStringUTFChars(hostname, nativeString);
		return (long) copy;
	}
