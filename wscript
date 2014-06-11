
#
# This file is the default set of rules to compile a Pebble project.
#
# Feel free to customize this to your needs.
#

top = '.'
out = 'build'

def options(ctx):
    ctx.load('pebble_sdk')

def configure(ctx):
    ctx.load('pebble_sdk')

def build(ctx):
    ctx.load('pebble_sdk')

    ctx.stlib(
    	source = ctx.path.ant_glob('src/**/*.c'), 
    	target = 'gbitmap-tools')

    ctx(rule='cp ${SRC} ${TGT}', source='libgbitmap-tools.a', target='../delivery/lib/libgbitmap-tools.a')
    ctx(rule='cp ${SRC} ${TGT}', source='src/gbitmap_tools.h', target='../delivery/include/gbitmap_tools.h')
