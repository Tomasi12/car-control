#ifndef LOGGER_H
#define LOGGER_H

#include "std_types.h"

#define LOG_LINE_LEN 32U
#define LOG_LINES    8U
#define LOG_TAG_LEN  8U
#define LOG_NAME_LEN 12U

void        log_init(void);
void        log_set_tag(const char *tag);
void        log_set_tag_safe(const char *tag);
void        log_event(const char *name, u32 tick);
void        log_temperature(float temp_c);
void        log_temperature_safe(float temp_c);
void        log_text(const char *msg);
void        log_text_safe(const char *msg);
void        log_tagged(const char *msg);
void        log_tagged_safe(const char *msg);
void        log_drop_oldest(void);
void        log_drop_oldest_safe(void);
u32         log_count(void);
const char *log_line(u32 index);

#endif /* LOGGER_H */
