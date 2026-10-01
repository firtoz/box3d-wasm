from pathlib import Path
from atomic_io import atomic_json
import ast,json,hashlib,sys
p=Path(__file__).resolve().parent;cell,stage,screen=sys.argv[1:4];d=p/cell
plans=ast.literal_eval(next(n.value for n in ast.parse((p/'stage.py').read_text()).body if isinstance(n,ast.Assign)and any(isinstance(t,ast.Name)and t.id=='plans'for t in n.targets)))
a=plans[stage]
if len(sys.argv)>4:
 coords=json.loads(sys.argv[4])
 for act in a:
  if act.get('label')in coords:act['x'],act['y']=coords[act['label']]
atomic_json(d/(stage+'-reviewed-plan.json'),{'screen':screen,'screen_sha256':hashlib.sha256((d/screen).read_bytes()).hexdigest(),'review':'Agent inspected this actual current PNG before selecting displayed widget/tab positions; coordinates in actions. No stale coordinates across lifetime/layout transition.','actions':a})
