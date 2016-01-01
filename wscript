top = '.'
out = '_build'


packageList = 'interface strategy util backtestbed'

BluesCXXFlag= ['-std=gnu++11' , '-O2']




import sys
def options(opt):
        opt.load('compiler_cxx')

        #conf.check(header_name='windows.h', features='c cprogram', define_name='BLUE_WINDOWS', mandatory=False)
        opt.load('waf_unit_test')

        opt.recurse(packageList)
        # conf.check(lib='gtest', uselib_store='GTEST')
    #opt.load("compiler_cxx")
    
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
        else:
            conf.define('_BL_LINUX_PLATFROM_', 1)

        conf.load('compiler_cxx')
        conf.load('waf_unit_test')

        conf.env.append_unique('CXXFLAGS', BluesCXXFlag)
        conf.recurse(packageList)
        try:
                conf.find_program('cpplint.py', var='CPPLINT', path_list = 'cpplint')
        except conf.errors.ConfigurationError:
                conf.to_log("run 'git submoudle'")
                print("run 'git submoudle'")


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
        lib = ['pthread']


        )

def build(bld):
        buildgmock(bld)
        bld.recurse(packageList)
        bld.add_post_fun(gtest_results)
        #from waflib.Tools import waf_unit_test
        #bld.add_post_fun(waf_unit_test.summary)

