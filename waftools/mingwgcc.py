#! /usr/bin/env python
# encoding: utf-8


print('→ loading the mingw tool')


# snap_gcc_root = '/usr/local/snap_gcc49'
# snap_gcc_bin = snap_gcc_root + '/bin'
# snap_gcc_lib64 = snap_gcc_root + '/lib64'


# cxx_flags = ['-ggdb', '-std=c++0x', '-fPIC', '-fno-strict-aliasing', '-rdynamic', '-std=c++11', '-DDEBUG', '-DBOOST_LOG_DYN_LINK']
# link_flags = ['-g', '-lpthread', '-ldl', '-lssl', '-lcrypto']
# custom_tool_dir = './build_tool'
# c_flags = ['-O2', '-ggdb', '-fPIC', '-fno-strict-aliasing', '-rdynamic', '-Wall',  '-DDEBUG', '-DSFL_LINUX', '-DBOOST_LOG_DYN_LINK']


# def SetupDefaultBoostPath(opt):
#   BOOST_LIBS = []
#   BOOST_LIBS.append(opt.path.find_dir('extern/boost/stage/lib').abspath())
#   BOOST_INCLUDES = []
#   BOOST_INCLUDES.append(opt.path.find_dir('extern/boost/').abspath())

# def options(opt):
#   opt.load('compiler_cxx boost compiler_c')
# #  opt.load('glog', tooldir=custom_tool_dir)
#   opt.load('quickfix', tooldir=custom_tool_dir)
#   opt.load('leveldb', tooldir=custom_tool_dir)

  
from waflib.Tools import ccroot, gxx, gcc

def LoadSnapGcc(conf):
    cc = conf.find_program('gcc', path_list=[snap_gcc_bin, ], var='CC')
    cxx = conf.find_program('g++', path_list=[snap_gcc_bin, ], var='CXX')
#    conf.find_program('gcc-ar', path_list=[snap_gcc_bin, ], var='AR')
    # try:
    #   distcc = conf.find_program('distcc')
    #   cc = distcc + cc
    #   cxx = distcc + cxx
    # except conf.errors.ConfigurationError:
    #   conf.to_log("distcc not found complie local instead")

    conf.get_cc_version(cc, gcc=True)
    conf.get_cc_version(cxx, gcc=True)
    conf.env.CC_NAME = 'gcc'
    conf.env.CC = cc
    conf.env.CXX_NAME = 'gcc'
    conf.env.CXX = cxx
    #conf.env.append_unique('RPATH', snap_gcc_lib64)
    
def AddBoostRPath(conf):
  conf.env.append_value('RPATH', conf.env.LIBPATH_BOOST) 
  conf.env.LIB_FUCKDEP = ['comm', 'price', 'util']


def configure(conf):
  LoadSnapGcc(conf)
  conf.load('compiler_c compiler_cxx boost')
  conf.check_boost(lib='system filesystem thread date_time iostreams program_options  log_setup log')
  AddBoostRPath(conf)
  conf.load('quickfix', tooldir=custom_tool_dir)
#  conf.load('glog', tooldir=custom_tool_dir)
  conf.load('leveldb', tooldir=custom_tool_dir)
  conf.env.append_value('CXXFLAGS', cxx_flags)
  conf.env.append_value('CFLAGS', c_flags)
  conf.env.append_value('LINKFLAGS', link_flags)
      
def build(bld):
  bld.recurse(target_folders)
