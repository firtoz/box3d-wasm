#!/usr/bin/env python3
"""Read archived complete CPU/GPU traces; never launches or changes physics."""
from pathlib import Path
import gzip,json,hashlib,math
R=Path(__file__).resolve().parent
sha=lambda p:hashlib.sha256(p.read_bytes()).hexdigest()
def clearance(p,q):
 x,y,z,w=q
 support=.5*(abs(2*(x*y+z*w))+abs(1-2*(x*x+z*z))+abs(2*(y*z-x*w)))
 return p[1]-support
report={}
for kind in ['ordinary','native']:
 path=R/'raw'/f'{kind}-comparison.txt.gz';text=gzip.decompress(path.read_bytes()).decode();current_body=None;data={}
 for line in text.splitlines():
  a=line.split()
  if not a:continue
  if a[0]=='B':current_body=int(a[2])
  elif a[0]=='F' and current_body==0:
   frame,engine=int(a[1]),int(a[2]);vals=list(map(float,a[4:7]+a[8:12]+a[13:16]+a[17:20]));assert len(vals)==13 and all(map(math.isfinite,vals));data[frame,engine]={'p':vals[:3],'q':vals[3:7],'v':vals[7:10],'w':vals[10:]}
 assert len(data)==6120
 frames=[]
 for i in range(220,233):
  rows=[]
  for engine in [0,1]:
   state=data[i,engine];prev=data[i-1,engine]
   rows.append({'engine':'cpu' if engine==0 else 'gpu','state':state,'minimum_corner_y':clearance(state['p'],state['q']),'translation_from_previous_frame':math.dist(state['p'],prev['p']),'linear_speed_times_dt':math.sqrt(sum(v*v for v in state['v']))/60})
  frames.append({'frame':i,'position_difference':math.dist(data[i,0]['p'],data[i,1]['p']),'velocity_difference':math.dist(data[i,0]['v'],data[i,1]['v']),'quaternion_lane_difference':math.sqrt(sum((a-b)**2 for a,b in zip(data[i,0]['q'],data[i,1]['q']))),'observations':rows})
 report[kind]={'raw_decompressed_sha256':hashlib.sha256(text.encode()).hexdigest(),'paired_body0_frames':3060,'window':frames}
result={'scope':'Derived diagnosis from nine-significant-digit printed poses; original executable limits/exits authoritative. Rotated unit-cube corner formula evaluated in host float64. Post-CCD recorded poses do not expose pre-CCD state/TOI fraction or complete cached state. No physical/release/performance acceptance.','observations':report,'source_difference':'CPU CCD triggers max(maxDeltaPosition,maxVelocity*dt)>0.5*minExtent with local vector rotation extents; GPU host CCD uses observed translation+quaternion_angle*scalar_max_extent>min(0.5*min_extent,SPECULATIVE_DISTANCE). Threshold/metric semantics differ.','next_discriminator':'Capture one earliest faithful impact with GPU pre/post host-CCD and CPU read-only finalize/CCD boundaries. Determine exact correction, classification and TOI independently before a candidate. Preserve old cutoff/no-CCD/paired-settling rejected controls. Do not blanket rollback or disable CCD.'}
(R/'analysis.json').write_text(json.dumps(result,indent=2)+'\n')
for k in report:
 for f in report[k]['window']:
  if f['frame'] in [226,227,228]:print(k,f['frame'],'position diff',f['position_difference'],'velocity diff',f['velocity_difference'],'corner y',[x['minimum_corner_y'] for x in f['observations']])
