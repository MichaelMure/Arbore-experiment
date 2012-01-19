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

#include "libArbore_scheduler_Scheduler.h"
#include <scheduler/scheduler.h>

/*
 * Class:     libArbore_scheduler_Scheduler
 * Method:    N_StartSchedulers
 * Signature: (I)V
 */
JNIEXPORT void JNICALL Java_libArbore_scheduler_Scheduler_N_1StartSchedulers
  (JNIEnv *, jclass, jint number)
{
	Scheduler::StartSchedulers(number);
}
