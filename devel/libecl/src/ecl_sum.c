#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <ecl_util.h>
#include <ecl_sum.h>
#include <hash.h>
#include <util.h>
#include <time.h>
#include <set.h>
#include <util.h>
#include <vector.h>
#include <int_vector.h>
#include <time_t_vector.h>
#include <ecl_smspec.h>
#include <ecl_sum_data.h>
#include <stringlist.h>

/**
   The ECLIPSE summary data is organised in a header file (.SMSPEC)
   and the actual summary data. This file implements a data structure
   ecl_sum_type which holds ECLIPSE summary data. Most of the actual
   implementation is in separate files ecl_smspec.c for the SMSPEC
   header, and ecl_sum_data for the actual data.
   
   Observe that this datastructure is built up around internalizing
   ECLIPSE summary data, the code has NO AMBITION of being able to
   write summary data.
*/




#define ECL_SUM_ID          89067

/*****************************************************************/

struct ecl_sum_struct {
  UTIL_TYPE_ID_DECLARATION;
  ecl_smspec_type   * smspec;   /* Internalized version of the SMSPEC file. */
  ecl_sum_data_type * data;     /* The data - can be NULL. */
};



/**
   Reads the data from ECLIPSE summary files, can either be a list of
   files BASE.S0000, BASE.S0001, BASE.S0002,.. or one unified
   file. Formatted/unformatted is detected automagically.
   
   The actual loading is implemented in the ecl_sum_data.c file.
*/


static void ecl_sum_fread_realloc_data(ecl_sum_type * ecl_sum , const stringlist_type * data_files , bool include_restart) {
  if (ecl_sum->data != NULL)
    ecl_sum_free_data( ecl_sum );
  ecl_sum->data   = ecl_sum_data_fread_alloc( ecl_sum->smspec , data_files , include_restart);
}


static ecl_sum_type * ecl_sum_fread_alloc__(const char *header_file , const stringlist_type *data_files , const char * key_join_string, bool include_restart) {
  ecl_sum_type *ecl_sum = util_malloc( sizeof * ecl_sum , __func__);
  UTIL_TYPE_ID_INIT( ecl_sum , ECL_SUM_ID );
  ecl_sum->smspec = ecl_smspec_fread_alloc( header_file , key_join_string); 
  ecl_sum->data   = NULL;
  ecl_sum_fread_realloc_data(ecl_sum , data_files , include_restart);
  return ecl_sum;
}

/**
   This will explicitly load the summary specified by @header_file and
   @data_files, i.e. if the case has been restarted from another case,
   it will NOT look for old summary information - that functionality
   is only invoked when using ecl_sum_fread_alloc_case() function.
*/

ecl_sum_type * ecl_sum_fread_alloc(const char *header_file , const stringlist_type *data_files , const char * key_join_string) {
  return ecl_sum_fread_alloc__( header_file , data_files , key_join_string , false );
}



UTIL_SAFE_CAST_FUNCTION( ecl_sum , ECL_SUM_ID );
UTIL_IS_INSTANCE_FUNCTION( ecl_sum , ECL_SUM_ID );

/**
   This function frees the data from the ecl_sum instance and sets the
   data pointer to NULL. The SMSPEC data is still valid, and can be
   reused with calls to ecl_sum_fread_realloc_data().
*/
  
void ecl_sum_free_data( ecl_sum_type * ecl_sum ) {
  ecl_sum_data_free( ecl_sum->data );
  ecl_sum->data = NULL;
}


void ecl_sum_free( ecl_sum_type * ecl_sum ) {
  ecl_sum_free_data( ecl_sum );
  ecl_smspec_free( ecl_sum->smspec );
  free( ecl_sum );
}



void ecl_sum_free__(void * __ecl_sum) {
  ecl_sum_type * ecl_sum = ecl_sum_safe_cast( __ecl_sum);
  ecl_sum_free( ecl_sum );
}




/**
   This function takes an input file, and loads the corresponding
   summary. The function extracts the path part, and the basename from
   the input file. The extension is not considered (the input need not
   even be a valid file). In principle a simulation directory with a
   given basename can contain four different simulation cases:

    * Formatted and unformatted.
    * Unified and not unified.
    
   The program will load the most recent dataset, by looking at the
   modification time stamps of the files; if no simulation case is
   found the function will return NULL. 

   If the SMSPEC file contains the RESTART keyword the function will
   iterate backwards to load summary information from previous runs
   (this is goverened by the local variable include_restart).
*/


ecl_sum_type * ecl_sum_fread_alloc_case(const char * input_file , const char * key_join_string){
  const bool include_restart = true;
  ecl_sum_type * ecl_sum     = NULL;
  char * path , * base;
  char * header_file;
  stringlist_type * summary_file_list = stringlist_alloc_new();

  util_alloc_file_components( input_file , &path , &base , NULL);
  if (ecl_util_alloc_summary_files( path , base , &header_file , summary_file_list )) 
    ecl_sum = ecl_sum_fread_alloc__( header_file , summary_file_list , key_join_string , include_restart);
  
  free(base);
  util_safe_free(path);
  free(header_file);
  stringlist_free( summary_file_list );

  return ecl_sum;
}


/*****************************************************************/
/* 
   Here comes lots of access functions - these are mostly thin
   wrapppers around ecl_smspec functions. See more 'extensive'
   documentation in ecl_smspec.c
   
   The functions returning an actual value,
   i.e. ecl_sum_get_well_var() will trustingly call ecl_sum_data_get()
   with whatever indices it gets. If the indices are invalid -
   ecl_sum_data_get() will abort. The abort is the 'correct'
   behaviour, but it is possible to abort in this scope as well, in
   that case more informative error message can be supplied (i.e. the
   well/variable B-33T2/WOPR does not exist, instead of just "invalid
   index" which is the best ecl_sum_data_get() can manage.).
*/

/*****************************************************************/
/* Well variables */

int  	ecl_sum_get_well_var_index(const ecl_sum_type * ecl_sum , const char * well , const char *var) { return ecl_smspec_get_well_var_index(ecl_sum->smspec , well , var); }
bool 	ecl_sum_has_well_var(const ecl_sum_type * ecl_sum , const char * well , const char *var)       { return ecl_smspec_has_well_var(ecl_sum->smspec , well , var); }

double  ecl_sum_get_well_var(const ecl_sum_type * ecl_sum , int ministep , const char * well , const char *var) {
  int index = ecl_sum_get_well_var_index( ecl_sum , well , var );
  return ecl_sum_data_get( ecl_sum->data , ministep , index);
}

double ecl_sum_get_well_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * well , const char * var) {
  int index = ecl_sum_get_well_var_index( ecl_sum , well , var );
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_well_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * well , const char * var) {
  int index = ecl_sum_get_well_var_index( ecl_sum , well , var );
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}


/*****************************************************************/
/* Group variables */

int  ecl_sum_get_group_var_index(const ecl_sum_type * ecl_sum , const char * group , const char *var) { return ecl_smspec_get_group_var_index( ecl_sum->smspec , group , var); }
bool ecl_sum_has_group_var(const ecl_sum_type * ecl_sum , const char * group , const char *var)       { return ecl_smspec_has_group_var( ecl_sum->smspec , group , var); }

double  ecl_sum_get_group_var(const ecl_sum_type * ecl_sum , int ministep , const char * group , const char *var) {
  int index = ecl_sum_get_group_var_index( ecl_sum , group , var );
  return ecl_sum_data_get( ecl_sum->data , ministep , index);
}

double ecl_sum_get_group_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * group , const char * var) {
  int index = ecl_sum_get_group_var_index( ecl_sum , group , var );
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_group_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * group , const char * var) {
  int index = ecl_sum_get_group_var_index( ecl_sum , group , var );
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}


/*****************************************************************/
/* Field variables */
int  ecl_sum_get_field_var_index(const ecl_sum_type * ecl_sum , const char *var) { return ecl_smspec_get_field_var_index( ecl_sum->smspec , var); }
bool ecl_sum_has_field_var(const ecl_sum_type * ecl_sum , const char *var)       { return ecl_smspec_has_field_var( ecl_sum->smspec , var); }

double ecl_sum_get_field_var(const ecl_sum_type * ecl_sum , int ministep , const char * var) {
  int index = ecl_sum_get_field_var_index( ecl_sum ,  var );
  return ecl_sum_data_get( ecl_sum->data , ministep , index);
}

double ecl_sum_get_field_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * var) {
  int index = ecl_sum_get_field_var_index( ecl_sum , var );
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_field_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * var) {
  int index = ecl_sum_get_field_var_index( ecl_sum , var );
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}


/*****************************************************************/
/* Block variables */

int  ecl_sum_get_block_var_index(const ecl_sum_type * ecl_sum , const char * block_var , int block_nr) { return ecl_smspec_get_block_var_index( ecl_sum->smspec , block_var , block_nr ); }
bool ecl_sum_has_block_var(const ecl_sum_type * ecl_sum , const char * block_var , int block_nr)       { return ecl_smspec_has_block_var( ecl_sum->smspec , block_var , block_nr ); }
double ecl_sum_get_block_var(const ecl_sum_type * ecl_sum , int ministep , const char * block_var , int block_nr) {
  int index = ecl_sum_get_block_var_index( ecl_sum ,  block_var , block_nr);
  return ecl_sum_data_get( ecl_sum->data , ministep , index);
}


int  ecl_sum_get_block_var_index_ijk(const ecl_sum_type * ecl_sum , const char * block_var , int i, int j , int k ) { 
  return ecl_smspec_get_block_var_index_ijk( ecl_sum->smspec , block_var , i , j , k); 
}

bool ecl_sum_has_block_var_ijk(const ecl_sum_type * ecl_sum , const char * block_var , int i, int j , int k) { 
  return ecl_smspec_has_block_var_ijk( ecl_sum->smspec , block_var , i ,j , k); 
}

double ecl_sum_get_block_var_ijk(const ecl_sum_type * ecl_sum , int ministep , const char * block_var , int i , int j , int k) {
  int index = ecl_sum_get_block_var_index_ijk( ecl_sum ,  block_var , i , j , k);
  return ecl_sum_data_get( ecl_sum->data , ministep , index);
}

double ecl_sum_get_block_var_ijk_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * block_var, int i , int j , int k) {
  int index = ecl_sum_get_block_var_index_ijk( ecl_sum , block_var ,i,j,k);
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_block_var_ijk_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * block_var, int i , int j , int k) {
  int index = ecl_sum_get_block_var_index_ijk( ecl_sum , block_var ,i,j,k);
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}



/*****************************************************************/
/* Region variables */
/**
   region_nr: [1...num_regions] (NOT C-based indexing)
*/

int  ecl_sum_get_region_var_index(const ecl_sum_type * ecl_sum , int region_nr , const char *var) { return ecl_smspec_get_region_var_index( ecl_sum->smspec , region_nr , var); }
bool ecl_sum_has_region_var(const ecl_sum_type * ecl_sum , int region_nr , const char *var)       { return ecl_smspec_has_region_var( ecl_sum->smspec , region_nr , var); }

double ecl_sum_get_region_var(const ecl_sum_type * ecl_sum , int ministep , int region_nr , const char *var) {
  int index = ecl_sum_get_region_var_index( ecl_sum ,  region_nr , var);
  return ecl_sum_data_get( ecl_sum->data , ministep , index);
}

double ecl_sum_get_region_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , int region_nr , const char * var) {
  int index = ecl_sum_get_region_var_index( ecl_sum , region_nr , var );
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_region_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , int region_nr , const char * var) {
  int index = ecl_sum_get_region_var_index( ecl_sum , region_nr , var );
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}



/*****************************************************************/
/* Misc variables */

int  	ecl_sum_get_misc_var_index(const ecl_sum_type * ecl_sum , const char *var) { return ecl_smspec_get_misc_var_index( ecl_sum->smspec , var ); }
bool 	ecl_sum_has_misc_var(const ecl_sum_type * ecl_sum , const char *var)       { return ecl_smspec_has_misc_var( ecl_sum->smspec , var ); }

double  ecl_sum_get_misc_var(const ecl_sum_type * ecl_sum , int ministep , const char *var) {
  int index = ecl_sum_get_misc_var_index( ecl_sum ,  var);
  return ecl_sum_data_get( ecl_sum->data , ministep , index);
}

double ecl_sum_get_misc_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * var) {
  int index = ecl_sum_get_misc_var_index( ecl_sum , var );
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_misc_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * var) {
  int index = ecl_sum_get_misc_var_index( ecl_sum , var );
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}



/*****************************************************************/
/* Well completion - not fully implemented ?? */

int ecl_sum_get_well_completion_var_index(const ecl_sum_type * ecl_sum , const char * well , const char *var, int cell_nr) { 
  return ecl_smspec_get_well_completion_var_index( ecl_sum->smspec , well , var , cell_nr);
}

bool ecl_sum_has_well_completion_var(const ecl_sum_type * ecl_sum , const char * well , const char *var, int cell_nr)  {
  return ecl_smspec_has_well_completion_var( ecl_sum->smspec , well , var , cell_nr);
}

double ecl_sum_get_well_completion_var(const ecl_sum_type * ecl_sum , int ministep , const char * well , const char *var, int cell_nr)  {
  int index = ecl_sum_get_well_completion_var_index(ecl_sum , well , var , cell_nr);
  return ecl_sum_data_get( ecl_sum->data , ministep , index);
}

/*****************************************************************/
/* General variables - this means WWCT:OP_1 - i.e. composite variables*/

int  ecl_sum_get_general_var_index(const ecl_sum_type * ecl_sum , const char * lookup_kw) { 
  return ecl_smspec_get_general_var_index( ecl_sum->smspec , lookup_kw); 
}

bool ecl_sum_has_general_var(const ecl_sum_type * ecl_sum , const char * lookup_kw)       { return ecl_smspec_has_general_var( ecl_sum->smspec , lookup_kw); }

double ecl_sum_get_general_var(const ecl_sum_type * ecl_sum , int ministep , const char * lookup_kw) {
  int index = ecl_sum_get_general_var_index(ecl_sum , lookup_kw);
  return ecl_sum_data_get( ecl_sum->data , ministep , index);
}

double ecl_sum_get_general_var_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time , const char * var) {
  int index = ecl_sum_get_general_var_index( ecl_sum , var );
  return ecl_sum_data_get_from_sim_time( ecl_sum->data , sim_time , index );
}

double ecl_sum_get_general_var_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days , const char * var) {
  int index = ecl_sum_get_general_var_index( ecl_sum , var );
  return ecl_sum_data_get_from_sim_days( ecl_sum->data , sim_days , index );
}

const char * ecl_sum_get_general_var_unit( const ecl_sum_type * ecl_sum , const char * var) {
  return ecl_smspec_get_general_var_unit(ecl_sum->smspec , var );
}

/*****************************************************************/
/* Indexed get - these functions can be used after another function
   has been used to query for index.
*/
   

double ecl_sum_iget( const ecl_sum_type * ecl_sum , int ministep , int index) {
  return ecl_sum_data_get(ecl_sum->data , ministep , index);
}

const char * ecl_sum_iget_unit( const ecl_sum_type * ecl_sum , int index) {
  return ecl_smspec_iget_unit(ecl_sum->smspec , index);
}


/*****************************************************************/
/* 
   Here comes a couple of functions relating to the time dimension,
   about reports and ministeps and such things. The functions here in
   this file are just thin wrappers of 'real' functions located in
   ecl_sum_data.c.
*/
   


bool  ecl_sum_has_report_step(const ecl_sum_type * ecl_sum , int report_step ) {
  return ecl_sum_data_has_report_step( ecl_sum->data , report_step );
}


bool  ecl_sum_has_ministep(const ecl_sum_type * ecl_sum , int ministep ) {
  return ecl_sum_data_has_ministep( ecl_sum->data , ministep );
}



void ecl_sum_get_ministep_range(const ecl_sum_type * ecl_sum , int * ministep1, int * ministep2) {
  ecl_sum_data_get_ministep_range(ecl_sum->data , ministep1 , ministep2);
}


int ecl_sum_get_last_report_step( const ecl_sum_type * ecl_sum) {
  return ecl_sum_data_get_last_report_step( ecl_sum->data );
}

int ecl_sum_get_first_report_step( const ecl_sum_type * ecl_sum ) {
  return ecl_sum_data_get_first_report_step( ecl_sum->data );
}

int ecl_sum_get_last_ministep( const ecl_sum_type * ecl_sum) {
  return ecl_sum_data_get_last_ministep( ecl_sum->data );
}

int ecl_sum_get_first_ministep( const ecl_sum_type * ecl_sum ) {
  return ecl_sum_data_get_first_ministep( ecl_sum->data );
}
   

/*
  Translates a report step to the correspondingly first and last
  ministep. Will set the two ministeps to -1 if the report step is not
  valid.
*/

void ecl_sum_report2ministep_range(const ecl_sum_type * ecl_sum , int report_step , int * ministep1 , int * ministep2 ){
  ecl_sum_data_report2ministep_range( ecl_sum->data , report_step , ministep1 , ministep2);
}

/*
  return the first ministep incluced in report step. 
*/
int ecl_sum_get_report_ministep_start( const ecl_sum_type * ecl_sum, int report_step) {
  int ministep1 , ministep2;
  ecl_sum_report2ministep_range( ecl_sum , report_step , &ministep1 , &ministep2);
  return ministep1;
}

int ecl_sum_get_report_ministep_end( const ecl_sum_type * ecl_sum, int report_step) {
  int ministep1 , ministep2;
  ecl_sum_report2ministep_range( ecl_sum , report_step , &ministep1 , &ministep2);
  return ministep2;
}


void ecl_sum_init_time_vector( const ecl_sum_type * ecl_sum , time_t_vector_type * time_vector , bool report_only ) {
  ecl_sum_data_init_time_vector( ecl_sum->data , time_vector , report_only );
}


time_t_vector_type * ecl_sum_alloc_time_vector( const ecl_sum_type * ecl_sum  , bool report_only) {
  return ecl_sum_data_alloc_time_vector( ecl_sum->data , report_only );
}

void ecl_sum_init_data_vector( const ecl_sum_type * ecl_sum , double_vector_type * data_vector , int data_index , bool report_only ) {
  ecl_sum_data_init_data_vector( ecl_sum->data , data_vector , data_index , report_only );
}


double_vector_type * ecl_sum_alloc_data_vector( const ecl_sum_type * ecl_sum  , int data_index , bool report_only) {
  return ecl_sum_data_alloc_data_vector( ecl_sum->data , data_index , report_only );
}



void ecl_sum_summarize( const ecl_sum_type * ecl_sum , FILE * stream ) {
  ecl_sum_data_summarize( ecl_sum->data , stream );
}



/**
   Returns the number of the first ministep where a limiting value is
   reached. If the limiting value is never reached, -1 is
   returned. The smspec_index should be calculated first with one of
   the
   
      ecl_sum_get_XXXX_index() 

   functions. I.e. the following code will give the first ministep
   where the water wut in well PX exceeds 0.25:

   {  
      int smspec_index   = ecl_sum_get_well_var( ecl_sum , "PX" , "WWCT" );
      int first_ministep = ecl_sum_get_first_ministep_gt( ecl_sum , smspec_index , 0.25);
   }
*/

int ecl_sum_get_first_ministep_gt(const ecl_sum_type * ecl_sum , int smspec_index , double limit) { 
  const int last_ministep = ecl_sum_get_last_ministep( ecl_sum );
  int ministep            = ecl_sum_get_first_ministep( ecl_sum ); 
  do {
    double value = ecl_sum_data_get( ecl_sum->data , ministep , smspec_index);
    if (value > limit)
      break;
    ministep++;
  } while (ministep < last_ministep);

  if (ministep == last_ministep)  /* Did not find it */
    ministep = -1;

  return ministep;
}


time_t ecl_sum_get_report_time( const ecl_sum_type * ecl_sum , int report_step ) {
  int ministep1 , ministep2;
  ecl_sum_report2ministep_range( ecl_sum , report_step , &ministep1 , &ministep2 );
  return ecl_sum_get_sim_time( ecl_sum , ministep2 );
}

time_t ecl_sum_get_sim_time( const ecl_sum_type * ecl_sum , int ministep ) {
  return ecl_sum_data_get_sim_time( ecl_sum->data , ministep);
}

time_t ecl_sum_get_start_time( const ecl_sum_type * ecl_sum) {
  return ecl_smspec_get_start_time( ecl_sum->smspec );
}

time_t ecl_sum_get_end_time( const ecl_sum_type * ecl_sum) {
  return ecl_sum_data_get_sim_end( ecl_sum->data );
}

double ecl_sum_get_sim_days( const ecl_sum_type * ecl_sum , int ministep ) {
  return ecl_sum_data_get_sim_days( ecl_sum->data , ministep);
}


int ecl_sum_get_ministep_from_sim_days( const ecl_sum_type * ecl_sum , double sim_days) {
  return ecl_sum_data_get_ministep_from_sim_days( ecl_sum->data , sim_days );
}


int ecl_sum_get_ministep_from_sim_time( const ecl_sum_type * ecl_sum , time_t sim_time) {
  return ecl_sum_data_get_ministep_from_sim_time(ecl_sum->data , sim_time );
}
 


/*****************************************************************/
/* This is essentially the summary.x program. */ 

void ecl_sum_fprintf(const ecl_sum_type * ecl_sum , FILE * stream , int nvars , const char ** var_list, bool report_only) {
  int first_report = ecl_sum_get_first_report_step( ecl_sum );
  int last_report  = ecl_sum_get_last_report_step( ecl_sum );
  bool *has_var    = util_malloc( nvars * sizeof * has_var   , __func__);
  int  *var_index  = util_malloc( nvars * sizeof * var_index , __func__);
  int report,ivar;
  
  for (ivar = 0; ivar < nvars; ivar++) {
    if (ecl_sum_has_general_var( ecl_sum , var_list[ivar] )) {
      has_var[ivar]   = true;
      var_index[ivar] = ecl_sum_get_general_var_index( ecl_sum , var_list[ivar] );
    } else {
      fprintf(stderr,"** Warning: could not find variable: \'%s\' in summary file \n", var_list[ivar]);
      has_var[ivar] = false;
    }
  }
    
  for (report = first_report; report <= last_report; report++) {
    if (ecl_sum_data_has_report_step(ecl_sum->data , report)) {
      int ministep1 , ministep2 , ministep;

      ecl_sum_data_report2ministep_range( ecl_sum->data , report , &ministep1 , &ministep2);
      if (report_only)
	ministep1 = ministep2;
      
      for (ministep = ministep1; ministep <= ministep2; ministep++) {
	if (ecl_sum_has_ministep(ecl_sum , ministep)) {
	  int day,month,year;
	  util_set_date_values(ecl_sum_get_sim_time(ecl_sum , ministep) , &day , &month, &year);
	  fprintf(stream , "%7.2f   %02d/%02d/%04d   " , ecl_sum_get_sim_days(ecl_sum , ministep) , day , month , year);
	  
	  for (ivar = 0; ivar < nvars; ivar++) 
	    if (has_var[ivar])
	      fprintf(stream , " %12.3f " , ecl_sum_iget(ecl_sum , ministep , var_index[ivar]));
	  
	  fprintf(stream , "\n");
	}
      }
      
    }
  }
  free( var_index );
  free( has_var );
}

const char * ecl_sum_get_case(const ecl_sum_type * ecl_sum) {
  return ecl_smspec_get_simulation_case( ecl_sum->smspec );
}


/**
   This function will check if the currently loaded case corresponds
   to the case specified by @input_file. The extension of @input file
   can be arbitrary (or nonexistent) and will be ignored (this can
   lead to errors with formatted/unformatted mixup if the simulation
   directory has been changed after the ecl_sum instance has been
   loaded).
*/


bool ecl_sum_same_case( const ecl_sum_type * ecl_sum , const char * input_file ) {
  bool   same_case = false;
  {
    char * path;
    char * base;

    util_alloc_file_components( input_file , &path , &base , NULL);
    {
      bool   fmt_file = ecl_smspec_get_formatted( ecl_sum->smspec );
      char * header_file = ecl_util_alloc_exfilename( path , base , ECL_SUMMARY_HEADER_FILE , fmt_file , -1 );
      if (header_file != NULL) {
        same_case = util_same_file( header_file , ecl_smspec_get_header_file( ecl_sum->smspec ));
        free( header_file );
      }
    }
    
    util_safe_free( path );
    util_safe_free( base );
  }
  return same_case;
}


/*****************************************************************/

bool ecl_sum_general_is_total(const ecl_sum_type * ecl_sum , const char * gen_key) {
  return ecl_smspec_general_is_total( ecl_sum->smspec , gen_key );
}

bool ecl_sum_var_is_total(const ecl_sum_type * ecl_sum , const char * gen_key) {
  return ecl_smspec_general_is_total( ecl_sum->smspec , gen_key );
}


/*****************************************************************/


stringlist_type * ecl_sum_alloc_matching_general_var_list(const ecl_sum_type * ecl_sum , const char * pattern) {
  return ecl_smspec_alloc_matching_general_var_list(ecl_sum->smspec , pattern );
}

void ecl_sum_select_matching_general_var_list( const ecl_sum_type * ecl_sum , const char * pattern , stringlist_type * keys) {
  ecl_smspec_select_matching_general_var_list( ecl_sum->smspec , pattern , keys );
}


stringlist_type * ecl_sum_alloc_well_list( const ecl_sum_type * ecl_sum ) {
  return ecl_smspec_alloc_well_list( ecl_sum->smspec );
}


stringlist_type * ecl_sum_alloc_well_var_list( const ecl_sum_type * ecl_sum ) {
  return ecl_smspec_alloc_well_var_list( ecl_sum->smspec );
}


/*****************************************************************/

ecl_smspec_var_type ecl_sum_identify_var_type(const ecl_sum_type * ecl_sum , const char * var) {
  return ecl_smspec_identify_var_type( ecl_sum->smspec , var );
}

/*****************************************************************/


void ecl_sum_resample_from_sim_time( const ecl_sum_type * ecl_sum , const time_t_vector_type * sim_time , double_vector_type * value , const char * gen_key) {
  int param_index = ecl_smspec_get_general_var_index( ecl_sum->smspec , gen_key );
  double_vector_reset( value );
  {
    int i;
    for (i=0; i < time_t_vector_size( sim_time ); i++)
      double_vector_iset( value , i , ecl_sum_data_get_from_sim_time( ecl_sum->data , time_t_vector_iget( sim_time , i ) , param_index));
  }
}


void ecl_sum_resample_from_sim_days( const ecl_sum_type * ecl_sum , const double_vector_type * sim_days , double_vector_type * value , const char * gen_key) {
  int param_index = ecl_smspec_get_general_var_index( ecl_sum->smspec , gen_key );
  double_vector_reset( value );
  {
    int i;
    for (i=0; i < double_vector_size( sim_days ); i++)
      double_vector_iset( value , i , ecl_sum_data_get_from_sim_days( ecl_sum->data , double_vector_iget( sim_days , i ) , param_index));
  }
}


time_t ecl_sum_time_from_days( const ecl_sum_type * ecl_sum , double sim_days ) {
  time_t t = ecl_smspec_get_start_time( ecl_sum->smspec );
  util_inplace_forward_days( &t , sim_days );
  return t;
}


double ecl_sum_days_from_time( const ecl_sum_type * ecl_sum , time_t sim_time ) {
  double seconds_diff = util_difftime( ecl_smspec_get_start_time( ecl_sum->smspec ) , sim_time , NULL , NULL , NULL, NULL);
  return seconds_diff * 1.0 / (3600 * 24.0);
}


double ecl_sum_get_sim_length( const ecl_sum_type * ecl_sum ) { 
  return ecl_sum_data_get_sim_length( ecl_sum->data );
}
