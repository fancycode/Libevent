/*
 * Copyright (c) 2007-2012 Niels Provos and Nick Mathewson
 * Copyright (c) 2002-2006 Niels Provos <provos@citi.umich.edu>
 * Copyright (c) 2012 Joachim Bauch <mail@joachim-bauch.de>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef EVENT2_BUFFEREVENT_INDIRECT_H_INCLUDED_
#define EVENT2_BUFFEREVENT_INDIRECT_H_INCLUDED_

#include <event2/event-config.h>
#include <event2/bufferevent.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BEV_INDIRECT_MADE_PROGRESS 1
#define BEV_INDIRECT_BLOCKED 2
#define BEV_INDIRECT_ERR 4

/**
   Called when the bufferevent needs more data.
   
   The method should be executed non-blocking and BEV_INDIRECT_BLOCKED
   should be returned if no data is available.
   
   You can call bufferevent_indirect_notify_ready with EV_READ when
   more data is available for reading.

   @param bev the bufferevent for which data should be read
   @param indirect the indirect object given when the bufferevent was created
   @param data the pointer to read the data to
   @param size the number of bytes to read, will be replaced with the number
      of bytes actually read
   @return any combination of BEV_INDIRECT_*
**/
typedef int (*bufferevent_indirect_read_cb)(struct bufferevent *bev,
    void *indirect, void *data, size_t *size);

/**
   Called when the bufferevent has data to be sent out.

   The method should be executed non-blocking and BEV_INDIRECT_BLOCKED
   should be returned if currently no data can be written.

   You can call bufferevent_indirect_notify_ready with EV_READ when
   more data can be written.

   @param bev the bufferevent which has data available
   @param indirect the indirect object given when the bufferevent was created
   @param data the buffer that should be sent
   @return any combination of BEV_INDIRECT_*
**/
typedef int (*bufferevent_indirect_write_cb)(struct bufferevent *bev,
    void *indirect, struct evbuffer *data);

/**
   Called when reading/writing should be enabled.
   
   @param bev the bufferevent which wants to enable reading/writing
   @param indirect the indirect object given when the bufferevent was created
   @param what any combination of EV_READ | EV_WRITE
   @return 0 on success, -1 on failure
**/
typedef int (*bufferevent_indirect_enable_cb)(struct bufferevent *bev,
    void *indirect, short what);

/**
   Called when reading/writing should be disabled.
   
   @param bev the bufferevent which wants to disable reading/writing
   @param indirect the indirect object given when the bufferevent was created
   @param what any combination of EV_READ | EV_WRITE
   @return 0 on success, -1 on failure
**/
typedef int (*bufferevent_indirect_disable_cb)(struct bufferevent *bev,
    void *indirect, short what);

/**
   Called when a new connection should be established.
   
   The connection should be established asynchronously and on connection
   bufferevent_indirect_notify_event must be called with BEV_EVENT_CONNECTED
   or with BEV_EVENT_ERROR if the connection failed.
   
   @param bev the bufferevent which wants to connect
   @param indirect the indirect object given when the bufferevent was created
   @param addr the address we should connect to
   @param socklen the length of the address
   @return 0 on success, -1 on failure
**/
typedef int (*bufferevent_indirect_connect_cb)(struct bufferevent *bev,
    void *indirect, struct sockaddr *addr, int socklen);

/**
   Called when the indirect IO should be cleaned up.
   
   @param bev the bufferevent which is about to be released
   @param indirect the indirect object given when the bufferevent was created
   @return 0 on success, -1 on failure
**/
typedef int (*bufferevent_indirect_cleanup_cb)(struct bufferevent *bev,
    void *indirect);


/**
   Callback functions used by the indirect bufferevent to perform IO.
**/
struct bufferevent_indirect_callbacks
{
    bufferevent_indirect_read_cb        cb_read;
    bufferevent_indirect_write_cb       cb_write;
    bufferevent_indirect_enable_cb      cb_enable;
    bufferevent_indirect_disable_cb     cb_disable;
    bufferevent_indirect_connect_cb     cb_connect;
    bufferevent_indirect_cleanup_cb     cb_cleanup;
};

/**
   Create a new bufferevent that uses callbacks to perform IO.
   
   As the indirect IO usually takes place in a separate thread,
   at least BEV_OPT_THREADSAFE should be provided.
   
   @param base the event base to associate with the new bufferevent
   @param indirect object to pass along to the callbacks
   @param callbacks callback functions to use when performing IO
   @param options zero or more BEV_OPT_* flags
   @return a pointer to a newly allocated bufferevent struct, or NULL if an
	  error occurred
**/
struct bufferevent *
bufferevent_indirect_new(struct event_base *base,
    void *indirect,
    const struct bufferevent_indirect_callbacks *callbacks,
    int options);

/**
   Launch a connect() attempt with an indirect bufferevent.

   When the connect succeeds, the eventcb will be invoked with
   BEV_EVENT_CONNECTED set.

   @param bev an existing bufferevent allocated with
       bufferevent_indirect_new().
   @param addr the address we should connect to
   @param socklen the length of the address
   @return 0 on success, -1 on failure.
 */
int bufferevent_indirect_connect(struct bufferevent *bev, struct sockaddr *addr,
    int socklen);

/**
   Resolve the hostname 'hostname' and connect to it as with
   bufferevent_indirect_connect().

   @param bev An existing bufferevent allocated with
       bufferevent_indirect_new().
   @param evdns_base Optionally, an evdns_base to use for resolving hostnames
      asynchronously. May be set to NULL for a blocking resolve.
   @param family A preferred address family to resolve addresses to, or
      AF_UNSPEC for no preference.  Only AF_INET, AF_INET6, and AF_UNSPEC are
      supported.
   @param hostname The hostname to resolve
   @param port The port to connect to on the resolved address.
   @return 0 if successful, -1 on failure.
   @see bufferevent_socket_connect_hostname
 */
int bufferevent_indirect_connect_hostname(struct bufferevent *bev,
    struct evdns_base *evdns, int family, const char *hostname, int port);

/**
   Notify bufferevent that the indirect IO is ready for reading/writing.
   
   This should be called after reading/writing was blocked in previous
   calls to the corresponding callbacks.
   
   @param bev An existing bufferevent allocated with
       bufferevent_indirect_new().
   @param what any combination of EV_READ | EV_WRITE
   @return 0 if successful, -1 on failure.
**/
int bufferevent_indirect_notify_ready(struct bufferevent *bev, short what);

/**
   Notify bufferevent that the state of the indirect IO has changed.
   
   @param bev An existing bufferevent allocated with
       bufferevent_indirect_new().
   @param what any combination of BEV_EVENT_*
   @return 0 if successful, -1 on failure.
**/
int bufferevent_indirect_notify_event(struct bufferevent *bev, short what);

#ifdef __cplusplus
}
#endif

#endif /* EVENT2_BUFFEREVENT_INDIRECT_H_INCLUDED_ */
