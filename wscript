#!/usr/bin/python

top = '.'
out = '_build'


packageList = 'interface strategy util backtestbed bluemessage'
protobuf_base = 'protobuf-2.6.1'
protobuf_src = protobuf_base + '/src'

BluesCXXFlag= ['-std=gnu++14' , '-O2' ,'-ggdb']
#,'-I/usr/local/gcc53/include/c++/5.3.0/'
#BluesCXXFlag= ['-std=c++1y' , '-O2', '-ggdb']



import sys
def options(opt):
        opt.load('compiler_cxx boost')

        #conf.check(header_name='windows.h', features='c cprogram', define_name='BLUE_WINDOWS', mandatory=False)
        opt.load('waf_unit_test')
        opt.recurse(packageList)
    
from waflib.Configure import conf

def gtest_results(bld):
    lst = getattr(bld, 'utest_results', [])
    if not lst:
        return
    for (f, code, out, err) in lst:
        # if not code:
        #     continue

        # uncomment if you want to see what's happening
        # print(str(out, 'utf-8'))
        output = str(out).splitlines()
        from waflib import Logs
        for i, line in enumerate(output):
            if '[ RUN      ]' in line and code:
                i += 1
                if '    OK ]' in output[i]:
                    continue
                while not '[ ' in output[i]:
                    Logs.warn('%s' % output[i])
                    i += 1
            elif ' FAILED  ]' in line and code:
                Logs.error('%s' % line)
            elif ' PASSED  ]' in line:
                Logs.info('%s' % line)


def configure(conf):
        print (sys.platform)
        if sys.platform == 'win32' or sys.platform == 'cygwin':
            conf.define('_BL_WIN32_PLATFROM_', 1)
           # conf.env['CXX'] = "/usr/bin/x86_64-w64-mingw32-g++.exe"
           # conf.env.append_unique("CXXFLAGS", ["-D__USE_W32_SOCKETS"])
        else:
            conf.define('_BL_LINUX_PLATFROM_', 1)

       # conf.env['CXX'] = "/cygdrive/c/MinGW/bin/g++.exe"
       # conf.load('g++')


        conf.load('compiler_cxx  boost')
        conf.check_boost("filesystem system serialization")
        conf.load('waf_unit_test')
        conf.env.append_unique('CXXFLAGS', BluesCXXFlag)

        #conf.find_program('protoc', var='PROTOC')
        #conf.env.PROTOC_ST = '-I%s'

        conf.recurse(packageList)
        try:
                conf.find_program('cpplint.py', var='CPPLINT', path_list = 'cpplint')
        except conf.errors.ConfigurationError:
                conf.to_log("run 'git submoudle'")
                print("run 'git submoudle'")



def buildProtoBuf(bld):
        #protoc_srcfiles =  protobuf_src + '/google/protobuf/compiler/cpp/*.cc ' + protobuf_src + '/google/protobuf/compiler/*.cc' ,
        exclpatten = ['**/*unittest.cc' , '**/mock_*', '**/test*']        
        protoc_srcfiles = bld.path.ant_glob(protobuf_src + '/google/protobuf/compiler/cpp/*.cc ', excl = exclpatten) + bld.path.ant_glob(protobuf_src + '/google/protobuf/compiler/*.cc ', excl = exclpatten)
        protobuf_srcfiles = bld.path.ant_glob(protobuf_src + '/google/**/*cc ',  excl = exclpatten) 


        bld.stlib(
                source          = protobuf_srcfiles,
                target          = 'protobuf',
                includes =  protobuf_src + ' ' + protobuf_base,
                export_includes = protobuf_src,
                name = 'PROTOBUF'
               # use             =  ['com_interface', 'util', 'BOOST']
               ) 
        bld.program(
                source          = protoc_srcfiles,
                target = 'protoc',
                includes =  protobuf_src + ' ' + protobuf_base,
                use             =  ['PROTOBUF'],

                name = 'PROTOC'
                )
       



def buildgmock(bld):
        gtestinc = bld.path.abspath() + '/googletest/googletest/include'
        gmockinc = bld.path.abspath() + '/googletest/googlemock/include'
        gtestsrc = bld.path.abspath() + '/googletest/googletest'
        gmocksrc = bld.path.abspath() + '/googletest/googlemock'
        gmockinclude = gtestinc + " " +  gmockinc + " " + gtestsrc +" " + gmocksrc
        # print (gmockinclude)
        bld.stlib(
        source          = ['googletest/googlemock/src/gmock_main.cc', 'googletest/googlemock/src/gmock-all.cc',
                            'googletest/googletest/src/gtest-all.cc'],
        target          = 'gmock',
        includes     =  gmockinclude,
        export_includes = [gtestinc, gmockinc],
       # cxxflags     = ['-std=gnu++11', '-O2'],
        lib = ['pthread']
        )

def build(bld):
        buildgmock(bld)
        buildProtoBuf(bld)
        bld.add_group()
        bld.recurse(packageList)
        bld.add_post_fun(gtest_results)
        #from waflib.Tools import waf_unit_test
        #bld.add_post_fun(waf_unit_test.summary)




import re
from waflib.Task import Task
from waflib.TaskGen import extension 




class protoc(Task):
    # protoc expects the input proto file to be an absolute path.


    run_str = '${PROTOC_BIN} ${PROTOC_FLAGS} ${XXPROTOC_FLAGS} ${SRC[0].abspath()}'
    color   = 'BLUE'
    ext_out = ['.h', 'pb.cc']
    # def scan(self):
    #     """
    #     Scan .proto dependencies
    #     """
    #     node = self.inputs[0]

    #     nodes = []
    #     names = []
    #     seen = []

    #     if not node: return (nodes, names)

    #     def parse_node(node):
    #         if node in seen:
    #             return
    #         seen.append(node)
    #         code = node.read().splitlines()
    #         for line in code:
    #             m = re.search(r'^import\s+"(.*)";.*(//)?.*', line)
    #             if m:
    #                 dep = m.groups()[0]
    #                 for incpath in self.env.INCPATHS:
    #                     found = incpath.find_resource(dep)
    #                     if found:
    #                         nodes.append(found)
    #                         parse_node(found)
    #                     else:
    #                         names.append(dep)

    #     parse_node(node)
    #     return (nodes, names)

@extension('.proto')
def process_protoc(self, node):
    cpp_node = node.change_ext('.pb.cc')
    hpp_node = node.change_ext('.pb.h')


    protoc_comp = self.bld.get_tgen_by_name('PROTOC').link_task.outputs[0];
   # protoc_comp = '/home/snake/sr/temp/ngtest/temp/protoc.exe'

    self.create_task('protoc', [node], [cpp_node, hpp_node])
    self.source.append(cpp_node)
    self.env.PROTOC_BIN = protoc_comp.abspath();
    self.env.PROTOC_FLAGS = '--cpp_out=%s' % node.parent.get_bld().abspath() 
    self.env.XXPROTOC_FLAGS = '-I%s' % node.parent.abspath()
    # use = getattr(self, 'use', '')
    # if not 'PROTOBUF' in use:
    #     self.use = self.to_list(use) + ['PROTOBUF']