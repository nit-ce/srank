/* srank main header*/

/* string buffer, variable-sized string */
struct sbuf *sbuf_make(void);
void sbuf_free(struct sbuf *sb);
char *sbuf_done(struct sbuf *sb);
char *sbuf_buf(struct sbuf *sb);
void sbuf_chr(struct sbuf *sb, int c);
void sbuf_str(struct sbuf *sb, char *s);
void sbuf_mem(struct sbuf *sb, char *s, int len);
void sbuf_printf(struct sbuf *sbuf, char *s, ...);
int sbuf_len(struct sbuf *sb);
void sbuf_cut(struct sbuf *s, int len);

/* string index */
struct sidx *sidx_make(void);
void sidx_free(struct sidx *sidx);
int sidx_get(struct sidx *sidx, char *key);
int sidx_put(struct sidx *sidx, char *key);
char *sidx_str(struct sidx *sidx, int id);
/* managing data */
int sidx_len(struct sidx *sidx);
void *sidx_datget(struct sidx *sidx, int id);
void sidx_datset(struct sidx *sidx, int id, void *dat);
