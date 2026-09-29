#!/usr/bin/env python3
import argparse,gzip,json,struct,math
from pathlib import Path
parser=argparse.ArgumentParser(description="Inspect spherical soft-limit compliance from captured start transforms and cached impulses; diagnostic, not an acceptance gate.")
parser.add_argument('trace',type=Path)
parser.add_argument('--body-identity',type=int,required=True)
parser.add_argument('--frames',type=int,required=True)
parser.add_argument('--out',type=Path,required=True)
args=parser.parse_args()
if args.frames<1 or args.body_identity<1:parser.error('positive frame count and body identity required')
f=lambda x:struct.unpack('!f',bytes.fromhex(x))[0]
v=lambda x:[f(a) for a in x]
def add(a,b):return [x+y for x,y in zip(a,b)]
def scale(s,a):return [s*x for x in a]
def dot(a,b):return sum(x*y for x,y in zip(a,b))
def cross(a,b):return [a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]]
def norm(a):return math.sqrt(dot(a,a))
def unit(a):
 n=norm(a);return scale(1/n,a) if n>1e-12 else [0,0,0]
def mul(a,b):return add(add(scale(a[3],b[:3]),scale(b[3],a[:3])),cross(a[:3],b[:3]))+[a[3]*b[3]-dot(a[:3],b[:3])]
def inv(q):return [-q[0],-q[1],-q[2],q[3]]
def rotate(q,a):return add(a,scale(2,cross(q[:3],add(cross(q[:3],a),scale(q[3],a)))))
rows=[];count=0;first_frame=None
opener=gzip.open if args.trace.suffix=='.gz' else open
with opener(args.trace,'rt') as source:
 for line in source:
  count+=1;d=json.loads(line)
  if first_frame is None:first_frame=d['frame']
  if d.get('schema')!='rain-cell-diagnostic-excerpt-v1' or d.get('frame')!=first_frame+count-1:raise ValueError('schema or frame sequence mismatch')
  candidates=[j for j in d['joints'] if j['b']==args.body_identity and j['kind']==3]
  if not candidates:continue
  if len(candidates)!=1:raise ValueError('ambiguous spherical joint selection')
  j=candidates[0];identity_a=j['a'];identity_b=j['b']
  bodies={b['identity']:b for b in d['bodies']};order=sorted(int(k) for k in d['step_start_bodies']);starts=[d['step_start_bodies'][str(k)] for k in order];a,b=bodies[identity_a],bodies[identity_b]
  qa=mul(v(starts[order.index(identity_a)]['rot']),v(j['frame_a_rotation']));qb=mul(v(starts[order.index(identity_b)]['rot']),v(j['frame_b_rotation']))
  ca=rotate(qa,[0,0,1]);cb=rotate(qb,[0,0,1]);swing=unit(cross(ca,cb));relative=mul(inv(qa),qb);den=relative[2]**2+relative[3]**2;assert den>1e-10
  jac=add(ca,scale(math.sqrt((relative[0]**2+relative[1]**2)/den),cross(swing,ca)))
  dt=f(d['settings']['dt']);twist=f(j['lower_impulse'])-f(j['upper_impulse'])
  endA=mul(v(a['rot']),v(j['frame_a_rotation']));endB=mul(v(b['rot']),v(j['frame_b_rotation']));endrel=mul(inv(endA),endB);angle=2*math.atan2(endrel[2],endrel[3]);angle=(angle+math.pi)%(2*math.pi)-math.pi
  def apply_inertia(body,axis):
   q=v(starts[order.index(body['identity'])]['rot']);local=rotate(inv(q),axis);diag=v(body['inv_inertia']);off=v(body['gpu_motion']['inv_inertia_offdiag']);upper=v(body['gpu_motion']['inv_inertia_upper'])
   # Keep distinct upper/lower lanes, matching WGSL matrix columns.
   result=[diag[0]*local[0]+upper[0]*local[1]+upper[1]*local[2],off[0]*local[0]+diag[1]*local[1]+upper[2]*local[2],off[1]*local[0]+off[2]*local[1]+diag[2]*local[2]]
   return rotate(q,result)
  inverse_effective_mass=dot(jac,add(apply_inertia(a,jac),apply_inertia(b,jac)));assert inverse_effective_mass>0
  effective_mass=1/inverse_effective_mass;frequency=min(f(j['hertz']),.25/dt)
  stiffness=effective_mass*(2*math.pi*frequency)**2
  row={'effective_twist_mass':effective_mass,'soft_constraint_stiffness':stiffness,'static_softness_residual_estimate':twist/dt/stiffness,'endpoint_sleep_flags':[bool(a['flags']&4),bool(b['flags']&4)],'joint_identity':j['identity'],'a':identity_a,'b':identity_b,'frame':d['frame'],'lower_violation':max(0,f(j['lower_translation'])-angle),'twist_limit_impulse':twist,'prepared_twist_torque':scale(twist/dt,jac),'spring_torque':scale(1/dt,v(j['spring_angular_impulse'])),'motor_torque':scale(1/dt,v(j['motor_angular_impulse'])),'swing_torque':scale(f(j['swing_impulse'])/dt,swing),'point_impulse':v(j['angular_impulse']),'endpoint_angular_speeds':[norm(v(a['omega'])),norm(v(b['omega']))]}
  rows.append(row)
  swing_k=dot(swing,add(apply_inertia(a,swing),apply_inertia(b,swing)))
  swing_mass=1/swing_k if swing_k>0 else None
  swing_stiffness=swing_mass*(2*math.pi*frequency)**2 if swing_mass is not None else None
  end_swing_angle=2*math.atan2(math.hypot(endrel[0],endrel[1]),math.hypot(endrel[2],endrel[3]))
  end_swing_axis=unit(cross(rotate(endA,[0,0,1]),rotate(endB,[0,0,1])))
  row.update(effective_swing_mass=swing_mass,
             swing_soft_constraint_stiffness=swing_stiffness,
             swing_static_softness_residual_estimate=f(j['swing_impulse'])/dt/swing_stiffness if swing_stiffness else None,
             cone_violation_libm=max(0,end_swing_angle-f(j['target_translation'])),
             start_end_swing_axis_dot=dot(swing,end_swing_axis) if norm(swing)>0 and norm(end_swing_axis)>0 else None)
if count!=args.frames or not rows:raise ValueError(f'expected {args.frames} frames and a matching joint, got {count} frames and {len(rows)} observations')
args.out.write_text(json.dumps({'rows':rows,'scope':'float64 reconstruction from captured float32 start transforms and cached final impulses; these are cached impulse/h values, not a full per-substep torque balance; sleeping endpoints may retain impulses prepared before this step'},indent=2)+'\n')
print(json.dumps({'observations':len(rows),'last':rows[-1]},indent=2))
