from os import unlink, symlink, chdir
from os.path import exists

srcdir = '.'
blddir = 'build'
VERSION = '0.0.1'

def set_options(opt):
	opt.tool_options('compiler_cxx')

def configure(conf):
	conf.check_tool('compiler_cxx')
	conf.check_tool('node_addon')

def build(bld):
	obj = bld.new_task_gen('cxx', 'shlib', 'node_addon')
	obj.target = 'bindings'
	obj.source = 'nodefsevents.cc'
	obj.framework = ['Foundation','CoreServices']

def shutdown():
	# HACK to get bindings.node out of build directory.
	# better way to do this?
	if exists('bindings.node'):
		unlink('bindings.node')
	if exists('build/default/bindings.node') and not exists('bindings.node'):
			symlink('build/default/bindings.node', 'bindings.node')