/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */


void chan_notice(mychan_t *chan, const char *msg);
uint chan_access_to_chanacs(const access_t *acc);
void chan_access_sync(mychan_t *chan, access_t *acc);
void chan_clear_acs(mychan_t *chan);
void chan_drop(mychan_t *chan);


inline
void chan_drop(mychan_t *const chan)
{
	hook_call_channel_drop(chan);

//	if(chan->chan != NULL)
//		part(chan->name, myservice->nick);

	object_unref(chan);
}


inline
void chan_clear_acs(mychan_t *const chan)
{
	mowgli_node_t *n, *tn;
	MOWGLI_ITER_FOREACH_SAFE(n, tn, chan->chanacs.head)
		object_unref(n->data);
}


inline
void chan_access_sync(mychan_t *const chan,
                      access_t *const acc)
{
	chanacs_t *acs = chanacs_find(chan, entity(acc->user), 0);
	uint level = chan_access_to_chanacs(acc);
	if(!level && !acs)
		return;

	if(acs)
	{
		uint remove = 0;
		chanacs_modify(acs, &level, &remove, ca_all);
		return;
	}

	chanacs_add(chan, entity(acc->user), level, CURRTIME, entity(myservice->me->myuser));
}


inline
uint chan_access_to_chanacs(const access_t *const acc)
{
	uint ret = 0;
	if(acc->mask & GA_FOUNDER)    ret |= CA_FOUNDER;
	if(acc->mask & GA_SUCCESSOR)  ret |= CA_SUCCESSOR_0;
	if(acc->mask & GA_CONTACT)    ret |= CA_ACLVIEW | CA_VOICE | CA_TOPIC;
	if(acc->mask & GA_FLAGS)      ret |= CA_FLAGS;
	if(acc->mask & GA_CHANADMIN)  ret |= -1 & ~(CA_FOUNDER|CA_SUCCESSOR_0|CA_AKICK);
	if(acc->mask & GA_USERADMIN)  ret |= CA_ACLVIEW;
	if(acc->mask & GA_OPERATOR)   ret |= CA_OP;
	if(acc->mask & GA_INVITE)     ret |= CA_INVITE;
	if(acc->mask & GA_CLOAK)      ret |= CA_VOICE;
	return ret;
}


inline
void chan_notice(mychan_t *const chan,
                 const char *const msg)
{
	notice(myservice->me->nick, chan->name, msg);
}
