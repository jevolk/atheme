/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code are whatever Mr. Pitcock wants idc.
 */


template<class T = void>
void for_each(mowgli_list_t &list,
              const std::function<void (T &)> &closure)
{
	mowgli_node_t *n;
	MOWGLI_LIST_FOREACH(n, list.head)
		closure(*reinterpret_cast<T *>(n->data));
}


template<class Key = void,
         class T = void>
void for_each(mowgli_dictionary_t *const &dict,
              const std::function<void (const Key *const &, T *const &)> &closure)
{
	mowgli_dictionary_elem_t elem;
	mowgli_dictionary_iteration_state_t state;
	MOWGLI_DICTIONARY_FOREACH(&elem, &state, dict)
		closure(reinterpret_cast<const Key *>(elem.key), reinterpret_cast<T *>(elem.data));
}


template<class T = void>
void for_each(mowgli_dictionary_t *const &dict,
              const std::function<void (T *const &)> &closure)
{
	mowgli_dictionary_elem_t elem;
	mowgli_dictionary_iteration_state_t state;
	MOWGLI_DICTIONARY_FOREACH(&elem, &state, dict)
		closure(reinterpret_cast<T *>(elem.data));
}


template<class Key = void,
         class T = void>
void for_each(mowgli_patricia_t *const &dict,
              const std::function<void (const Key *const &, T *const &)> &closure)
{
	void *elem;
	mowgli_patricia_iteration_state_t state;
	MOWGLI_PATRICIA_FOREACH(elem, &state, dict)
		closure(reinterpret_cast<const Key *>(state.pspare[0]), reinterpret_cast<T *>(elem));
}


template<class T = void>
void for_each(mowgli_patricia_t *const &dict,
              const std::function<void (T *const &)> &closure)
{
	T *elem;
	mowgli_patricia_iteration_state_t state;
	MOWGLI_PATRICIA_FOREACH((reinterpret_cast<void *&>(elem)), &state, dict)
		closure(elem);
}


template<class T = v8::Value>
void for_each(const v8::Local<v8::Context> &ctx,
              const v8::Local<v8::Object> &obj,
              const std::function<void (v8::Local<T>)> &closure)
{
	for(size_t i(0); has(ctx, obj, i); ++i)
		closure(get<T>(ctx, obj, i));
}


template<class T = v8::Value>
void for_each(const v8::Local<v8::Object> &obj,
              const std::function<void (v8::Local<T>)> &closure)
{
	for_each<T>(ctx(), obj, closure);
}


template<class T = v8::Value>
void for_each(const v8::Local<v8::Context> &ctx,
              const v8::Local<v8::Object> &obj,
              const std::function<void (v8::Local<v8::Name> &key, v8::Local<T> val)> &closure)
{
	keys(ctx, obj, [&ctx, &obj, &closure]
	(v8::Local<v8::Name> key)
	{
		closure(key, get<T>(ctx, obj, key));
	});
}


template<class T = v8::Value>
void for_each(const v8::Local<v8::Object> &obj,
              const std::function<void (v8::Local<v8::Name> &key, v8::Local<T> val)> &closure)
{
	for_each<T>(ctx(), obj, closure);
}


template<class T = v8::Value>
void for_each_own(const v8::Local<v8::Context> &ctx,
                  const v8::Local<v8::Object> &obj,
                  const std::function<void (v8::Local<v8::Name> &key, v8::Local<T> val)> &closure)
{
	keys_own(ctx, obj, [&ctx, &obj, &closure]
	(v8::Local<v8::Name> key)
	{
		closure(key, get<T>(ctx, obj, key));
	});
}


template<class T = v8::Value>
void for_each_own(const v8::Local<v8::Object> &obj,
                  const std::function<void (v8::Local<v8::Name> &key, v8::Local<T> val)> &closure)
{
	for_each_own<T>(ctx(), obj, closure);
}
