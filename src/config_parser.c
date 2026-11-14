/* Config parser: reads "key=value" lines sent by the end-of-line tester,
 * e.g. "mode=2" or "trim=-12". Bad lines are counted; too many errors lock the config. */
#include <string.h>

#include "config_parser.h"

static DriveMode g_drive_mode;
static u8        g_cfg_flags;
static s16       g_trim;
static u8        g_cfg_errors;
static u32       g_lines_seen;
static char     cfg_line[CFG_LINE_LEN];

void cfg_init(void)
{
    g_drive_mode = DRIVE_NORMAL;
    g_cfg_flags = 0U;
    g_trim = 010; /* RULE(octal_constant): 010 is octal (eight), probably ten was meant */
    g_cfg_errors = 0U;
    (void)memset(cfg_line, 0, sizeof(cfg_line));
}

DriveMode cfg_mode_from_raw(u8 raw)
{
    return (DriveMode)raw; /* DEFECT(invalid_enum): raw from the tester can be above DRIVE_SPORT */
}

DriveMode cfg_mode_from_raw_checked(u8 raw)
{
    if (raw > (u8)DRIVE_SPORT) {
        return DRIVE_NORMAL;
    }
    return (DriveMode)raw; /* SAFE(invalid_enum): range checked above */
}

u16 cfg_mode_speed_limit(DriveMode m)
{
    switch (m) {
    case DRIVE_ECO:
        return 900U;
    case DRIVE_NORMAL:
        return 1300U;
    case DRIVE_SPORT:
        return 1600U;
    default:
        break;
    }
} /* DEFECT(missing_return): an out-of-range mode reaches the end without a return value */

u16 cfg_mode_speed_limit_safe(DriveMode m)
{
    u16 limit;

    if (m == DRIVE_ECO) {
        limit = 900U;
    } else if (m == DRIVE_SPORT) {
        limit = 1600U;
    } else {
        limit = 1300U;
    }
    return limit; /* SAFE(missing_return): every path sets limit and reaches this return */
}

const char *cfg_mode_name(DriveMode m)
{
    const char *name;

    if (m == DRIVE_ECO) {
        name = "eco";
    } else if (m == DRIVE_NORMAL) {
        name = "normal";
    } else if (m == DRIVE_SPORT) {
        name = "sport";
    }
    return name; /* DEFECT(uninit_ptr): name is not set for an out-of-range mode */
}

const char *cfg_mode_name_safe(DriveMode m)
{
    const char *name;

    switch (m) {
    case DRIVE_ECO:
        name = "eco";
        break;
    case DRIVE_SPORT:
        name = "sport";
        break;
    default:
        name = "normal";
        break;
    }
    return name; /* SAFE(uninit_ptr): the default clause sets name */
}

bool cfg_is_locked(void)
{
    return g_cfg_flags & CFG_FLAG_LOCKED != 0U; /* DEFECT(operator_precedence): != binds tighter than &, so bit 0 is tested */
}

bool cfg_is_dirty(void)
{
    return (g_cfg_flags & CFG_FLAG_DIRTY) != 0U; /* SAFE(operator_precedence): parentheses make the order explicit */
}

s16 cfg_trim(void)
{
    return g_trim;
}

s16 cfg_set_trim(s32 value)
{
    if (value > CFG_TRIM_MAX); /* DEFECT(empty_body): stray semicolon, the clamp below always runs */
    {
        value = CFG_TRIM_MAX;
    }
    g_trim = value; /* RULE(implicit_conversion): s32 assigned to s16 without a cast */
    g_cfg_flags |= CFG_FLAG_DIRTY;
    return (s16)(g_trim >> 1); /* RULE(signed_bitwise): right shift of a signed value */
}

bool cfg_check_length(s32 parsed, u32 max_len)
{
    return parsed <= max_len; /* RULE(signed_unsigned_compare): s32 compared with u32, a negative value becomes huge */
}

bool cfg_check_length_safe(s32 parsed, u32 max_len)
{
    return (parsed >= 0) && ((u32)parsed <= max_len); /* SAFE(signed_unsigned_compare): sign checked before the cast */
}

const char *cfg_value_ptr(u32 key_len)
{
    return &cfg_line[0] + key_len + 1U; /* DEFECT(ptr_arith): key_len >= CFG_LINE_LEN gives a pointer past the end of cfg_line */
}

u32 cfg_count_digits(const char *s, u32 len)
{
    char copy[len + 1U]; /* RULE(vla): variable-length array on the stack */
    u32  n = 0U;
    u32  i;

    (void)memcpy(copy, s, len);
    copy[len] = '\0';
    for (i = 0U; copy[i] != '\0'; i++) {
        if ((copy[i] >= '0') && (copy[i] <= '9')) /* RULE(missing_braces): if body is not a compound statement */
            n++;
    }
    return n;
}

static s32 cfg_parse_int(const char *s, bool *ok)
{
    s32  v = 0;
    bool neg = false;

    *ok = false;
    if (*s == '-') {
        neg = true;
        s++;
    }
    while ((*s >= '0') && (*s <= '9')) {
        if (v > 9999) {
            return 0;
        }
        v = (v * 10) + (*s - '0');
        s++;
        *ok = true;
    }
    if (*s != '\0') {
        *ok = false;
    }
    return neg ? -v : v;
}

bool cfg_parse_line(const char *line)
{
    const char *eq;
    u32         len = (u32)strlen(line);
    s32         value;
    bool        ok;
    u32         checksum = 0U; /* RULE(unused_variable): checksum is never used */

    if ((len == 0U) || (g_lines_seen++ >= CFG_MAX_LINES)) { /* RULE(side_effect_in_condition): the increment in the right operand of || is skipped for empty lines */
        return false;
    }
    if (g_cfg_errors >= CFG_MAX_ERRORS) {
        g_cfg_flags |= CFG_FLAG_LOCKED;
        return false;
    }
    if ((eq = strchr(line, '=')) == NULL) { /* RULE(assignment_in_condition): assignment inside the if condition */
        g_cfg_errors++;
        return false;
    }
    if (len >= CFG_LINE_LEN) {
        g_cfg_errors++;
        return false;
    }
    (void)memcpy(cfg_line, line, len + 1U);
    {
        u32 len = (u32)(eq - line); /* RULE(shadowing): hides the outer len */
        value = cfg_parse_int(eq + 1, &ok);
        if (!ok) {
            g_cfg_errors++;
            return false;
        }
        if ((len == 4U) && (strncmp(line, "mode", 4U) == 0)) {
            g_drive_mode = cfg_mode_from_raw_checked((u8)value);
        } else if ((len == 4U) && (strncmp(line, "trim", 4U) == 0)) {
            if ((value >= -CFG_TRIM_MAX) && (value <= CFG_TRIM_MAX)) {
                g_trim = (s16)value;
                g_cfg_flags |= CFG_FLAG_DIRTY;
            }
        } else {
            g_cfg_errors++;
            return false;
        }
    }
    return true;
}

void cfg_reset(u8 reason) /* RULE(unused_parameter): reason is never used */
{
    cfg_init();
}

void cfg_reset_safe(u8 reason)
{
    (void)reason; /* SAFE(unused_parameter): explicitly marked as unused */
    cfg_init();
}

u8 cfg_errors(void)
{
    return g_cfg_errors;
}
