#include "tp_player.h"
#include "tp_questions.h"

TPPlayer *tp_player_init(TPPlayer *self, guint32 player_id)
{
	memset(self,'\0',sizeof(TPPlayer));

	self->player_id=player_id;
	//self->qa=g_array_new(FALSE,FALSE,sizeof(TPQa));

	printf("pointer made %p\n",self->qa);

	return self;
}
