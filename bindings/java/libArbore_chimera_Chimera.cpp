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

#include "libArbore_chimera_Chimera.h"
#include <chimera/chimera.h>
#include <chimera/routing.h>
#include <chimera/messages.h>
#include <net/packet_type_list.h>

#include "JavaCallbackMessages.h"

/*
 * Class:     libArbore_chimera_Chimera
 * Method:    N_getMe
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_libArbore_chimera_Chimera_N_1getMe
  (JNIEnv *, jobject, jlong instance)
	{
		Chimera* ch = (Chimera*) instance;
		Host h = ch->GetMe();
		return (long) new Host(h);
	}

/*
 * Class:     libArbore_chimera_Chimera
 * Method:    N_getNetwork
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_libArbore_chimera_Chimera_N_1getNetwork
  (JNIEnv *, jobject, jlong instance)
	{
		Chimera* ch = (Chimera*) instance;
		return (long) (ch->GetNetwork());
	}

/*
 * Class:     libArbore_chimera_Chimera
 * Method:    N_getLeafset
 * Signature: (J)J
 */
JNIEXPORT jlong JNICALL Java_libArbore_chimera_Chimera_N_1getLeafset
  (JNIEnv *, jobject, jlong instance)
{
	Chimera* ch = (Chimera*) instance;
	std::vector<Host> leafset = ch->GetRouting()->getLeafset();
	return (long) new std::vector<Host>(leafset);
}

/*
 * Class:     libArbore_chimera_Chimera
 * Method:    N_join
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_libArbore_chimera_Chimera_N_1join
  (JNIEnv *, jobject, jlong instance, jlong bootstrap)
	{
		Chimera* ch = (Chimera*) instance;
		Host* host = (Host*) bootstrap;
		ch->Join(*host);
	}

/*
 * Class:     libArbore_chimera_Chimera
 * Method:    N_route
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_libArbore_chimera_Chimera_N_1route
  (JNIEnv *, jobject, jlong instance, jlong pckt)
	{
		Chimera* ch = (Chimera*) instance;
		Packet* pck = (Packet*) pckt;
		return ch->Route(*pck);
	}

/*
 * Class:     libArbore_chimera_Chimera
 * Method:    initCppSide
 * Signature: (JIJ)J
 */
JNIEXPORT jlong JNICALL Java_libArbore_chimera_Chimera_initCppSide
  (JNIEnv *env, jobject ob, jint port, jlong key)
	{
		jint result = env->GetJavaVM(&javaVM);
		if (result < 0) {
			pf_log[W_ERR] << "Error retrieving Java VM";
		}

		Key mykey = (Key) key;
		Chimera *chimera = new Chimera(NULL,port,mykey);

		packet_type_list.RegisterType(JavaCallbackChatType);

		return (jlong) chimera;
	}

/*
 * Class:     libArbore_chimera_Chimera
 * Method:    destroyCppSide
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_libArbore_chimera_Chimera_destroyCppSide
  (JNIEnv *, jobject, jlong instance)
	{
		Chimera* ch = (Chimera*) instance;
		delete ch;
	}
