from pathlib import Path
import os,sys,json,subprocess,time
# Own one isolated diagnostic X server and persist the actual viewer status.
xvfb,receipt,*command=sys.argv[1:];out=Path(receipt).parent;display=next(i for i in range(310,410) if not Path('/tmp/.X11-unix/X'+str(i)).exists() and not Path('/tmp/.X'+str(i)+'-lock').exists());env=os.environ.copy();env['DISPLAY']=':'+str(display);args=[xvfb,env['DISPLAY'],'-screen','0','640x360x24','-nolisten','tcp','-ac'];result={'display':env['DISPLAY'],'command':args};server=None
try:
 with (out/'xvfb.log').open('w') as log:
  server=subprocess.Popen(args,stdout=log,stderr=subprocess.STDOUT)
  for _ in range(200):
   assert server.poll() is None,'Xvfb failed'
   if Path('/tmp/.X11-unix/X'+str(display)).exists():break
   time.sleep(.05)
  else:raise RuntimeError('Xvfb initialization timeout')
  process=subprocess.run(command,env=env);result['child_exit']=process.returncode;Path(receipt).write_text(json.dumps({'exit':process.returncode})+'\n');exitcode=process.returncode if process.returncode>=0 else 128-process.returncode
finally:
 if server is not None:
  server.terminate()
  try:result['server_exit']=server.wait(timeout=10)
  except subprocess.TimeoutExpired:server.kill();result['server_exit']=server.wait()
 (out/'display-exit.json').write_text(json.dumps(result,indent=2)+'\n')
raise SystemExit(exitcode)
