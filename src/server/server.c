#include <libwebsockets.h>
#include <string.h>
#include <stdio.h>

#include "sending.h"

#define EXAMPLE_RX_BUFFER_BYTES (100)
const char *GLOBAL_USERNAME="ADMIN";
char GLOBAL_GUESS_MEMORY='\0';

typedef struct Lookup_tbl
{
	const char *file_ext;
	const char *mime;
}Lookup_tbl;

const Lookup_tbl lookup_table[]=
{
	{"html","text/html"},
	{"css","text/css"},
	{"js","text/js"}
};
size_t lookup_tbl_size=sizeof(lookup_table)/sizeof(lookup_table[0]);

static int callback_http( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	switch( reason )
	{
		case LWS_CALLBACK_HTTP:
		{
			if(len>0)
			{
				printf("Requesting file:: %s\n",(char*)in);
				
				char *provide_file=NULL;
				int provide_file_len=0;
				int s=0;
				
				const char *cur_mime=lookup_table[0].mime;
				
				if(len>1)
				{
					char *infile=(char*)in+1;
					
					provide_file_len=asprintf(&provide_file,"./html/%s",infile);
				}
				
				if(provide_file && access(provide_file, F_OK) != -1)
				{
					for(int i=0;i<lookup_tbl_size;i++)
					{
						size_t file_ext_len=strlen(lookup_table[i].file_ext);
					
						if((file_ext_len+1<provide_file_len) && strncmp(provide_file+provide_file_len-file_ext_len,lookup_table[i].file_ext,file_ext_len)==0)
						{
							cur_mime=lookup_table[i].mime;
							break;
						}
					}
					
					s=lws_serve_http_file( wsi, provide_file, cur_mime, NULL, 0 );
				}
				else
				{
					s=lws_serve_http_file( wsi, "./html/index.html", "text/html", NULL, 0 );
				}
				
				if(provide_file)
				{
					free(provide_file);
				}
				
				if (s < 0 || ((s > 0) && lws_http_transaction_completed(wsi)))
				{
					return -1;
				}
			}
			break;
		}
		case LWS_CALLBACK_HTTP_FILE_COMPLETION:
			if (lws_http_transaction_completed(wsi))
			{
				return -1;
			}
		return 1;
		default:
			break;
	}

	return 0;
}

static int callback_example( struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len )
{
	printf("USER:: %p WSI:: %p REASON:: %d\n",user,wsi,reason);

	switch( reason )
	{
		case LWS_CALLBACK_ESTABLISHED:
			init_user(wsi,NULL,wsi);
			send_user_message(wsi, SEND_STORE, "C%-10s%s", GLOBAL_USERNAME, "Hello and welcome!");
			send_user_message(wsi, SEND_STORE, "C%-10s%s", GLOBAL_USERNAME, "Amazing weather isnt it?");
		break;
		case LWS_CALLBACK_CLOSED:
			deinit_user(wsi);
		break;
		case LWS_CALLBACK_RECEIVE:
		{
			char *input=(char*)in;
			
			//for set and get
			if(len==2)
			{
				if(input[0]=='G')
				{
					//compare the box character (num)
					if(input[1]==GLOBAL_GUESS_MEMORY)
					{
						send_global_message(wsi, SEND_ERASE, "C%-10s%s", GLOBAL_USERNAME, "CORRECT GUESS!");
					}
					else
					{
						send_global_message(wsi, SEND_ERASE, "C%-10s%s", GLOBAL_USERNAME, "WRONG GUESS!");
					}
					
					return 0;
				}
				else if(input[0]=='S')
				{
					GLOBAL_GUESS_MEMORY=input[1];
					
					send_global_message(wsi, SEND_STORE, "C%-10s%s", GLOBAL_USERNAME, "BOX IS SET!");
					
					return 0;
				}
			}
			else if(input[0]=='C')
			{
				send_global_data(wsi, SEND_STORE, strndup(input,len),len,TRUE);
			}
			break;
		}
		case LWS_CALLBACK_SERVER_WRITEABLE:
		{
			send_messages(wsi,reason,user,in,len);
			break;
		}
		default:
			break;
	}

	return 0;
}

static struct lws_protocols protocols[] =
{
	/* The first protocol must always be the HTTP handler */
	{
		"http-only",   /* name */
		callback_http, /* callback */
		0,             /* No per session data. */
		0,             /* max frame size / rx buffer */
	},
	{
		"example-protocol",
		callback_example,
		0,
		EXAMPLE_RX_BUFFER_BYTES,
	},
	{ NULL, NULL, 0, 0 } /* terminator */
};

int main( int argc, char *argv[] )
{
	struct lws_context_creation_info info;
	memset( &info, 0, sizeof(info) );

	info.port = 8000;
	info.protocols = protocols;
	info.gid = -1;
	info.uid = -1;

	struct lws_context *context = lws_create_context( &info );

	while( 1 )
	{
		lws_service( context, /* timeout_ms = */ 1000000 );
	}

	lws_context_destroy( context );

	return 0;
}
