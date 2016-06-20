/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


template<class T = void>
void remove_if(mowgli_list_t &list,
               const std::function<bool (T *const &)> &closure)
{
	mowgli_node_t *n, *tn;
	MOWGLI_LIST_FOREACH_SAFE(n, tn, list.head)
		if(closure(reinterpret_cast<T *>(n->data)))
			mowgli_node_delete(n, &list);
}


template<class T = void>
void remove_and_free_if(mowgli_list_t &list,
                        const std::function<bool (T *const &)> &closure)
{
	mowgli_node_t *n, *tn;
	MOWGLI_LIST_FOREACH_SAFE(n, tn, list.head)
		if(closure(reinterpret_cast<T *>(n->data)))
		{
			mowgli_node_delete(n, &list);
			mowgli_node_free(n);
		}
}
