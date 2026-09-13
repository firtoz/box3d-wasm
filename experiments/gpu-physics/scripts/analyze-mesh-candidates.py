#!/usr/bin/env python3
"""Compare observed mesh candidates and retained witnesses; diagnostic, not a gate."""
import argparse,json,math,hashlib
from pathlib import Path
p=argparse.ArgumentParser(description=__doc__)
p.add_argument('log',type=Path);p.add_argument('out',type=Path)
a=p.parse_args();log=a.log.read_text()
assert 'Validation Error' not in log and 'overflow true' not in log
rows=[]
for line in log.splitlines():
 if line.startswith('mesh-candidate 2 1 '):
  r=line.split();rows.append({'point':list(map(float,r[6:9])),'separation':float(r[10]),'triangle':int(r[18]),'feature':int(r[20])})
assert rows
# This native second penetrating witness is measured in the identical-state
# CPU replay, with mesh at body origin zero. Compare actual world points.
target=[-1.92611849,2.91893005,-0.857758999]
match=min(rows,key=lambda r:math.dist(r['point'],target))
assert math.dist(match['point'],target)<1e-5
# Native b3CullPoints rule, applied once to the full candidate set, unlike the
# runtime's streaming four-point storage. This is deliberately an oracle only.
unique=[]
for r in rows:
 if not any(math.dist(r['point'],s['point'])<1e-5 for s in unique):unique.append(r)
# Tangentially identical points represent the same normal constraint. Preserve
# one for this oracle to avoid the diagnostic reproducing duplicate samples.
tangent_unique=[]
for r in unique:
 if not any(math.hypot(r['point'][0]-v['point'][0],r['point'][2]-v['point'][2])<1e-5 for v in tangent_unique):
  tangent_unique.append(r)
unique=tangent_unique
pts=[(r['point'][0],r['point'][2]) for r in unique]
sep=[r['separation'] for r in unique]
sub=lambda a,b:(a[0]-b[0],a[1]-b[1])
cross=lambda a,b:a[0]*b[1]-a[1]*b[0]
tol=(.25*.005)**2
def better(score,depth,best,bdepth):
 return score>best+tol or (score>=best-tol and depth<bdepth-.005)
best=0.;bd=math.inf;pair=None
for i in range(len(pts)):
 for j in range(i+1,len(pts)):
  d=math.dist(pts[i],pts[j])**2
  if better(d,sep[i]+sep[j],best,bd):best,bd,pair=d,sep[i]+sep[j],(i,j)
assert pair and best>tol
selected=list(pair);a0,b=[pts[i] for i in pair];best=0.;bd=math.inf;third=None
for i in range(len(pts)):
 if i in selected:continue
 area=cross(sub(b,a0),sub(pts[i],a0))
 if better(abs(area),sep[i],best,bd):best,bd,third=abs(area),sep[i],i
if third is not None:
 selected.append(third);c=pts[third]
 if cross(sub(b,a0),sub(c,a0))<0:b,c=c,b
 best=0.;bd=math.inf;fourth=None
 for i in range(len(pts)):
  if i in selected:continue
  point=pts[i];score=max(cross(sub(point,a0),sub(b,a0)),cross(sub(point,b),sub(c,b)),cross(sub(point,c),sub(a0,c)))
  if better(score,sep[i],best,bd):best,bd,fourth=score,sep[i],i
 if fourth is not None:selected.append(fourth)
report={'status':'diagnostic','native_second_witness':target,'generated_match':match,
 'match_distance':math.dist(match['point'],target),'accepted_candidates':rows,
 'full_cluster_oracle_selection':[unique[i] for i in selected],
 'limitations':['CPU full-cluster rule is an offline comparison, not a production GPU implementation.',
 'Source candidate separations omit native 0.005 m mesh rest offset.',
 'This evidence isolates witness loss after generation; native scene acceptance remains fail.'],
 'sha256':{str(f):hashlib.sha256(f.read_bytes()).hexdigest() for f in [a.log,Path(__file__)]}}
a.out.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items() if k not in ['accepted_candidates','sha256']},indent=2))
