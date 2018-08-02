
/**
	Header file for Sending
*/
#pragma once

#ifndef SENDING
#define SENDING

#include <stdarg.h>
#include <libwebsockets.h>
#include <glib.h>
#include <time.h>

typedef enum Message_type
{
	SEND_STORE,
	SEND_ERASE
}Message_type;

typedef struct User_info
{
	const void *user_id;
	
	const void *user;
	
	int current_global_message;
	int current_user_message;
	
	GArray *send_array; ///< Chat_memory
	
	uint8_t send_data : 1;
}User_info;

typedef struct Chat_memory
{
	const char *data;
	size_t len;
	unsigned long time;
	Message_type mtype;
	
	uint8_t free_data : 1;
	uint8_t history : 1;
}Chat_memory;

int send_global_data(struct lws *wsi,Message_type mtype,const char *data,size_t len,uint8_t free_data);
int send_global_message_v(struct lws *wsi,Message_type mtype,const char *data,va_list args);
int send_global_message(struct lws *wsi,Message_type mtype,const char *data,...);
int init_user(struct lws *wsi,void *user_id,void *user);
int deinit_user(struct lws *wsi);
int send_user_data_from_struct(User_info *user,struct lws *wsi,Message_type mtype,const char *data,size_t len,uint8_t free_data);
int send_user_data(struct lws *wsi,Message_type mtype,const char *data,size_t len,uint8_t free_data);
int send_user_message_v(struct lws *wsi,Message_type mtype,const char *data,va_list args);
int send_user_message(struct lws *wsi,Message_type mtype,const char *data,...);
int send_global_data_from_id(void *user_id,Message_type mtype,const char *data,size_t len,uint8_t free_data);
int send_global_message_from_id_v(void *user_id,Message_type mtype,const char *data,va_list args);
int send_global_message_from_id(void *user_id,Message_type mtype,const char *data,...);
int send_messages(struct lws *wsi,enum lws_callback_reasons reason,void *user,void *in,size_t len);

#endif
