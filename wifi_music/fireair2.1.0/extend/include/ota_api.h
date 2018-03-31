
#ifndef _OTA_API_H_
#define _OTA_API_H_

extern void flash_burn(void);
extern int ota_update(update_info *info);
extern int ota_check_firm_version(update_info *info);

extern void ota_set_external_callback(void *ffc);

#endif

