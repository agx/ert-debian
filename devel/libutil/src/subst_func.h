#ifndef __SUBST_FUNC_H__
#define __SUBST_FUNC_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stringlist.h>


typedef  char * (subst_func_ftype) (const stringlist_type * );
typedef  struct subst_func_struct        subst_func_type;
typedef  struct subst_func_pool_struct   subst_func_pool_type;


char  *  subst_func_eval( const subst_func_type * subst_func , const stringlist_type * args );

/*****************************************************************/

subst_func_pool_type * subst_func_pool_alloc();
void                   subst_func_pool_free( subst_func_pool_type * pool );
void                   subst_func_pool_add_func( subst_func_pool_type * pool , const char * func_name , const char * doc_string , subst_func_ftype * func , bool vararg, int argc_min , int argc_max);
subst_func_type      * subst_func_pool_get_func( const subst_func_pool_type * pool , const char * func_name );
bool                   subst_func_pool_has_func( const subst_func_pool_type * pool , const char * func_name );
UTIL_IS_INSTANCE_HEADER( subst_func_pool );

/*****************************************************************/
char * subst_func_randint( const stringlist_type * args );
char * subst_func_randfloat( const stringlist_type * args );
char * subst_func_add( const stringlist_type * args );
char * subst_func_mul( const stringlist_type * args );
char * subst_func_exp( const stringlist_type * args );
char * subst_func_log( const stringlist_type * args );
char * subst_func_pow10( const stringlist_type * args );


#ifdef __cplusplus
}
#endif
#endif
