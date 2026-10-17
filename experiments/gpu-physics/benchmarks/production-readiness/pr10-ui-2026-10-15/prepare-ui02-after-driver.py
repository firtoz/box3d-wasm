from pathlib import Path
import hashlib,json
D=Path(__file__).resolve().parent;s=(D/'ui02-before-driver.py').read_text();s=s.replace("len(r['actions'])<20","len(r['actions'])<40")
s=s.replace("check('panel wheel incorrectly changes camera',abs(after-before)>.05)","check('panel first wheel leaves camera unchanged',abs(after-before)<.05)")
s=s.replace("check('world first wheel incorrectly swallowed',abs(after-before)<.05)","check('world first wheel reaches camera',abs(after-before)>.05)")
insert=r'''
  # An existing world-owned button gesture retains the original capture route.
  hover(*world);q=frames();before=radius(q);index=len(events())
  action('world-owned left down',lambda:(xdo('mousedown',1),held.add('mouse1')))
  action('held world gesture wheel over panel',lambda:burst(*panel));q=frames();after=radius(q);rows=events()[index:];wheel=next(x for x in rows if abs(x['scroll'][1])>0)
  r['held_case']={'radius_before':before,'radius_after':after,'actual_handler_scroll':wheel}
  check('held world button modifier preserved',bool(wheel['modifiers']&256));check('held world capture route preserved',not wheel['captured']and abs(after-before)>.05)
  action('release world-owned left',lambda:(xdo('mouseup',1),held.remove('mouse1')))
  hover(*world);q=frames();before=radius(q);key('ctrl+o');q=frames();index=len(events());action('popup outside wheel',lambda:burst(*world));q=frames();after=radius(q);rows=events()[index:];wheel=next(x for x in rows if abs(x['scroll'][1])>0)
  r['popup_case']={'radius_before':before,'radius_after':after,'actual_handler_scroll':wheel,'ui_windows':sorted(set(x['window']for x in q['items']))}
  check('popup owns outside wheel',wheel['captured']and abs(after-before)<.05);key('Escape');q=frames()
  key('period');check('keyboard step after popup preserved',state()['sample_step']==r['actions'][0]['state_after']['sample_step']+1)
'''
s=s.replace(' finally:X.XCloseDisplay(d)',insert+' finally:X.XCloseDisplay(d)')
s=s.replace("r['status']='UI02_DEFECT_REPRODUCED_ACTUAL_HANDLER'","r['status']='UI02_REPAIR_VERIFIED_ACTUAL_INPUT'").replace("emit('UI02 both directions reproduced; no repair yet')","emit('UI02 both directions repaired; held gesture/popup/keyboard checked')")
(D/'ui02-after-driver.py').write_text(s);(D/'ui02-after-driver-protocol.json').write_text(json.dumps({'status':'FROZEN_BEFORE_RUN','driver_sha256':hashlib.sha256(s.encode()).hexdigest(),'order':['cpu','ordinary-gpu','native-gpu','ordinary-both','native-both'],'apps':5,'seconds_each':60,'ready_seconds':25,'screens_each':3,'actions_each':40,'retries':0,'actual_inputs':['pause','queued enter+wheel','queued leave+wheel','held world left button +panel wheel +release','popup outsidewheel/Escape','single step/CtrlQ']},indent=2)+'\n')
print('after driver frozen')
