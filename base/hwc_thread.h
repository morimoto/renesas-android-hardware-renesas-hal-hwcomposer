/*
 *
 * Copyright (C) 2014 Renesas Electronics Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#ifndef _HWC_INTERFACE_THREAD_BASE
#define _HWC_INTERFACE_THREAD_BASE

#include <utils/Thread.h>
#include <utils/Log.h>

using namespace android;

/*! 
 *  @brief Interface of thread control.
 */
class HWCThread {
public:
	enum {
		/* 0~4 : reserved for thread class */
		TERMINATE   = 1,
		/* 5~ : available for user */
		USEREVENT   = 5,
	};

private:
	char           thread_name[64];
	void           *(*thread_entry)(void *);
	void           *thread_arg;

	int            pipe_fd[2];

	/* implement thread */
	class ImplThread : public Thread {
	private:
		bool threadLoop() {
			obj->thread_entry(obj->thread_arg);
			return false;
		}
        HWCThread *obj;
	public:
		ImplThread(HWCThread *_obj): obj(_obj) { }
		virtual ~ImplThread() { }
	};
	sp<ImplThread> mThread;

public:
	/* information */
	inline bool isstart(void) {
		return (mThread.get()) ? true : false;
	}
	inline const char *threadname(void) {
		return &thread_name[0];
	}
	inline int   get_waitevent_fd(void) {
		return pipe_fd[0];
	}

	/* send event to thread */
	int            receive_event(void);
	void           send_event(int code);

	/* thread api */
	void           terminate(void);
	void           start(const char *name, int priority, void *(*routine)(void *), void *arg);

	HWCThread();
	virtual ~HWCThread();
};

#endif
