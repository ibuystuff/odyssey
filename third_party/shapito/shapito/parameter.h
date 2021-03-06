#ifndef SHAPITO_PARAMETER_H
#define SHAPITO_PARAMETER_H

/*
 * Shapito.
 *
 * Protocol-level PostgreSQL client library.
*/

typedef struct shapito_parameter  shapito_parameter_t;
typedef struct shapito_parameters shapito_parameters_t;

struct shapito_parameter
{
	uint32_t name_len;
	uint32_t value_len;
	char     data[];
};

struct shapito_parameters
{
	shapito_stream_t buf;
};

static inline char*
shapito_parameter_name(shapito_parameter_t *param)
{
	return param->data;
}

static inline char*
shapito_parameter_value(shapito_parameter_t *param)
{
	return param->data + param->name_len;
}

static inline shapito_parameter_t*
shapito_parameter_next(shapito_parameter_t *param)
{
	return (shapito_parameter_t*)(param->data + param->name_len +
	                              param->value_len);
}

int shapito_parameter_quote(char*, char*, int);

static inline void
shapito_parameters_init(shapito_parameters_t *params)
{
	shapito_stream_init(&params->buf);
}

static inline void
shapito_parameters_free(shapito_parameters_t *params)
{
	shapito_stream_free(&params->buf);
}

static inline int
shapito_parameters_copy(shapito_parameters_t *dest, shapito_parameters_t *src)
{
	int size = shapito_stream_used(&src->buf);
	int rc;
	rc = shapito_stream_ensure(&dest->buf, size);
	if (shapito_unlikely(rc == -1))
		return -1;
	memcpy(dest->buf.pos, src->buf.start, size);
	shapito_stream_advance(&dest->buf, size);
	return 0;
}

int shapito_parameters_add(shapito_parameters_t*,
                           char *name,
                           uint32_t name_len,
                           char *value,
                           uint32_t value_len);

int shapito_parameters_update(shapito_parameters_t*,
                              char *name,
                              uint32_t name_len,
                              char *value,
                              uint32_t value_len);

shapito_parameter_t*
shapito_parameters_find(shapito_parameters_t*, char*, int);

#endif /* SHAPITO_PARAMETER_H */
