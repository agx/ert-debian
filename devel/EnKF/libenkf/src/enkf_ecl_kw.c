#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <util.h>
#include <enkf_state.h>
#include <ecl_kw.h>
#include <enkf_ecl_kw.h>
#include <enkf_ecl_kw_config.h>


struct enkf_ecl_kw_struct {
  /*
    Due to the way these objects are allocated it is simplest to
    let each object have it's own config object.
  */
  ecl_type_enum                  ecl_type;
  const enkf_ecl_kw_config_type  *config;
  const enkf_state_type          *enkf_state;
  double                         *data;
};


static enkf_ecl_kw_type * enkf_ecl_kw_alloc2(const enkf_state_type * enkf_state , const enkf_ecl_kw_config_type *config) {
  enkf_ecl_kw_type     * enkf_kw = malloc(sizeof *enkf_kw);
  enkf_kw->config     = config;
  enkf_kw->enkf_state = enkf_state;
  enkf_kw->data       = enkf_util_malloc(enkf_ecl_kw_config_get_size(enkf_kw->config) * sizeof * enkf_kw->data , __func__);
  return enkf_kw;
}


enkf_ecl_kw_type * enkf_ecl_kw_alloc(const enkf_state_type * enkf_state , const char * ens_file , int size , const char * ecl_kw_name) { 
  
  enkf_ecl_kw_config_type * config  = enkf_ecl_kw_config_alloc(size , ecl_kw_name , ens_file);
  return enkf_ecl_kw_alloc2(enkf_state , config);

}


enkf_ecl_kw_type * enkf_kw_copyc(const enkf_ecl_kw_type * src) {
  enkf_ecl_kw_config_type * config = enkf_ecl_kw_config_copyc(src->config);
  enkf_ecl_kw_type        * new_kw = enkf_ecl_kw_alloc2(src->enkf_state , config);
  
  memcpy(new->data , src->data , enkf_eckl_kw_config_get_size(config) * sizeof * src->data);
  return new;
}



ecl_kw_type * enkf_ecl_kw_alloc_ecl_kw(const enkf_ecl_kw_type *enkf_kw , bool fmt_file , bool endian_convert) {
  const int size     = enkf_ecl_kw_config_get_size(enkf_kw->config);
  const char *header = enkf_kw->config->ecl_kw_name;
  ecl_kw_type * ecl_kw;
  void *data;

  if (enkf_kw->ecl_type == ecl_float_type) {
    data = enkf_util_malloc(size * sizeof(float) , __func__ );
    util_double_to_float(data , enkf_kw->data , size);
  } else
    data = enkf_kw->data;
  
  ecl_kw = ecl_kw_alloc_complete(fmt_file , endian_convert , header , size , enkf_kw->ecl_type , data);
  if (enkf_kw->ecl_type == ecl_float_type) 
    free(data);

  return ecl_kw;
}


void enkf_ecl_kw_get_data(enkf_ecl_kw_type * enkf_kw , const ecl_kw_type *ecl_kw) {
  enkf_kw->ecl_type =  ecl_kw_get_type(ecl_kw);
  if (enkf_kw->ecl_type == ecl_double_type) 
  ecl_kw_get_memcpy_data(ecl_kw , enkf_kw->data);
  else if (enkf_kw->ecl_type == ecl_float_type) 
  util_float_to_double(enkf_kw->data , ecl_kw_get_data_ref(ecl_kw) , ecl_kw_get_size(ecl_kw));
  else {
    fprintf(stderr,"%s fatal error ECLIPSE type:%s can not be used in EnKF - aborting \n",__func__  , ecl_kw_get_str_type_ref(ecl_kw));
    abort();
  }
}


void enkf_ecl_kw_clear(enkf_ecl_kw_type * enkf_kw) {
  int i;
  for (i = 0; i < enkf_ecl_kw_config_get_size(enkf_kw->config); i++)
    enkf_kw->data[i] = 0;
}


enkf_ecl_kw_type * enkf_ecl_kw_copyc(const enkf_ecl_kw_type *enkf_kw) {
  enkf_ecl_kw_type * new_kw = enkf_ecl_kw_alloc2(enkf_kw->enkf_state , enkf_kw->config);
  memcpy(new_kw->data , enkf_kw->data , enkf_ecl_kw_config_get_size(enkf_kw->config) * sizeof * enkf_kw->data);
  new_kw->ecl_type = enkf_kw->ecl_type;
  return new_kw;
}

char * enkf_ecl_kw_alloc_ensname(const enkf_ecl_kw_type *enkf_ecl_kw) {
  char *ens_file  = "NULL";
  return ens_file;
}

void enkf_ecl_kw_ens_write(const enkf_ecl_kw_type * enkf_ecl_kw) {
  char * ens_file = "NULL";
  FILE * stream   = enkf_util_fopen_w(ens_file , __func__);
  const int    size = enkf_ecl_kw_config_get_size(enkf_ecl_kw->config);
  fwrite(&size    , sizeof  size     , 1 , stream);
  enkf_util_fwrite(enkf_ecl_kw->data    , sizeof *enkf_ecl_kw->data    , size , stream , __func__);
  fclose(stream);
  free(ens_file);
}

void enkf_ecl_kw_ens_read(enkf_ecl_kw_type * enkf_ecl_kw) {
  char * ens_file = "NULL";
  FILE * stream   = enkf_util_fopen_r(ens_file , __func__);
  int  size;
  fread(&size , sizeof  size     , 1 , stream);
  enkf_util_fread(enkf_ecl_kw->data , sizeof *enkf_ecl_kw->data , size , stream , __func__);
  fclose(stream);
  free(ens_file);
}

void enkf_ecl_kw_free(enkf_ecl_kw_type *enkf_ecl_kw) {
  enkf_ecl_kw_config_free((enkf_ecl_kw_config_type *) enkf_ecl_kw->config);
  free(enkf_ecl_kw->data);
  free(enkf_ecl_kw);
}

void enkf_ecl_kw_serialize(const enkf_ecl_kw_type *enkf_ecl_kw , double *serial_data , size_t *_offset) {
  const int    size = enkf_ecl_kw_config_get_size(enkf_ecl_kw->config);
  int offset = *_offset;

  memcpy(&serial_data[offset] , enkf_ecl_kw->data , size * sizeof * enkf_ecl_kw->data);
  offset += size;

  *_offset = offset;
}



/*
  MATH_OPS(enkf_ecl_kw);
*/


/******************************************************************/
/* Anonumously generated functions used by the enkf_node object   */
/******************************************************************/
VOID_FUNC      (enkf_ecl_kw_free      , enkf_ecl_kw_type)
VOID_FUNC_CONST(enkf_ecl_kw_ens_write , enkf_ecl_kw_type)
VOID_FUNC_CONST(enkf_ecl_kw_copyc     , enkf_ecl_kw_type)
VOID_FUNC      (enkf_ecl_kw_ens_read  , enkf_ecl_kw_type)
VOID_FUNC      (enkf_ecl_kw_clear     , enkf_ecl_kw_type)
VOID_SERIALIZE (enkf_ecl_kw_serialize , enkf_ecl_kw_type)


