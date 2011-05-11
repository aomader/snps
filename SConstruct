env = Environment()
env.Append(CCFLAGS=['-O3', '-Wall', '-pedantic', '-std=c99'])
env.Append(CPPPATH=['src'])

env.ParseConfig('pkg-config --cflags --libs glib-2.0')
env.Library('libsnps', Glob('src/libsnps/*.c'), srcdir='src/libsnps')

env.Append(LIBS=['libsnps', 'm'])
env.Append(LIBPATH=['.'])
env.Program('demo1', 'demo1.c', srcdir='src/demo')
env.Program('demo2', 'demo2.c', srcdir='src/demo')
