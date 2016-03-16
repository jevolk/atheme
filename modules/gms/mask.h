/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

typedef struct mask_vtable
{
	char (*flag_to_char)(const uint);
	uint (*char_to_flag)(const char);
	const char *(*flag_reflect)(const uint);
	const char *(*char_reflect)(const char);
}
mask_vtable_t;

size_t mask_reflect(const mask_vtable_t *vt, const uint mask, char *buf, const size_t max);
uint mask_char_to_flag(const mask_vtable_t *vt, const char letter);
char mask_flag_to_char(const mask_vtable_t *vt, const uint flag);
void mask_to_str(const mask_vtable_t *vt, const uint mask, char *buf);
uint mask_from_str(const mask_vtable_t *vt, const char *str);
uint mask_delta(const mask_vtable_t *vt, const char *delta, uint *mask);


inline
uint mask_delta(const mask_vtable_t *const vt,
                const char *str,
                uint *const mask)
{
	int what = 1;
	for(; *str; ++str) switch(*str)
	{
		case '+':  what = 1;  continue;
		case '-':  what = 0;  continue;
		default:
			if(what)
				*mask |= mask_char_to_flag(vt, *str);
			else
				*mask &= ~mask_char_to_flag(vt, *str);

			continue;
	}

	return *mask;
}


inline
uint mask_from_str(const mask_vtable_t *const vt,
                   const char *str)
{
	uint mask = 0;
	for(; *str; ++str)
		mask |= mask_char_to_flag(vt, *str);

	return mask;
}


inline
void mask_to_str(const mask_vtable_t *const vt,
                 const uint mask,
                 char *buf)
{
	for(uint i = 1; i; i <<= 1)
	{
		if(~mask & i)
			continue;

		*buf = vt->flag_to_char(i);
		buf++;
	}

	*buf = '\0';
}


inline
uint mask_char_to_flag(const mask_vtable_t *const vt,
                       const char letter)
{
	if(vt->char_to_flag)
		return vt->char_to_flag(letter);

	for(uint i = 1; i; i <<= 1)
		if(vt->flag_to_char(i) == letter)
			return i;
}


inline
char mask_flag_to_char(const mask_vtable_t *const vt,
                       const uint flag)
{
	if(vt->flag_to_char)
		return vt->flag_to_char(flag);

	for(int i = 0; i < 256; i++)
		if(vt->char_to_flag(i) == flag)
			return i;

	return '\0';
}


inline
size_t mask_reflect(const mask_vtable_t *const vt,
                    const uint mask,
                    char *const buf,
                    const size_t max)
{
	size_t ret = 0;

	if(!max)
		return ret;

	buf[0] = '\0';
	for(uint i = 1; i; i <<= 1)
	{
		if(~mask & i)
			continue;

		const char *r;
		if(vt->flag_reflect)
			r = vt->flag_reflect(i);
		else if(vt->char_reflect)
			r = vt->char_reflect(mask_flag_to_char(vt,i));
		else
			r = NULL;

		if(r)
		{
			mowgli_strlcat(buf, r, max);
			ret = mowgli_strlcat(buf, " ", max);
		}
	}

	if(ret)
		buf[--ret] = '\0';

	return ret;
}
