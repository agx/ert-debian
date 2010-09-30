#ifndef __OBS_DATA_H__
#define __OBS_DATA_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdbool.h>
#include <matrix.h>
#include <meas_data.h>
#include <hash.h>
#include <enkf_types.h>

typedef struct obs_data_struct   obs_data_type;
typedef struct obs_block_struct  obs_block_type;

active_type  obs_block_iget_active_mode( const obs_block_type * obs_block , int iobs);
const char * obs_data_iget_keyword( const obs_data_type * obs_data , int index );
double       obs_data_iget_value( const obs_data_type * obs_data , int index );
double       obs_data_iget_std( const obs_data_type * obs_data , int index );
active_type  obs_data_iget_active_mode( const obs_data_type * obs_data , int index );
void         obs_block_deactivate( obs_block_type * obs_block , int iobs , const char * msg);
int          obs_block_get_size( const obs_block_type * obs_block );
void         obs_block_iset( obs_block_type * obs_block , int iobs , double value , double std);

double obs_block_iget_std( const obs_block_type * obs_block , int iobs);
double obs_block_iget_value( const obs_block_type * obs_block , int iobs);
bool   obs_block_iget_active( const obs_block_type * obs_block , int iobs);


obs_block_type       * obs_data_iget_block( obs_data_type * obs_data , int index );
const obs_block_type *     obs_data_iget_block_const( const obs_data_type * obs_data , int block_nr);
obs_block_type *     obs_data_get_block( obs_data_type * obs_data , const char * obs_key );
obs_block_type *     obs_data_add_block( obs_data_type * obs_data , const char * obs_key , int obs_size , matrix_type * error_covar , bool error_covar_owner);

obs_data_type      * obs_data_alloc();
void                 obs_data_free(obs_data_type *);
void                 obs_data_reset(obs_data_type * obs_data);
matrix_type        * obs_data_allocD(const obs_data_type * obs_data , const matrix_type * E  , const matrix_type * S);
matrix_type        * obs_data_allocR(const obs_data_type * obs_data , int active_size );
double             * obs_data_alloc_innov(const obs_data_type * obs_data , const meas_data_type * meas_data , int active_size);
matrix_type        * obs_data_allocE(const obs_data_type * obs_data , int ens_size, int active_size);
void                 obs_data_scale(const obs_data_type * obs_data , matrix_type *S , matrix_type *E , matrix_type *D , matrix_type *R , double *innov);
void                 obs_data_fprintf(const obs_data_type * , const meas_data_type * meas_data , FILE *);
void                 obs_data_iget_value_std(const obs_data_type * obs_data , int index , double * value ,  double * std);
int                  obs_data_get_active_size(  obs_data_type * obs_data );
int                  obs_data_get_num_blocks( const obs_data_type * obs_data );
const char * obs_block_get_key( const obs_block_type * obs_block) ;

#ifdef __cplusplus
}
#endif
#endif
