#include <stdlib.h>
#include <util.h>
#include <enkf_util.h>
#include <enkf_ecl_kw_config.h>


enkf_ecl_kw_config_type * enkf_ecl_kw_config_alloc(int size , const char *ecl_kw_name , const char * ens_file) {
  enkf_ecl_kw_config_type *config = malloc(sizeof *config);
  config->size          = size;
  config->ecl_kw_name   = util_alloc_string_copy(ecl_kw_name);
  config->ens_file      = util_alloc_string_copy(ens_file);
  config->var_type      = ecl_restart;
  return config;
}


enkf_ecl_kw_config_type * enkf_ecl_kw_config_alloc(const enkf_ecl_kw_config_type * src) {
  enkf_ecl_kw_config_type * new = enkc_ecl_kw_config_alloc(src->size , src->ecl_kw_name , src->ens_file);
  return new;
}


int enkf_ecl_kw_config_get_size(const enkf_ecl_kw_config_type * config) { return config->size; }

const char * enkf_ecl_kw_config_get_ensname_ref(const enkf_ecl_kw_config_type * enkf_ecl_kw_config) {
  return enkf_ecl_kw_config->ens_file;
}


void enkf_ecl_kw_config_free(enkf_ecl_kw_config_type *config) {
  free((char *) config->ecl_kw_name);
  free(config->ens_file);
  free(config);
}


