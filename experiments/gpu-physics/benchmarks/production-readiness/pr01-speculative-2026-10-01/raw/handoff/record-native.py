from pathlib import Path
import hashlib,json,os,subprocess,time,signal
root=Path.cwd();out=root/'artifacts/production-readiness/pr01-speculative/handoff';sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
protocol=out/'recording-protocol.json';d=json.loads(protocol.read_text());d['native_checks']={'floor_center_min':36*.0254-.005,'ghost_launch':'upward speed >0.5m/s while center <36*.0254+.01+4*.0254 (upstream scene threshold)','health':'300 completed/submitted steps,finite state,no Sokol errors or capacity loss'};protocol.write_text(json.dumps(d,indent=2)+'\n')
xvfb=out/'xvfb/usr/bin/Xvfb';display=':97';assert not Path('/tmp/.X97-lock').exists()
xlog=(out/'xvfb.log').open('w');server=subprocess.Popen([str(xvfb),display,'-screen','0','1280x720x24','-nolisten','tcp','-ac'],stdout=xlog,stderr=subprocess.STDOUT)
env=os.environ.copy();env.update(DISPLAY=display,GPU_BENCH_WIDTH='1280',GPU_BENCH_HEIGHT='720',GPU_PHYSICS_ADAPTER='nvidia',GPU_PHYSICS_BACKEND='vulkan',WGPU_BACKEND='vulkan',GPU_PHYSICS_LIVE_CONTACT_ORDER='1',VK_DRIVER_FILES='/usr/share/vulkan/icd.d/nvidia_icd.json',GPU_PHYSICS_PIPELINE_CACHE_DIR=str(out/'pipelines'),__NV_PRIME_RENDER_OFFLOAD='0',__GLX_VENDOR_LIBRARY_NAME='mesa',LIBGL_ALWAYS_SOFTWARE='1')
try:
 for _ in range(100):
  if subprocess.run(['xdpyinfo','-display',display],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL).returncode==0:break
  if server.poll() is not None:raise RuntimeError('Xvfb failed')
  time.sleep(.1)
 for kind,directory,target,label in [('cpu',out.parent/'cpu-cmake','samples_cpu','000-box3d-cpu'),('gpu',out/'native-gpu/cmake','samples_gpu','2026-10-01-speculative-handoff')]:
  result=out/f'native-scene-{kind}';result.mkdir();binary=directory/'bin'/target
  # Freeze the complete known source trees and build inputs before the serial clean build.
  paths=[]
  for base in [root/'c_abi',root/'native-samples',root/'src',root/'shaders',root.parent.parent/'box3d/include',root.parent.parent/'box3d/src',root.parent.parent/'box3d/samples',root.parent.parent/'box3d/shared',root.parent.parent/'box3d/gfx']:
   if base.exists():paths.extend(p for p in base.rglob('*') if p.is_file() and p.suffix in ['.c','.cpp','.h','.rs','.wgsl','.txt','.py','.sh'] and not any(part.startswith('build') for part in p.relative_to(base).parts))
  paths.extend(p for p in directory.glob('*') if p.is_file() and p.suffix in ['.cpp','.c','.h'])
  before={os.path.relpath(p,root):sha(p) for p in paths}
  command=['cmake','--build',str(directory),'--clean-first','--target',target,'-j4']
  with (result/'build.log').open('w') as f:code=subprocess.run(command,stdout=f,stderr=subprocess.STDOUT).returncode
  assert code==0 and before=={os.path.relpath(p,root):sha(p) for p in paths}
  link=(directory/f'CMakeFiles/{target}.dir/link.txt').read_text()
  linked={str(p):sha(p) for p in directory.rglob('*.a')}
  if kind=='gpu':
   library=directory.parent/'libgpu_physics.a';assert sha(library)==json.loads((out/'build-native.json').read_text())['binary_sha256'];linked[str(library)]=sha(library)
  build={'command':command,'exit':code,'source_inputs_before_and_after':before,'linked_archives':linked,'link_command':link,'binary_sha256':sha(binary),'log_sha256':sha(result/'build.log'),'gpu_engine_receipt':'../build-native.json' if kind=='gpu' else None,'dependencies':{str(p):subprocess.check_output(['git','-C',str(p),'rev-parse','HEAD'],text=True).strip() for p in (root.parent.parent/'box3d/.fetchcontent-cache').glob('*-src') if (p/'.git').exists()}}
  (result/'build.json').write_text(json.dumps(build,indent=2)+'\n')
  capture_env=env.copy()
  if kind=='gpu':
   raw=subprocess.check_output(['bash','-c','source scripts/native-samples-cache-env.sh; env -0'])
   for item in raw.split(b'\0'):
    if item.startswith((b'GPU_PHYSICS_',b'WGPU_')):
     key,value=item.decode().split('=',1);capture_env[key]=value
  video=root/f'recordings/snapshots/{label}/sbox-ghost-collisions.mp4';video.parent.mkdir(parents=True,exist_ok=True)
  assert not video.exists()
  ffcommand=['ffmpeg','-hide_banner','-loglevel','warning','-f','x11grab','-draw_mouse','0','-framerate','30','-video_size','1280x720','-i',display,'-c:v','libx264','-preset','veryfast','-crf','20','-pix_fmt','yuv420p','-movflags','+faststart',str(video)]
  with (result/'ffmpeg.log').open('w') as f:
   ff=subprocess.Popen(ffcommand,stdin=subprocess.PIPE,stdout=f,stderr=subprocess.STDOUT,env=capture_env);time.sleep(.5)
   command=[str(binary),'--sample-name','s&box Ghost Collisions','--bench-json',str(result/'health.json'),'--warmup','0','--timed','300','--paced','--completed-step','--no-sleep','--health-scan']
   (result/'launch.json').write_text(json.dumps({'command':command,'capture_command':ffcommand,'binary_sha256':sha(binary),'environment':{k:v for k,v in capture_env.items() if k.startswith(('GPU_','WGPU_','DISPLAY','VK_','__NV','__GLX','LIBGL'))},'purpose':'visual/correctness, not performance','running_pid':os.getpid()},indent=2)+'\n')
   with (result/'stdout').open('w') as stdout,(result/'stderr').open('w') as stderr:code=subprocess.run(command,env=capture_env,stdout=stdout,stderr=stderr,timeout=180).returncode
   ff.communicate(b'q\n',timeout=30)
  assert code==0 and ff.returncode==0
  probe=json.loads(subprocess.check_output(['ffprobe','-v','error','-show_streams','-of','json',str(video)],text=True))
  (result/'capture.json').write_text(json.dumps({'exit':code,'ffmpeg_exit':ff.returncode,'video_sha256':sha(video),'probe':probe,'includes_startup_loading':True},indent=2)+'\n')
  print(kind,'captured',flush=True)
finally:
 server.terminate();server.wait();xlog.close()
