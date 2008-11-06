#ifndef __SUMMARY_CONFIG_H__
#define __SUMMARY_CONFIG_H__
#include <stdbool.h>
#include <stdlib.h>
#include <enkf_macros.h>
#include <ecl_sum.h>



typedef struct summary_config_struct summary_config_type;

void                   summary_config_set_obs_config_file(summary_config_type * , const char * );
const char           * summary_config_get_config_txt_file_ref(const summary_config_type * );
summary_config_type  * summary_config_fscanf_alloc(const char * , const char * );
summary_config_type  * summary_config_alloc(int , const char ** );
void                   summary_config_free(summary_config_type * );
int                    summary_config_get_active_mask(const summary_config_type *);
int                    summary_config_get_var_index(const summary_config_type * , const char * );
const char          ** summary_config_get_var_list_ref(const summary_config_type *);
void                   summary_config_add_var(summary_config_type *  , const char * );
bool                   summary_config_has_var(const summary_config_type * , const char * );
void                   summary_config_summarize(const summary_config_type * );


GET_ACTIVE_LIST_HEADER(summary);
GET_DATA_SIZE_HEADER(summary);
VOID_CONFIG_FREE_HEADER(summary);
#endif
