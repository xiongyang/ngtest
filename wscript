top = '.'
out = '_build'


packageList = 'interface strategy util'

BluesCXXFlag= ['-std=c++11' , '-O2']

import sys
def options(opt):
        opt.load('compiler_cxx')

        #conf.check(header_name='windows.h', features='c cprogram', define_name='BLUE_WINDOWS', mandatory=False)

        opt.recurse(packageList)
    #opt.load("compiler_cxx")
    

def configure(conf):
        print (sys.platform)
        if sys.platform == 'win32' or sys.platform == 'cygwin':
            conf.define('_WIN32_PLATFROM_', 1)
        else:
            conf.define('_LINUX_PLATFROM_', 1)

        conf.load('compiler_cxx')
        conf.env.append_unique('CXXFLAGS', BluesCXXFlag)
        conf.env.append_unique('CXXFLAGS', BluesCXXFlag)
        conf.recurse(packageList)

def build(bld):
         bld.recurse(packageList)