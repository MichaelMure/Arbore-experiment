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

#include "libArbore_chimera_Leafset.h"
#include <chimera/leafset.h>
#include <net/host.h>
#include <net/pf_addr.h>
#include <vector>

/*
 * Class:     libArbore_chimera_Leafset
 * Method:    N_toString
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_libArbore_chimera_Leafset_N_1toString
  (JNIEnv *env, jobject, jlong instance)
{
	std::vector<Host> *leafset = (std::vector<Host>*) instance;
	std::string str = "";
	int index = 0;

	std::vector<Host>::const_iterator it = leafset->begin();
	while(it < leafset->end())
	{
		str += TypToStr(index) + " --> " + it->GetAddr().GetStr() + '\n';
		index++;
		it++;
	}

	return env->NewStringUTF(str.c_str());
}

/*
 * Class:     libArbore_chimera_Leafset
 * Method:    N_getHost
 * Signature: (JI)J
 */
JNIEXPORT jlong JNICALL Java_libArbore_chimera_Leafset_N_1getHost
  (JNIEnv *, jobject, jlong instance, jint index)
{
	std::vector<Host> *leafset = (std::vector<Host>*) instance;
	Host host = leafset->at(index);
	Host *copy = new Host(host);
	return (long) copy;
}

/*
 * Class:     libArbore_chimera_Leafset
 * Method:    N_getHostNumber
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_libArbore_chimera_Leafset_N_1getHostNumber
  (JNIEnv *, jobject, jlong instance)
{
	std::vector<Host> *leafset = (std::vector<Host>*) instance;
	return leafset->size();
}


/*
 * Class:     libArbore_chimera_Leafset
 * Method:    destroyCppSide
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_libArbore_chimera_Leafset_destroyCppSide
  (JNIEnv *, jobject, jlong instance)
{
	std::vector<Host> *leafset = (std::vector<Host>*) instance;
	delete leafset;
}
