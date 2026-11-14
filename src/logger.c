/* Text event logger: formats short messages into fixed-size lines and keeps the newest LOG_LINES.
 * The service tool reads the lines over CAN. Some messages come from the tester. */
#include <stdio.h>
#include <string.h>

#include "logger.h"

static char log_lines[LOG_LINES][LOG_LINE_LEN];
static u32  log_used;
static char log_tag[LOG_TAG_LEN];

void log_init(void)
{
    (void)memset(log_lines, 0, sizeof(log_lines));
    (void)memset(log_tag, 0, sizeof(log_tag));
    log_used = 0U;
}

static char *log_next_line(void)
{
    if (log_used == LOG_LINES) {
        log_drop_oldest_safe();
    }
    log_used++;
    return log_lines[log_used - 1U];
}

void log_set_tag(const char *tag)
{
    (void)strcpy(log_tag, tag); /* DEFECT(string_overflow): a tag of 8 or more characters overflows log_tag */
}

void log_set_tag_safe(const char *tag)
{
    (void)strncpy(log_tag, tag, LOG_TAG_LEN - 1U); /* SAFE(missing_null_terminator): the last byte is cleared on the next line */
    log_tag[LOG_TAG_LEN - 1U] = '\0';
}

void log_event(const char *name, u32 tick)
{
    char short_name[LOG_NAME_LEN];

    (void)strncpy(short_name, name, sizeof(short_name)); /* DEFECT(missing_null_terminator): a name of 12 or more characters leaves short_name unterminated */
    (void)snprintf(log_next_line(), LOG_LINE_LEN, "%s@%lu", short_name, (unsigned long)tick);
}

void log_temperature(float temp_c)
{
    (void)snprintf(log_next_line(), LOG_LINE_LEN, "T=%d C", temp_c); /* DEFECT(format_string): %d expects an int, a double is passed */
}

void log_temperature_safe(float temp_c)
{
    (void)snprintf(log_next_line(), LOG_LINE_LEN, "T=%d C", (int)temp_c); /* SAFE(format_string): argument is cast to int */
}

void log_text(const char *msg)
{
    (void)snprintf(log_next_line(), LOG_LINE_LEN, msg); /* DEFECT(uncontrolled_format): msg comes from the tester and may contain % directives */
}

void log_text_safe(const char *msg)
{
    (void)snprintf(log_next_line(), LOG_LINE_LEN, "%s", msg); /* SAFE(uncontrolled_format): msg is an argument, not the format */
}

void log_tagged(const char *msg)
{
    (void)sprintf(log_next_line(), "%s: %s", log_tag, msg); /* DEFECT(string_overflow): tag and message together can exceed LOG_LINE_LEN */
}

void log_tagged_safe(const char *msg)
{
    (void)snprintf(log_next_line(), LOG_LINE_LEN, "%s: %s", log_tag, msg); /* SAFE(string_overflow): snprintf truncates to the line size */
}

void log_drop_oldest(void)
{
    if (log_used > 0U) {
        (void)memcpy(log_lines[0], log_lines[1], (LOG_LINES - 1U) * LOG_LINE_LEN); /* DEFECT(overlapping_copy): source and destination overlap, memcpy is undefined here */
        log_used--;
    }
}

void log_drop_oldest_safe(void)
{
    if (log_used > 0U) {
        (void)memmove(log_lines[0], log_lines[1], (LOG_LINES - 1U) * LOG_LINE_LEN); /* SAFE(overlapping_copy): memmove allows overlap */
        (void)memset(log_lines[LOG_LINES - 1U], 0, LOG_LINE_LEN);
        log_used--;
    }
}

u32 log_count(void)
{
    return log_used;
}

const char *log_line(u32 index)
{
    if (index >= log_used) {
        return "";
    }
    if (strlen(log_lines[index]) > 24U) { /* RULE(magic_number): 24 is an unnamed constant */
        return "(long)";
    }
    return log_lines[index];
}
