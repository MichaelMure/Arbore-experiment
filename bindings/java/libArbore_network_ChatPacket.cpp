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

#include "libArbore_network_ChatPacket.h"
#include <net/packet.h>
#include <chimera/messages.h>

/*
 * Class:     libArbore_network_ChatPacket
 * Method:    N_toString
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_libArbore_network_ChatPacket_N_1toString
  (JNIEnv *env, jobject, jlong instance)
{
	Packet *packet = (Packet*) instance;
	std::string str = packet->GetPacketInfo();
	return env->NewStringUTF(str.c_str());
}

/*
 * Class:     libArbore_network_ChatPacket
 * Method:    N_SetFlags
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_libArbore_network_ChatPacket_N_1SetFlags
  (JNIEnv *, jobject, jlong instance, jint flags)
{
	Packet *packet = (Packet*) instance;
	packet->SetFlags(flags);
}

/*
 * Class:     libArbore_network_ChatPacket
 * Method:    initCppSide
 * Signature: (JJLjava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_libArbore_network_ChatPacket_initCppSide
  (JNIEnv *env, jobject, jlong key_source, jlong key_destination, jstring s)
{
	const char *nativeString = env->GetStringUTFChars(s, 0);
	Packet* packet = new Packet(ChimeraChatType, *((Key*)key_source), *((Key*)key_destination));

	packet->SetArg(CHIMERA_CHAT_MESSAGE, std::string(nativeString));
	packet->SetFlag(Packet::MUSTROUTE);
	env->ReleaseStringUTFChars(s, nativeString);
	return (jlong) packet;
}

/*
 * Class:     libArbore_network_ChatPacket
 * Method:    destroyCppSide
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_libArbore_network_ChatPacket_destroyCppSide
  (JNIEnv *, jobject, jlong instance)
{
	Packet *packet = (Packet*) instance;
	delete packet;
}
