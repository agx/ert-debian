#  Copyright (C) 2011  Statoil ASA, Norway. 
#   
#  The file 'job_queue.py' is part of ERT - Ensemble based Reservoir Tool. 
#   
#  ERT is free software: you can redistribute it and/or modify 
#  it under the terms of the GNU General Public License as published by 
#  the Free Software Foundation, either version 3 of the License, or 
#  (at your option) any later version. 
#   
#  ERT is distributed in the hope that it will be useful, but WITHOUT ANY 
#  WARRANTY; without even the implied warranty of MERCHANTABILITY or 
#  FITNESS FOR A PARTICULAR PURPOSE.   
#   
#  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
#  for more details. 

import  time
import  threading
import  ctypes

from    ert.cwrap.cwrap       import *

# Need to import this to ensure that the ctime type is registered
import  ert.util.ctime        


import  libjob_queue

from    job  import Job


class QueueThread( threading.Thread ):

    def __init__(self , queue):
        threading.Thread.__init__( self )
        self.queue = queue

    def run(self):
        verbose = False
        cfunc.start_queue( self.queue , self.queue.size , verbose )   
        


class JobList:
    def __init__(self):
        self.job_list = []
        self.job_dict = {}
        

    def __getitem__(self , index):
        job = None
        if isinstance(index , types.StringType):
            job = self.job_dict.get( index )
        elif isinstance(index , types.IntType):
            try:
                job = self.job_list[index]
            except:
                job = None
        return job


    def add_job( self , job , job_name ):
        job_index  = len( self.job_list )
        job.job_nr = job_index
        self.job_dict[ job_name ] = job
        self.job_list.append( job )
        

    @property
    def size(self):
        return len( self.job_list )



class exList:
    def __init__(self , joblist):
        self.joblist = joblist

    def __getitem__(self , index):
        job = self.joblist.__getitem__(index)
        if job:
            return True
        else:
            return False

        

class statusList:
    def __init__(self , joblist ):
        self.joblist = joblist

    def __getitem__(self , index):
        job = self.joblist.__getitem__(index)
        if job:
            return job.status()
        else:
            return None


class runtimeList:
    def __init__(self , joblist , queue):
        self.joblist = joblist
        self.queue   = queue

    def __getitem__(self , index):
        job = self.joblist.__getitem__(index)
        if job:
            sim_start = cfunc.iget_sim_start( self.queue , job.job_nr )
            if not sim_start.ctime() == -1:
                return time.time( ) - sim_start.ctime()
            else:
                return None
        else:
            return None


class JobQueue:
    
    # If the queue is created with size == 0 that means that it will
    # just grow as needed; for the queue layer to know when to exit
    # you must call the function submit_complete() when you have no
    # more jobs to submit.
    #
    # If the number of jobs is known in advance you can create the
    # queue with a finite value for size, in that case it is not
    # necessary to explitly inform the queue layer when all jobs have
    # been submitted.

    def __init__(self , driver , cmd , max_submit = 1 , size = 0):
        OK_file     = None 
        exit_file   = None
        self.c_ptr  = cfunc.alloc_queue(max_submit , False , OK_file , exit_file , cmd )
        self.driver = driver
        self.jobs   = JobList()
        self.size   = 0


        self.exists   = exList( self.jobs )
        self.status   = statusList( self.jobs )
        self.run_time = runtimeList( self.jobs , self )
        cfunc.set_driver( self , driver.c_ptr )
        self.start( blocking = False )

    
    def kill_job(self , index):
        job = self.jobs.__getitem__( index )
        if job:
            job.kill()

    def start( self , blocking = False):
        self.queue_thread = QueueThread( self )
        self.queue_thread.start()
    

    def __del__(self):
        cfunc.free_queue( self )


    def from_param( self ):
        return ctypes.c_void_p( self.c_ptr )

    def add_job( self , run_path , job_name , argv):
        c_argv = (ctypes.c_char_p * len(argv))()
        c_argv[:] = argv
        job_index = self.jobs.size
        queue_index = cfunc.cadd_job_mt( self , run_path , job_name , len(argv) , c_argv)
        job = Job( self.driver , cfunc.get_job_ptr( self , queue_index ) , queue_index , False )
        
        self.jobs.add_job( job , job_name )
        return job

    def clear( self ):
        pass

    
    # This method is used to signal the queue layer that all jobs have
    # been submitted, and that the queue can exit when all jobs have
    # completed. If the queue is created with a finite size it is not
    # necessary to use this function.
    def submit_complete( self ):
        cfunc.submit_complete( self )


    @property
    def running(self):
        return cfunc.is_running( self )

    @property
    def num_running( self ):
        return cfunc.num_running( self )

    @property
    def num_pending( self ):
        return cfunc.num_pending( self )

    @property
    def num_waiting( self ):
        return cfunc.num_waiting( self )

    @property
    def num_complete( self ):
        return cfunc.num_complete( self )

    def exists(self , index):
        job = self.__getitem__(index)
        if job:
            return True
        else:
            return False



#class EclQueue( JobQueue ):
#    default_eclipse_cmd  = ecl_default.cmd
#    default_version      = ecl_default.version
#
#    
#    def __init__(self , driver , ecl_version = default_version , eclipse_cmd = default_eclipse_cmd, size = 0):
#        JobQueue.__init__( self , driver , eclipse_cmd , size = size)
#        self.ecl_version = ecl_version
#        
#        
#    def add_job( self , data_file):
#        (path_base , ext) = os.path.splitext( data_file )
#        (run_path , base) = os.path.split( path_base )
#        
#        argv = [ self.ecl_version , path_base , "%s" % ecl_util.get_num_cpu( data_file )]
#        return JobQueue.add_job( self , run_path , base , argv)


#################################################################

cwrapper = CWrapper( libjob_queue.lib )
cwrapper.registerType( "job_queue" , JobQueue )
cfunc  = CWrapperNameSpace( "JobQeueu" )

cfunc.alloc_queue     = cwrapper.prototype("c_void_p job_queue_alloc( int , bool , char* , char* , char* )")
cfunc.free_queue      = cwrapper.prototype("void job_queue_free( job_queue )")
cfunc.set_max_running = cwrapper.prototype("void job_queue_set_max_running( job_queue , int)")
cfunc.get_max_running = cwrapper.prototype("int  job_queue_get_max_running( job_queue )")
cfunc.set_driver      = cwrapper.prototype("void job_queue_set_driver( job_queue , c_void_p )")
cfunc.cadd_job_mt     = cwrapper.prototype("int  job_queue_add_job_mt( job_queue , char* , char* , int , char**)")
cfunc.cadd_job_st     = cwrapper.prototype("int  job_queue_add_job_st( job_queue , char* , char* , int , char**)")
cfunc.start_queue     = cwrapper.prototype("void job_queue_run_jobs( job_queue , int , bool)")
cfunc.num_running     = cwrapper.prototype("int  job_queue_get_num_running( job_queue )")
cfunc.num_complete    = cwrapper.prototype("int  job_queue_get_num_complete( job_queue )")
cfunc.num_waiting     = cwrapper.prototype("int  job_queue_get_num_waiting( job_queue )")
cfunc.num_pending     = cwrapper.prototype("int  job_queue_get_num_pending( job_queue )")
cfunc.is_running      = cwrapper.prototype("int  job_queue_is_running( job_queue )")
cfunc.submit_complete = cwrapper.prototype("void job_queue_submit_complete( job_queue )")
cfunc.get_job_ptr     = cwrapper.prototype("c_void_p job_queue_iget_job_data( job_queue , int)")
cfunc.iget_sim_start  = cwrapper.prototype("time_t job_queue_iget_sim_start( job_queue , int)")
cfunc.get_active_size = cwrapper.prototype("int job_queue_get_active_size( job_queue )")