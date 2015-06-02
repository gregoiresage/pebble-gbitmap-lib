
#
# This file is the default set of rules to compile a Pebble project.
#
# Feel free to customize this to your needs.
#

import os.path

top = '.'
out = 'build'

def options(ctx):
    ctx.load('pebble_sdk')

def configure(ctx):
    ctx.load('pebble_sdk')

def build(ctx):
    ctx.load('pebble_sdk')

    build_worker = os.path.exists('worker_src')
    binaries = []

    for p in ctx.env.TARGET_PLATFORMS:
        ctx.set_env(ctx.all_envs[p])
        ctx.set_group(ctx.env.PLATFORM_NAME)
        app_elf='{}/pebble-app.elf'.format(ctx.env.BUILD_DIR)
        ctx.pbl_program(source=ctx.path.ant_glob('src/**/*.c'),
        target=app_elf)

        ctx.stlib(
            source = ctx.path.ant_glob('src/**/*.c'), 
            target = str(p) + '/gbitmap-tools')

        ctx(rule='cp ${SRC} ${TGT}', source=str(p) + '/libgbitmap-tools.a', target='../delivery/lib/' +str(p) + '/libgbitmap-tools.a')

    ctx(rule='cp ${SRC} ${TGT}', source='src/gbitmap_tools.h', target='../delivery/include/gbitmap_tools.h')
    