#ifndef CONFIG_PARSER_H
#define CONFIG_PARSER_H

#include "std_types.h"

#define CFG_LINE_LEN     32U
#define CFG_MAX_ERRORS   5U
#define CFG_MAX_LINES    64U
#define CFG_TRIM_MAX     50
#define CFG_FLAG_DIRTY   0x01U
#define CFG_FLAG_LOCKED  0x04U

typedef enum {
    DRIVE_ECO = 0,
    DRIVE_NORMAL = 1,
    DRIVE_SPORT = 2
} DriveMode;

void        cfg_init(void);
DriveMode   cfg_mode_from_raw(u8 raw);
DriveMode   cfg_mode_from_raw_checked(u8 raw);
u16         cfg_mode_speed_limit(DriveMode m);
u16         cfg_mode_speed_limit_safe(DriveMode m);
const char *cfg_mode_name(DriveMode m);
const char *cfg_mode_name_safe(DriveMode m);
bool        cfg_parse_line(const char *line);
bool        cfg_is_locked(void);
bool        cfg_is_dirty(void);
s16         cfg_trim(void);
s16         cfg_set_trim(s32 value);
bool        cfg_check_length(s32 parsed, u32 max_len);
bool        cfg_check_length_safe(s32 parsed, u32 max_len);
const char *cfg_value_ptr(u32 key_len);
u32         cfg_count_digits(const char *s, u32 len);
void        cfg_reset(u8 reason);
void        cfg_reset_safe(u8 reason);
u8          cfg_errors(void);

#endif /* CONFIG_PARSER_H */
