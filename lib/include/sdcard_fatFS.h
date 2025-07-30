#ifndef FATFS_H
#define FATFS_H

#include "sd_card.h"
#include "ff.h"
#include "diskio.h"
#include "f_util.h"
#include "hw_config.h"
#include "my_debug.h"
#include "rtc.h"
#include "sd_card.h"

static sd_card_t *sd_get_by_name(const char *const name);

static FATFS *sd_get_fs_by_name(const char *name);

static void run_setrtc();

static void run_format();

int8_t run_mount();

int8_t run_unmount();

static void run_getfree();

static void run_ls();

static void run_cat();

void save_data_to_sd(const char *filename, const char *data);

void read_file(const char *filename);

#endif // FATFS_H