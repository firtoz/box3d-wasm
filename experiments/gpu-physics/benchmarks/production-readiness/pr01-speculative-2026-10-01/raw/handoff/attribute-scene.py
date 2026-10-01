from pathlib import Path
import subprocess,json,hashlib,os
root=Path.cwd();out=root/'artifacts/production-readiness/pr01-speculative/handoff';base=out.parent/'baseline-source/experiments/gpu-physics';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest();result=out/'sbox-baseline-attribution';result.mkdir()
(result/'protocol.json').write_text(json.dumps({'hypothesis':'The new 5mm screen fails during initial landing in CPU and candidate at exactly the same height. Determine whether frozen pre-PR01 GPU sources reproduce this existing discrete landing rather than treating an unchanged event as a speculative-control regression.','budget':{'candidate_runs':0,'baseline_diagnostic_runs':1,'steps':300},'settings':'Same native cache/ordering,NVIDIA/Vulkan,scene defaults,dt1/60,4substeps,completed health,no-sleep,paced','required':'Retain original screen failure. Baseline must complete all300steps. Report comparison and any new/worse excursion; no performance or final physical qualification follows.','stop':'One baseline diagnostic; retain result; no selective retry/physics edits within this protocol.'},indent=2)+'\n')
lib=out.parent/'baseline-native.a';assert sha(lib)==json.loads((out.parent/'baseline-native-build.json').read_text())['binary_sha256']
directory=result/'cmake'
for tag,cmd in [('configure',['cmake','-S',str(base/'native-samples'),'-B',str(directory),'-DBOX3D_DIR='+str(root.parent.parent/'box3d'),'-DGPU_PHYSICS_DIR='+str(base),'-DGPU_PHYSICS_LIB='+str(lib),'-DGPU_SAMPLES=ON','-DBOTH_SAMPLES=OFF','-DGPU_PORTABLE_API=ON','-DCMAKE_BUILD_TYPE=Release','-DFETCHCONTENT_FULLY_DISCONNECTED=ON']),('build',['cmake','--build',str(directory),'--target','samples_gpu','-j4'])]:
 inputs={str(p.relative_to(base)):sha(p) for p in base.rglob('*') if p.is_file()}
 with (result/f'{tag}.log').open('w') as f:code=subprocess.run(cmd,stdout=f,stderr=subprocess.STDOUT).returncode
 assert inputs=={str(p.relative_to(base)):sha(p) for p in base.rglob('*') if p.is_file()}
 (result/f'{tag}.json').write_text(json.dumps({'command':cmd,'exit':code,'archive_revision':'bd4f7966151028542297be81a330e80e83687531','adapter_inputs_before_and_after':inputs,'rust_library_sha256':sha(lib),'rust_build_receipt':'../../baseline-native-build.json','log_sha256':sha(result/f'{tag}.log')},indent=2)+'\n')
 if code:raise SystemExit(code)
# A private virtual screen captures only this task's window.
import time
xlog=(result/'xvfb.log').open('w');server=subprocess.Popen([str(out/'xvfb/usr/bin/Xvfb'),':97','-screen','0','1280x720x24','-nolisten','tcp','-ac'],stdout=xlog,stderr=subprocess.STDOUT)
try:
 env=os.environ.copy();raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'])
 for item in raw.split(b'\0'):
  if item.startswith((b'GPU_PHYSICS_',b'WGPU_')):
   k,v=item.decode().split('=',1);env[k]=v
 env.update(DISPLAY=':97',GPU_BENCH_WIDTH='1280',GPU_BENCH_HEIGHT='720',GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(out/'pipelines'),__NV_PRIME_RENDER_OFFLOAD='0',__GLX_VENDOR_LIBRARY_NAME='mesa',LIBGL_ALWAYS_SOFTWARE='1')
 for _ in range(100):
  if subprocess.run(['xdpyinfo','-display',':97'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL).returncode==0:break
  time.sleep(.1)
 binary=directory/'bin/samples_gpu';video=result/'sbox-ghost-collisions.mp4'
 with (result/'ffmpeg.log').open('w') as f:
  fc=['ffmpeg','-hide_banner','-loglevel','warning','-f','x11grab','-draw_mouse','0','-framerate','30','-video_size','1280x720','-i',':97','-c:v','libx264','-preset','veryfast','-crf','20','-pix_fmt','yuv420p',str(video)];ff=subprocess.Popen(fc,stdin=subprocess.PIPE,stdout=f,stderr=subprocess.STDOUT,env=env);time.sleep(.5)
  cmd=[str(binary),'--sample-name','s&box Ghost Collisions','--bench-json',str(result/'health.json'),'--warmup','0','--timed','300','--paced','--completed-step','--no-sleep','--health-scan']
  with (result/'stdout').open('w') as stdout,(result/'stderr').open('w') as stderr:code=subprocess.run(cmd,env=env,stdout=stdout,stderr=stderr,timeout=180).returncode
  ff.communicate(b'q\n',timeout=30)
 (result/'run.json').write_text(json.dumps({'command':cmd,'exit':code,'capture_exit':ff.returncode,'environment':{k:v for k,v in env.items() if k.startswith(('GPU_','WGPU_','VK_','DISPLAY','LIBGL','__GLX','__NV'))},'binary_sha256':sha(binary),'video_sha256':sha(video)},indent=2)+'\n')
 assert code==0 and ff.returncode==0
 print('baseline diagnostic complete',flush=True)
finally:server.terminate();server.wait();xlog.close()
