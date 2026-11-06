#!/usr/bin/env python3
"""Smoke + T04 acceptance; guest toolchain fixtures must be supplied explicitly."""
import argparse, hashlib, json, os, pathlib, shutil, subprocess
EXPECTED={"test04.cah":"983c1855e689cca1214f7ff02cd5bd25b9d18a2ce7ed94c13768ea86d9d21921","test04.cfb":"2020f4fd16f37388448da2f485c2f9fa5ed03ff331c972af10c2494403f6158a","test04.btl":"0e809905275fd653eb36423c11dab209e9b0b6d670ca6eefefd6ec175d801f43"}
def run(executable,fixtures,output,smoke_executable=None):
    output.mkdir(parents=True,exist_ok=False)
    for p in fixtures.iterdir():
        if p.is_file() and p.name not in EXPECTED:shutil.copy2(p,output/p.name)
    stages=[('smoke',['ILINK.BTL','test04.tah','hostio.lib','convert.lib','/f','occama.lnk','/o','test04.cah'],None),('ILINK',['ILINK.BTL','test04.tah','hostio.lib','convert.lib','/f','occama.lnk','/o','test04.cah'],'test04.cah'),('OCCONF',['OCCONF.BTL','hello.pgm','/o','test04.cfb'],'test04.cfb'),('ICOLLECT',['ICOLLECT.BTL','test04.cfb','/o','test04.btl'],'test04.btl'),('runtime',['test04.btl'],None)]
    results=[]
    for stage,args,artifact in stages:
        env=os.environ.copy();env['IBOARDSIZE']='#400000';env['T425_ISERVER_SPID_PROFILE']='compat-v14';env.pop('T425_SIM_SMOKE_ON_COMMAND',None)
        if stage=='smoke':env['T425_SIM_SMOKE_ON_COMMAND']='1'
        with (output/(stage+'.stdout')).open('wb') as out,(output/(stage+'.stderr')).open('wb') as err:
            selected=smoke_executable if stage=='smoke' and smoke_executable else executable
            rc=subprocess.run([str(selected),'-sz','-sl','T425SIM0','-sb',*args],cwd=output,env=env,stdout=out,stderr=err,timeout=900).returncode
        stdout=(output/(stage+'.stdout')).read_text(errors='replace');stderr=(output/(stage+'.stderr')).read_text(errors='replace')
        digest=hashlib.sha256((output/artifact).read_bytes()).hexdigest() if artifact and (output/artifact).exists() else None
        ok=rc==0 and (('T425SIM_SMOKE_PASS' in stderr) if stage=='smoke' else ('cpu_error=0 iserver_status=0' in stderr and '(exit with 0)' in stdout))
        if artifact:ok=ok and digest==EXPECTED[artifact]
        if stage=='runtime':ok=ok and 'Hello world...' in stdout
        results.append(dict(stage=stage,result='PASS' if ok else 'FAIL',exit=rc,artifact=artifact,sha256=digest))
        (output/'results.json').write_text(json.dumps(results,indent=2))
        if not ok:raise SystemExit('First failed stage: '+stage)
    return results
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--executable',type=pathlib.Path,required=True);p.add_argument('--smoke-executable',type=pathlib.Path);p.add_argument('--fixtures',type=pathlib.Path,required=True);p.add_argument('--output',type=pathlib.Path,required=True);a=p.parse_args();print(json.dumps(run(a.executable.resolve(),a.fixtures.resolve(),a.output.resolve(),a.smoke_executable.resolve() if a.smoke_executable else None),indent=2))
