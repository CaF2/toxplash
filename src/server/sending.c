#include "sending.h"

static GArray *GLOBAL_SEND_TO_ALL_MEMORY=NULL;
static int GLOBAL_DO_SEND_DATA=0;

static GHashTable *GLOBAL_USER_SEND_TABLE=NULL;
guint GLOBAL_USER_SEND_TABLE_FREE_POS=0;


/**
	Code for handeling the User_info struct.
*/

/**
	Constructor for User_info
	
	@param self
		reset or initialize the User_info struct.
	@returns
		initialized User_info struct.
*/
static User_info *user_info_init(User_info *self,const void *user_id,const void *user)
{
	memset(self,'\0',sizeof(User_info));

	self->user_id=user_id;
	self->user=user;
	
	return self;
}

/**
	Constructor and allocator for the User_info struct
	
	@returns
		newly allocated User_info struct.
*/
static User_info *user_info_new(const void *user_id,const void *user)
{
	User_info *self=malloc(sizeof(User_info));
	if(!self)
	{
		return NULL;
	}

	return user_info_init(self,user_id,user);
}

/**
	De-initialize the struct but does not free the input.
	
	@param self
		Struct to handle
*/
static void user_info_finalize(User_info *self)
{
	
}

/**
	Destructor of the struct, will call user_info_finalize and free. 
	
	@param self
		Struct to handle. Note that self will be freed inside this function
*/
static void user_info_free(User_info *self)
{
	user_info_finalize(self);

	free(self);
}

/////////////////////////////


/**
	Code for handeling the Chat_memory struct.
*/

/**
	Constructor for Chat_memory
	
	@param self
		reset or initialize the Chat_memory struct.
	@returns
		initialized Chat_memory struct.
*/
static Chat_memory *chat_memory_init(Chat_memory *self, const char *data, size_t len, Message_type mtype, uint8_t free_data)
{
	memset(self,'\0',sizeof(Chat_memory));

	self->data=data;
	self->len=len;
	self->time=time(NULL);
	self->mtype=mtype;

	return self;
}

/**
	Constructor and allocator for the Chat_memory struct
	
	@returns
		newly allocated Chat_memory struct.
*/
/*static Chat_memory *chat_memory_new(const char *data,size_t len,uint8_t free_data)*/
/*{*/
/*	Chat_memory *self=malloc(sizeof(Chat_memory));*/
/*	if(!self)*/
/*	{*/
/*		return NULL;*/
/*	}*/

/*	return chat_memory_init(self,data,len,free_data);*/
/*}*/

/**
	De-initialize the struct but does not free the input.
	
	@param self
		Struct to handle
*/
static void chat_memory_finalize(Chat_memory *self)
{
	
}

/**
	Destructor of the struct, will call chat_memory_finalize and free. 
	
	@param self
		Struct to handle. Note that self will be freed inside this function
*/
/*static void chat_memory_free(Chat_memory *self)*/
/*{*/
/*	chat_memory_finalize(self);*/

/*	free(self);*/
/*}*/

/////////////////////////////

int send_global_data(struct lws *wsi, Message_type mtype, const char *data, size_t len, uint8_t free_data)
{
	//printf("INSERTING:: %s\n",data);

	if(GLOBAL_SEND_TO_ALL_MEMORY==NULL)
	{
		GLOBAL_SEND_TO_ALL_MEMORY=g_array_sized_new(FALSE,FALSE,sizeof(Chat_memory),10);
		g_array_set_clear_func(GLOBAL_SEND_TO_ALL_MEMORY,(GDestroyNotify)chat_memory_finalize);
	}
	
	Chat_memory cdata;
	
	chat_memory_init(&cdata,data,len,mtype,free_data);
	
	g_array_append_val(GLOBAL_SEND_TO_ALL_MEMORY,cdata);
	
	if(GLOBAL_DO_SEND_DATA==0)
	{
		lws_callback_on_writable_all_protocol( lws_get_context( wsi ), lws_get_protocol( wsi ) );
		GLOBAL_DO_SEND_DATA=1;
	}
	
	return 0;
}

int send_global_message_v(struct lws *wsi, Message_type mtype, const char *data, va_list args)
{
	char *sdata=NULL;
	size_t len=vasprintf(&sdata,data,args);
	
	int ret=send_global_data(wsi,mtype,sdata,len,TRUE);
	
	return ret;
}

int send_global_message(struct lws *wsi, Message_type mtype, const char *data, ...)
{
	va_list args;

	va_start(args, data);

	int ret = send_global_message_v(wsi,mtype,data, args);

	va_end(args);

	return ret;
}

/// SEND USER DATA

int init_user(struct lws *wsi,void *user_id,void *user)
{
	if(GLOBAL_USER_SEND_TABLE==NULL)
	{
		GLOBAL_USER_SEND_TABLE=g_hash_table_new_full(g_direct_hash,g_direct_equal,NULL,(GDestroyNotify)user_info_free);
	}
	
	User_info *cdata=user_info_new(user_id,user);
	
	g_hash_table_insert(GLOBAL_USER_SEND_TABLE,wsi,cdata);
	
	return 0;
}

int deinit_user(struct lws *wsi)
{
	if(GLOBAL_USER_SEND_TABLE==NULL)
	{
		return 1;
	}
	
	if(g_hash_table_remove(GLOBAL_USER_SEND_TABLE,wsi)==FALSE)
	{
		return 2;
	}
	
	return 0;
}

int send_user_data_from_struct(User_info *user, struct lws *wsi, Message_type mtype, const char *data, size_t len, uint8_t free_data)
{
	if(GLOBAL_USER_SEND_TABLE==NULL)
	{
		return 1;
	}
	
	if(user->send_array==NULL)
	{
		user->send_array=g_array_sized_new(FALSE,FALSE,sizeof(Chat_memory),3);
		g_array_set_clear_func(user->send_array,(GDestroyNotify)chat_memory_finalize);
	}
	
	Chat_memory cdata;
	
	chat_memory_init(&cdata,data,len,mtype,free_data);
	
	g_array_append_val(user->send_array,cdata);
	
	if(user->send_data==0)
	{
		lws_callback_on_writable(wsi);
		user->send_data=1;
	}
	
	return 0;
}

int send_user_data(struct lws *wsi, Message_type mtype, const char *data, size_t len, uint8_t free_data)
{
	if(GLOBAL_USER_SEND_TABLE==NULL)
	{
		return 1;
	}
	
	void *orig_key;
	void *orig_val;
	
	if(g_hash_table_lookup_extended(GLOBAL_USER_SEND_TABLE,wsi,&orig_key,&orig_val)==FALSE)
	{
		return 2;
	}
	
	User_info *useri=orig_val;
	int ret=send_user_data_from_struct(useri,wsi,mtype,data,len,free_data);
	
	return ret;
}

int send_user_message_v(struct lws *wsi, Message_type mtype, const char *data, va_list args)
{
	char *sdata=NULL;
	size_t len=vasprintf(&sdata,data,args);
	
	int ret=send_user_data(wsi,mtype,sdata,len,TRUE);
	
	return ret;
}

int send_user_message(struct lws *wsi, Message_type mtype, const char *data, ...)
{
	va_list args;

	va_start(args, data);

	int ret = send_user_message_v(wsi, mtype, data, args);

	va_end(args);

	return ret;
}

/// SEND DATA

int send_global_data_from_id(void *user_id, Message_type mtype, const char *data, size_t len, uint8_t free_data)
{
	//iterate users and insert to user, if matching id
	
	void _itr(gpointer key, gpointer value, gpointer user_data)
	{
		User_info *user=value;
		struct lws *wsi=key;
		
		if(user==NULL)
		{
			return;
		}
		
		if(user->user_id==user_id)
		{
			send_user_data_from_struct(user,wsi,mtype,data,len,free_data);
		}
	}
	
	g_hash_table_foreach(GLOBAL_USER_SEND_TABLE,_itr,NULL);
	
	return 0;
}

int send_global_message_from_id_v(void *user_id, Message_type mtype, const char *data, va_list args)
{
	char *sdata=NULL;
	size_t len=vasprintf(&sdata,data,args);
	
	int ret=send_global_data_from_id(user_id,mtype,sdata,len,TRUE);
	
	return ret;
}

int send_global_message_from_id(void *user_id, Message_type mtype, const char *data, ...)
{
	va_list args;

	va_start(args, data);

	int ret = send_global_message_from_id_v(user_id,mtype,data, args);

	va_end(args);

	return ret;
}

static Chat_memory *send_global_messages(User_info *useri, struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len,int *is_done_after)
{
	if(GLOBAL_SEND_TO_ALL_MEMORY==NULL || GLOBAL_USER_SEND_TABLE==NULL || useri==NULL)
	{
		return NULL;
	}

	//printf("SENDING:: %d %d\n",useri->current_global_message,GLOBAL_SEND_TO_ALL_MEMORY->len);

	if(useri->current_global_message>=GLOBAL_SEND_TO_ALL_MEMORY->len)
	{
		return NULL;
	}
	
	if(useri->current_global_message+1>GLOBAL_SEND_TO_ALL_MEMORY->len)
	{
		*is_done_after=1;
	}
	else
	{
		*is_done_after=0;
	}

	return &g_array_index(GLOBAL_SEND_TO_ALL_MEMORY, Chat_memory, useri->current_global_message);

/*	char *real_data=malloc(LWS_SEND_BUFFER_PRE_PADDING+memory->len+LWS_SEND_BUFFER_POST_PADDING);*/
/*	*/
/*	memcpy(real_data+LWS_SEND_BUFFER_PRE_PADDING,memory->data,memory->len);*/
/*	*/
/*	//printf("SENDINGS:: %s %ld\n",memory->data,memory->len);*/
/*	*/
/*	lws_write( wsi, (unsigned char*)&real_data[LWS_SEND_BUFFER_PRE_PADDING], memory->len, LWS_WRITE_TEXT );*/
/*	memory->history=1;*/
/*	*/
/*	free(real_data);*/
/*	*/
/*	useri->current_global_message++;*/
/*	GLOBAL_DO_SEND_DATA=0;*/
/*	*/
/*	if(useri->current_global_message<GLOBAL_SEND_TO_ALL_MEMORY->len)*/
/*	{*/
/*		Chat_memory *memory_next=&g_array_index(GLOBAL_SEND_TO_ALL_MEMORY, Chat_memory, useri->current_global_message);*/
/*		*/
/*		if(memory_next->history==0)*/
/*		{*/
/*			lws_callback_on_writable_all_protocol( lws_get_context( wsi ), lws_get_protocol( wsi ) );*/
/*		}*/
/*		else*/
/*		{*/
/*			lws_callback_on_writable(wsi);*/
/*		}*/
/*		*/
/*		return -2;*/
/*	}*/
/*	*/
/*	return -1;*/
}

static Chat_memory *send_user_messages(User_info *useri,struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len,int *is_done_after)
{
	if(GLOBAL_USER_SEND_TABLE==NULL || useri==NULL)
	{
		return NULL;
	}
	
	if(useri->send_array==0)
	{
		return NULL;
	}
	
	GArray *useria=useri->send_array;
	
	//printf("SENDING:: %d %d\n",useri->current_user_message,useria->len);

	if(useri->current_user_message>=useria->len)
	{
		return NULL;
	}
	
	if(useri->current_user_message+1>useria->len)
	{
		*is_done_after=1;
	}
	else
	{
		*is_done_after=0;
	}

	return &g_array_index(useria, Chat_memory, useri->current_user_message);

/*	char *real_data=malloc(LWS_SEND_BUFFER_PRE_PADDING+memory->len+LWS_SEND_BUFFER_POST_PADDING+1);*/
/*	*/
/*	memcpy(real_data+LWS_SEND_BUFFER_PRE_PADDING,memory->data,memory->len);*/
/*	*/
/*	lws_write( wsi, (unsigned char*)&real_data[LWS_SEND_BUFFER_PRE_PADDING], memory->len, LWS_WRITE_TEXT );*/
/*	memory->history=1;*/
/*	*/
/*	free(real_data);*/
/*	*/
/*	useri->current_user_message++;*/
/*	useri->send_data=0;*/
/*	*/
/*	if(useri->current_user_message<useria->len)*/
/*	{*/
/*		//Chat_memory *memory_next=&g_array_index(useria, Chat_memory, useri->current_user_message);*/
/*		*/
/*		lws_callback_on_writable(wsi);*/
/*		*/
/*		return -2;*/
/*	}*/
/*	*/
/*	return -1;*/
}


static int do_send_message(Chat_memory *memory,struct lws *wsi)
{
	char *real_data=malloc(LWS_SEND_BUFFER_PRE_PADDING+memory->len+LWS_SEND_BUFFER_POST_PADDING+1);
	
	memcpy(real_data+LWS_SEND_BUFFER_PRE_PADDING,memory->data,memory->len);
	
	lws_write( wsi, (unsigned char*)&real_data[LWS_SEND_BUFFER_PRE_PADDING], memory->len, LWS_WRITE_TEXT );
	memory->history=1;
	
	free(real_data);
	
	return 0;
}

static int do_send_global_message(User_info *useri, Chat_memory *memory, struct lws *wsi)
{
	do_send_message(memory,wsi);
	
	if(memory->mtype==SEND_ERASE)
	{
		if(GLOBAL_USER_SEND_TABLE_FREE_POS>=g_hash_table_size(GLOBAL_USER_SEND_TABLE)-1)
		{
			GLOBAL_SEND_TO_ALL_MEMORY=g_array_remove_index(GLOBAL_SEND_TO_ALL_MEMORY,useri->current_global_message);
			GLOBAL_USER_SEND_TABLE_FREE_POS=0;
		}
		else
		{
			GLOBAL_USER_SEND_TABLE_FREE_POS++;
		}
	}
	else
	{
		useri->current_global_message++;
	}
	GLOBAL_DO_SEND_DATA=0;
	
	return 0;
}

static int do_send_user_message(User_info *useri, Chat_memory *memory, struct lws *wsi)
{
	do_send_message(memory,wsi);

	if(memory->mtype==SEND_ERASE)
	{
		useri->send_array=g_array_remove_index(useri->send_array,useri->current_user_message);
	}
	else
	{
		useri->current_user_message++;
	}
	useri->send_data=0;
	
	return 0;
}

int send_messages(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{

	int user_messages_done_after=-1;
	int global_messages_done_after=-1;

	void *orig_key;
	void *orig_val;
	
	if(g_hash_table_lookup_extended(GLOBAL_USER_SEND_TABLE,wsi,&orig_key,&orig_val)==FALSE)
	{
		return 1;
	}
	
	User_info *useri=orig_val;

	Chat_memory *user_message=send_user_messages(useri, wsi, reason, user, in, len, &user_messages_done_after);
	Chat_memory *global_message=send_global_messages(useri, wsi, reason, user, in, len, &global_messages_done_after);
	
	printf("DONE AFTER %d %d\n",user_messages_done_after,global_messages_done_after);
	
	if(user_message && global_message)
	{
		if(user_message->time <= global_message->time)
		{
			//send user message
			do_send_user_message(useri,user_message,wsi);
		}
		else
		{
			//send global message
			do_send_global_message(useri,global_message,wsi);
		}
	}
	else if(user_message)
	{
		do_send_user_message(useri,user_message,wsi);
	}
	else if(global_message)
	{
		do_send_global_message(useri,global_message,wsi);
	}
	else
	{
		return 0;
	}
	
	if(user_messages_done_after==0 || global_messages_done_after==0)
	{
		Chat_memory *memory_next=NULL;
	
		if(global_messages_done_after==0)
		{
			memory_next=&g_array_index(GLOBAL_SEND_TO_ALL_MEMORY, Chat_memory, useri->current_global_message);
		}
		
		if(memory_next && memory_next->history==0)
		{
			lws_callback_on_writable_all_protocol( lws_get_context( wsi ), lws_get_protocol( wsi ) );
		}
		else
		{
			lws_callback_on_writable(wsi);
		}
	}
	
	return 0;
}
