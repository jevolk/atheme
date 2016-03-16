/*
 * Copyright (C) 2016 Jason Volk
 *
 * Rights to this code inherit the 'Atheme' project license by the 'Atheme Development Group'
 * gigantium humeris insidentes
 */

typedef enum
{
	GM_APPROVED     = 0x0001,
	GM_LISTED       = 0x0002,
	GM_PRIVATE      = 0x0004,
}
mode_flag_t;

struct mode_flag
{
	mode_flag_t flag;
	const char *reflection;
}
const *mode_table; // gms.c

static inline
mode_flag_t mode_char_to_flag(const char letter)
{
	return mode_table[letter].flag;
}

static inline
const char *mode_char_reflect(const char letter)
{
	return mode_table[letter].reflection;
}

mask_vtable_t mode_vtable =
{
	NULL,
	mode_char_to_flag,
	NULL,
	mode_char_reflect,
};

// Convenience utilities encapsulating the mask_ functions for modes
void mode_mask_to_str(const uint mask, char *buf);
uint mode_str_to_mask(const char *str);
uint mode_mask_delta(const char *delta, uint *mask);


inline
uint mode_mask_delta(const char *const str,
                     uint *const mask)
{
	return mask_delta(&mode_vtable, str, mask);
}


inline
uint mode_str_to_mask(const char *const str)
{
	return mask_from_str(&mode_vtable, str);
}


inline
void mode_mask_to_str(const uint mask,
                      char *const buf)
{
	mask_to_str(&mode_vtable, mask, buf);
}
