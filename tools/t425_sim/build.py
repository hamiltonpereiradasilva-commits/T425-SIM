#!/usr/bin/env python3
"""Build the functional T425-SIM profile from repository sources only."""
import argparse, hashlib, json, pathlib, subprocess
ROOT=pathlib.Path(__file__).resolve().parents[2]
def build(out):
    out=out.resolve();out.mkdir(parents=True,exist_ok=True)
    core=ROOT/"source/t425_core";vm=ROOT/"source/t425_vm"
    inmos=ROOT/"third_party/inmos/iserver-1.50v/iserver";isrc=inmos/"iserver/src"
    includes=[vm/"include",vm/"iserver_adapter",vm/"transport",vm/"platform/posix/include",core/"include",core/"channel",core/"process",core/"scheduler",core/"timer",core/"microcode",core/"phy",ROOT/"source/t425_phy/rp2040/core_adapter",isrc,inmos/"linkops/src",inmos/"linkios/src"]
    inc=[x for p in includes for x in ["-I",str(p)]]
    common=["-std=c11","-g","-ffile-prefix-map="+str(ROOT)+"=.","-D_DEFAULT_SOURCE","-D_POSIX_C_SOURCE=200809L","-DERRNO=errno","-include","errno.h","-DT425SIM",*inc]
    original=["-std=gnu11","-ffile-prefix-map="+str(ROOT)+"=.","-O2","-g","-mtune=generic","-march=x86-64","-fasynchronous-unwind-tables","-fstack-protector-strong","-fstack-clash-protection","-fcf-protection","-D_FORTIFY_SOURCE=2","-D_POSIX_C_SOURCE=200809L","-DT425SIM",*inc,"-include",str(ROOT/"third_party/inmos/compat/iserver_posix.h")]
    prefix='\n'.join(['#define HOST_TTY_NAME "/dev/tty"','#include <stdio.h>','#include <stdlib.h>','#include <unistd.h>','static int execute();','static int initcmds();'])+'\n'
    raw=(isrc/"sh.c").read_text();sh=out/"iserver_sh_compat.c"
    sh.write_text(prefix+''.join(l.replace('static int debug;','int debug;') for l in raw.splitlines(True) if 'malloc();' not in l))
    commands=[];units=[];objects=[]
    def compile_one(src,obj,flags,group):
        args=["cc",*flags,"-MD","-MF",str(obj.with_suffix(".d")),"-c",str(src),"-o",str(obj)]
        commands.append(args)
        subprocess.run(args,check=True)
        objects.append(str(obj));units.append(dict(source=str(src),object=str(obj),group=group,source_sha256=hashlib.sha256(src.read_bytes()).hexdigest()))
    for i,relative in enumerate(json.loads((ROOT/"tools/t425_sim/units.json").read_text())):
        compile_one(ROOT/relative,out/(str(i).zfill(3)+"_"+pathlib.Path(relative).stem+".o"),common,"productive")
    for name in ['filec','files','hostc','info','iserver','misc','pack','parsecl','record','serve','serverc','sh','ttyio']:
        compile_one(sh if name=='sh' else isrc/(name+'.c'),out/('inmos_'+name+'.o'),original+(['-Dmain=t425_inmos_iserver_main'] if name=='iserver' else []),"INMOS original")
    wraps=['t425_core_channel_dma_service','ReadLink','WriteLink','OPS_SendReply','SpId']
    def link(name,objs,extra):
        args=['cc',*["-Wl,--wrap="+x for x in wraps+extra],*objs,'-lusb-1.0','-Wl,-Map,'+str(out/(name+'.map')),'-o',str(out/name)]
        commands.append(args);subprocess.run(args,check=True)
    link('t425-sim',objects,[])
    saved=list(objects)
    compile_one(ROOT/'tests/t425_sim/service_trace.c',out/'service_trace.o',common,'test observer only')
    link('t425-sim-test',saved+[str(out/'service_trace.o')],['Serve','OPS_GetRequest','OPS_BootWrite','SpCommand'])
    (out/'build_manifest.json').write_text(json.dumps(dict(compiler=subprocess.check_output(['cc','--version']).decode(),units=units,commands=commands),indent=2))
    print('T425-SIM build complete:',out)
if __name__=='__main__':
    parser=argparse.ArgumentParser();parser.add_argument('--build-dir',type=pathlib.Path,default=ROOT/'build/t425_sim');args=parser.parse_args();build(args.build_dir)
