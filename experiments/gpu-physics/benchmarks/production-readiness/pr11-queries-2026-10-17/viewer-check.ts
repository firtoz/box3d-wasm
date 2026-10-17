import type {Driver as DriverType} from "../pr10-ui-2026-10-15/native-ui-driver";
import {selectSample} from "../pr10-ui-2026-10-15/native-ui-cases";
import {writeFile} from "node:fs/promises";
import {resolve} from "node:path";
import assert from "node:assert/strict";
const near=(a:number[],b:number[])=>a.length===b.length&&a.every((v,i)=>Math.abs(v-b[i])<=1e-5);
const plus=(a:number[],b:number[],scale=1)=>a.map((v,i)=>v+scale*b[i]);
const label=process.argv[2];assert(["ordinary-both","native-both"].includes(label));
const generatedDriver=resolve(import.meta.dir,"raw/native-ui-driver.ts");
if(!await Bun.file(generatedDriver).exists())throw Error("Run experiments/gpu-physics/scripts/pr11-viewer-build.ts to prepare the temporary diagnostic driver first.");
const {Driver}=await import(generatedDriver) as {Driver:typeof DriverType};
const d=new Driver(label,{budgetFile:"viewer-budget.json",buildFile:"viewer-build.json",runDirectory:"raw/viewer-runs-qualified-ui"});
d.emit=(s:string)=>{if(!s.startsWith("PASS "))console.log(label,s);};
async function frame(){const q=await d.frames();return await d.until(()=>{const a=d.latest("pr11_frame");return a?.request>=q.request?a:null;});}
function project(a:any,p:number[]){
 const mul=(m:number[],v:number[])=>[0,1,2,3].map(i=>v.reduce((n,x,j)=>n+m[4*j+i]*x,0));
 const relative=plus(p,a.origin,-1);const clip=mul(a.proj,mul(a.view,[...relative,1]));
 if(clip[3]<=0)return null;
 return [(1+clip[0]/clip[3])*a.viewport[0]/2,(1-clip[1]/clip[3])*a.viewport[1]/2];
}
function markers(a:any,scale:number){
 const hits=a.queries.filter((q:any)=>q.hit);
 const points=a.draws.filter((p:any)=>p.kind==="point"&&hits.some((q:any)=>near(p.a,q.point)));
 d.check("Query hit marker has actual returned point",points.length>0,{a});
 d.check("Normal segment uses actual returned normal",hits.some((q:any)=>a.draws.some((l:any)=>l.kind==="line"&&near(l.a,q.point)&&near(l.b,plus(q.point,q.normal,scale)))),{a});
 const screens=points.map((p:any)=>({point:p.a,pixel:project(a,p.a)}));
 d.check("Camera projects returned hit markers into viewport",screens.some((s:any)=>s.pixel&&s.pixel[0]>=0&&s.pixel[0]<=a.viewport[0]&&s.pixel[1]>=0&&s.pixel[1]<=a.viewport[1]),{screens});
 return screens;
}
try {
 await d.run(async()=>{
   await selectSample(d,"Collision","Initial Overlap",1);
   const overlap=await frame();const overlapScreens=markers(overlap,.5);
   d.check("Initial overlap returns zero fraction and normal",overlap.queries.some((q:any)=>q.hit&&q.fraction===0&&near(q.normal,[0,0,0])));
   await d.screenshot("initial-overlap-after");
   await selectSample(d,"Collision","Shape Cast",12);
   const casts=await frame();const castScreens=markers(casts,.2);
   await d.screenshot("shape-cast-after");
   await selectSample(d,"Bodies","Cast",1);
   const body=await frame();
   d.check("Body Cast original default is a query miss",body.queries.some((q:any)=>q.kind==="body-shape"&&!q.hit),{body});
   d.check("Body Cast miss submits the white end sphere",body.draws.some((x:any)=>x.kind==="sphere"&&near(x.a,[-6.5,2.5,.5])&&near(x.color,[1,1,1,1])),{body});
   d.check("Body Cast displays its actual ray endpoints",body.draws.some((x:any)=>x.kind==="line"&&near(x.a,[-9.75,3,-4])&&near(x.b,[-9.75,3,4])),{body});
   d.check("Scene change removes old world-query markers",!body.queries.some((q:any)=>q.kind.startsWith("world-")),{body});
   await d.screenshot("body-cast-default");
   await d.hover(470,330);await d.down("Shift_L");await d.x("mousedown",1);d.held.add("mouse1");await d.frames();await d.x("mousemove","--window",d.wid,518,354);await d.frames();await d.x("mouseup",1);d.held.delete("mouse1");await d.up("Shift_L");
   const shifted=await frame();const result=shifted.queries.find((q:any)=>q.kind==="body-shape");d.check("Shift-drag actual sphere colour follows returned hit/miss",result&&shifted.draws.some((x:any)=>x.kind==="sphere"&&(result.hit?x.color[0]===0&&x.color[1]>0&&x.color[2]===0:near(x.color,[1,1,1,1]))),{shifted});
   await d.screenshot("body-cast-shift");
   d.r.pr11={overlap,overlapScreens,casts,castScreens,body,shifted};d.save();
 });
} catch(error) {d.r.status="FAILURE_RETAINED";d.r.error=String(error);d.save();throw error;}
finally {await d.cleanup();await writeFile(resolve(import.meta.dir,`${label}-visual-result.json`),JSON.stringify(d.r,null,2)+"\n");}
