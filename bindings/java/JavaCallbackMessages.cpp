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

#include "JavaCallbackMessages.h"

#include <net/packet_type.h>
#include <net/packet_handler.h>
#include <net/packet.h>
#include <net/host.h>

JavaVM *javaVM = NULL;

class JavaCallbackChatMessage : public NetworkMessage
{
public:
	void Handle(Chimera& chimera, const Host& sender, const Packet& pckt)
	{
		if(javaVM == NULL)
		{
			pf_log[W_ERR] << "Global variable are uninitialized, abording handle of chat message.";
			return;
		}

		std::string message = pckt.GetArg<std::string>(JAVA_CALLBACK_CHAT_MESSAGE);
		Host *host_copy = new Host(sender);


		JNIEnv *env;
		jint result = javaVM->AttachCurrentThread((void**)&env, NULL);
		if (result != JNI_OK)  {
			pf_log[W_ERR] << "Failed to attach the JVM to the thread : " << result;
			return;
		}

		result = javaVM->GetEnv((void**) &env, JNI_VERSION_1_2);
		if (result != JNI_OK) {
			pf_log[W_ERR] << "Failed to get the environment using GetEnv() : " << result;
			return;
		}

		const char *className = "libArbore/chimera/Chimera";
		jclass cls = env->FindClass(className);

		if(cls == NULL)
		{
			pf_log[W_ERR] << "Cant find class Chimera";
		}

		jmethodID mid = env->GetStaticMethodID(cls, "MessageCallback", "(Ljava/lang/String;J)V");

		if (mid == NULL)
		{
			pf_log[W_ERR] << "Cant find callback method";
			return;
		}

		env->CallStaticVoidMethod(cls, mid, env->NewStringUTF(message.c_str()), (jlong) host_copy);
	}
};

/* We use a custom message type (21), so it's not a regular chimera chat message */
PacketType JavaCallbackChatType(21, new JavaCallbackChatMessage, Packet::REQUESTACK|Packet::MUSTROUTE,
                                "CHAT", /* CHIMERA_CHAT_MESSAGE */ T_STR, T_END);
