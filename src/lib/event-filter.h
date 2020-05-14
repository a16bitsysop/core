#ifndef EVENT_FILTER_H
#define EVENT_FILTER_H

struct event;

struct event_filter_field {
	const char *key;
	const char *value;
};

struct event_filter_query {
	/* NULL-terminated list of categories */
	const char *const *categories;
	/* key=NULL-terminated list of key=value fields */
	const struct event_filter_field *fields;

	/* event name. Supports '*' and '?' wildcards. */
	const char *name;
	/* source filename:line */
	const char *source_filename;
	unsigned int source_linenum;

	/* context associated with this query. This is returned when iterating
	   through matched queries. If NULL, the iteration won't return this
	   query. */
	void *context;
};

struct event_filter *event_filter_create(void);
struct event_filter *event_filter_create_fragment(pool_t pool);
void event_filter_ref(struct event_filter *filter);
void event_filter_unref(struct event_filter **filter);

/* Add a new query to the filter. All of the categories and fields in the query
   need to match, so they're ANDed together. Separate queries are ORed
   together. */
void event_filter_add(struct event_filter *filter,
		      const struct event_filter_query *query);
/* Add queries from source filter to destination filter. */
void event_filter_merge(struct event_filter *dest,
			const struct event_filter *src);
/* Add queries from source filter to destination filter, but with supplied
   context overriding whatever context source queries had. */
void event_filter_merge_with_context(struct event_filter *dest,
				     const struct event_filter *src,
				     void *new_context);

/* Export the filter into a tabescaped string, so its fields are separated
   with TABs and there are no NUL, CR or LF characters. The context pointers
   aren't exported. */
void event_filter_export(struct event_filter *filter, string_t *dest);
/* Add queries to the filter from the given string. The string is expected to
   be generated by event_filter_export(). Returns TRUE on success, FALSE on
   invalid string. */
bool event_filter_import(struct event_filter *filter, const char *str,
			 const char **error_r);
/* Same as event_filter_import(), but string is already split into an array
   of strings via *_strsplit_tabescaped(). */
bool event_filter_import_unescaped(struct event_filter *filter,
				   const char *const *args,
				   const char **error_r);

/* Parse a string-ified query */
int event_filter_parse(const char *str, struct event_filter **filter_r,
		       const char **error_r);

/* Returns TRUE if the event matches the event filter. */
bool event_filter_match(struct event_filter *filter, struct event *event,
			const struct failure_context *ctx);
/* Same as event_filter_match(), but use the given source filename:linenum
   instead of taking it from the event. */
bool event_filter_match_source(struct event_filter *filter, struct event *event,
			       const char *source_filename,
			       unsigned int source_linenum,
			       const struct failure_context *ctx);

/* Iterate through all queries with non-NULL context that match the event. */
struct event_filter_match_iter *
event_filter_match_iter_init(struct event_filter *filter, struct event *event,
			     const struct failure_context *ctx);
/* Return context for the query that matched, or NULL when there are no more
   matches.  Note: This skips over any queries that have NULL context. */
void *event_filter_match_iter_next(struct event_filter_match_iter *iter);
void event_filter_match_iter_deinit(struct event_filter_match_iter **iter);

void event_filter_init(void);
void event_filter_deinit(void);

#endif
