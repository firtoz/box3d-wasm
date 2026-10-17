from pathlib import Path
import hashlib,json
D=Path(__file__).resolve().parent;old=(D/'ui-auto-driver.py').read_text();src=old.replace("'ui01-build-'+phase", "'ui02-build-'+phase").replace("'ui01-'+phase+'-runs'", "'ui02-'+phase+'-runs'").replace('time.monotonic()-start<180','time.monotonic()-start<60').replace("'180s overall automatic budget'","'60s UI02 budget'").replace("len(r['actions'])<60","len(r['actions'])<20").replace("len(r['screenshots'])<9","len(r['screenshots'])<3").replace("until(state,90)","until(state,25)");src=src.replace("env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');", "env['GPU_PR10_UI_TRACE']=str(dest/'ui.json');env['GPU_PR10_INPUT_TRACE']=str(dest/'inputs.jsonl');")
a=src.index("\n key('p');until(");b=src.index('\nexcept BaseException',a)
scenario=r'''
 key('p');until(lambda:(s if(s:=state())['pause']==1 else None),3)
 import ctypes as c
 X=c.CDLL('libX11.so.6');T=c.CDLL('libXtst.so.6');ptr=c.c_void_p;ulong=c.c_ulong
 X.XOpenDisplay.argtypes=[c.c_char_p];X.XOpenDisplay.restype=ptr;X.XDefaultRootWindow.argtypes=[ptr];X.XDefaultRootWindow.restype=ulong;X.XTranslateCoordinates.argtypes=[ptr,ulong,ulong,c.c_int,c.c_int,c.POINTER(c.c_int),c.POINTER(c.c_int),c.POINTER(ulong)];X.XTranslateCoordinates.restype=c.c_int;X.XFlush.argtypes=[ptr];X.XCloseDisplay.argtypes=[ptr]
 T.XTestFakeMotionEvent.argtypes=[ptr,c.c_int,c.c_int,c.c_int,ulong];T.XTestFakeMotionEvent.restype=c.c_int;T.XTestFakeButtonEvent.argtypes=[ptr,c.c_uint,c.c_int,ulong];T.XTestFakeButtonEvent.restype=c.c_int
 d=X.XOpenDisplay(env['DISPLAY'].encode());assert d
 def radius(q):
  line=next(x['label']for x in q['items']if x['window']=='Info'and x['label'].startswith('radius m '));return float(re.search(r'radius m ([0-9.e+-]+)',line)[1])
 def events():
  p=dest/'inputs.jsonl';return [json.loads(l)for l in p.read_text().splitlines()]if p.exists()else []
 def burst(x,y):
  focus();root=X.XDefaultRootWindow(d);rx=c.c_int();ry=c.c_int();child=ulong();assert X.XTranslateCoordinates(d,wid,root,round(x),round(y),c.byref(rx),c.byref(ry),c.byref(child));assert T.XTestFakeMotionEvent(d,-1,rx.value,ry.value,0);assert T.XTestFakeButtonEvent(d,5,1,0);assert T.XTestFakeButtonEvent(d,5,0,0);X.XFlush(d)
 try:
  q=frames();world=[q['width']*.25,q['height']*.5];hover(*world);q=frames();assert not q['want_capture_mouse'];before=radius(q);screenshot('before');clip=next(x['clip']for x in q['items']if x['window']=='Info');panel=[(clip[0]+clip[2])/2,(clip[1]+clip[3])/2];index=len(events());action('queued enter panel plus wheel',lambda:burst(*panel),{'point':panel,'last_want_capture_mouse':q['want_capture_mouse']});q=frames();after=radius(q);rows=events()[index:];wheel=next(x for x in rows if abs(x['scroll'][1])>0);r['enter_case']={'radius_before':before,'radius_after':after,'actual_handler_scroll':wheel,'paired_events':rows,'camera_changed':abs(after-before)>.05};check('actual enter point inside Info',wheel['inside_info']);check('previous-frame false capture reproduced',not wheel['captured']and not wheel['wanted_before']);check('panel wheel incorrectly changes camera',abs(after-before)>.05);screenshot('enter')
  assert q['want_capture_mouse'];before=radius(q);index=len(events());action('queued leave panel plus wheel',lambda:burst(*world),{'point':world,'last_want_capture_mouse':q['want_capture_mouse']});q=frames();after=radius(q);rows=events()[index:];wheel=next(x for x in rows if abs(x['scroll'][1])>0);r['leave_case']={'radius_before':before,'radius_after':after,'actual_handler_scroll':wheel,'paired_events':rows,'camera_changed':abs(after-before)>.05};check('actual leave point outside Info',not wheel['inside_info']);check('previous-frame true capture reproduced',wheel['captured']and wheel['wanted_before']);check('world first wheel incorrectly swallowed',abs(after-before)<.05);screenshot('leave')
 finally:X.XCloseDisplay(d)
 focus();xdo('key','--clearmodifiers','ctrl+q');app.wait(timeout=5);check('clean quit',app.returncode==0);r['status']='UI02_DEFECT_REPRODUCED_ACTUAL_HANDLER';save();emit('UI02 both directions reproduced; no repair yet')
'''
src=src[:a]+scenario+src[b:];(D/'ui02-before-driver.py').write_text(src);(D/'ui02-driver-protocol.json').write_text(json.dumps({'status':'FROZEN_BEFORE_RUN','driver_sha256':hashlib.sha256(src.encode()).hexdigest(),'shared_helper_source_sha256':hashlib.sha256(old.encode()).hexdigest(),'stimulus':'One XTEST queuedmotion+wheel packet/enter andleave, no servergrab. ActualHandleEvent wrapper tracespoint/capture/frame; actualcamera labelprovidesoutput.','order':['cpu','ordinary-gpu'],'max_apps':2,'seconds_each':60,'ready_seconds':25,'maxscreens_each':3,'maxinputs_each':20,'retries':0},indent=2)+'\n');print('UI02 driver source frozen; actualeventhandler/camera outputs, CPUfirst')
