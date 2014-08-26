import os
import stat

class WorkflowCommon(object):

    @staticmethod
    def createExternalDumpJob():
        with open("dump_job", "w") as f:
            f.write("INTERNAL FALSE\n")
            f.write("EXECUTABLE dump.py\n")
            f.write("MIN_ARG 2\n")
            f.write("MAX_ARG 2\n")
            f.write("ARG_TYPE 0 STRING\n")
            f.write("ARG_TYPE 1 STRING\n")


        with open("dump.py", "w") as f:
            f.write("#!/usr/bin/env python\n")
            f.write("import sys\n")
            f.write("f = open('%s' % sys.argv[1], 'w')\n")
            f.write("f.write('%s' % sys.argv[2])\n")
            f.write("f.close()\n")

        st = os.stat("dump.py")
        os.chmod("dump.py", st.st_mode | stat.S_IEXEC) # | stat.S_IXUSR | stat.S_IXGRP | stat.S_IXOTH

        with open("dump_workflow", "w") as f:
            f.write("DUMP dump1 dump_text_1\n")
            f.write("DUMP dump2 dump_<PARAM>_2\n")


    @staticmethod
    def createInternalFunctionJob():
        with open("select_case_job", "w") as f:
            f.write("INTERNAL True\n")
            f.write("FUNCTION enkf_main_select_case_JOB\n")
            f.write("MIN_ARG 1\n")
            f.write("MAX_ARG 1\n")
            f.write("ARG_TYPE 0 STRING\n")

        with open("printf_job", "w") as f:
            f.write("INTERNAL True\n")
            f.write("FUNCTION printf\n")
            f.write("MIN_ARG 4\n")
            f.write("MAX_ARG 5\n")
            f.write("ARG_TYPE 0 STRING\n")
            f.write("ARG_TYPE 1 INT\n")
            f.write("ARG_TYPE 2 FLOAT\n")
            f.write("ARG_TYPE 3 BOOL\n")
            f.write("ARG_TYPE 4 STRING\n")



    @staticmethod
    def createErtScriptsJob():
        with open("subtract_script.py", "w") as f:
            f.write("from ert.job_queue import ErtScript\n")
            f.write("\n")
            f.write("class SubtractScript(ErtScript):\n")
            f.write("    def run(self, arg1, arg2):\n")
            f.write("        return arg1 - arg2\n")


        with open("subtract_script_job", "w") as f:
            f.write("INTERNAL True\n")
            f.write("SCRIPT subtract_script.py\n")
            f.write("MIN_ARG 2\n")
            f.write("MAX_ARG 2\n")
            f.write("ARG_TYPE 0 FLOAT\n")
            f.write("ARG_TYPE 1 FLOAT\n")
