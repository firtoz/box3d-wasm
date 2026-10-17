import {D,read,write,sha,size,verifyBuild} from './review-closeout';
import {readFileSync,writeFileSync,renameSync,existsSync,statfsSync} from 'node:fs';
import {join,resolve} from 'node:path';
import {createHash} from 'node:crypto';
import assert from 'node:assert/strict';
const budget={status:'FROZEN',seconds:35,files:308,bytes_read:536870912,max_temporary_file_bytes:33554432,output_bytes:524288,builds:0,apps:0,captures:0,deletions:0,scope:'Atomic canonical JSON whitespace compaction of listed terminal task receipts only. Exact original bytes reconstructable. Images, sources, current build inputs and shared caches unchanged.'};
write('storage-receipt-compaction-apply-budget.json',budget);
const p=read('storage-receipt-compaction-plan.json'),b=verifyBuild('ui23-build.json');
const protectedSet=new Set([...Object.keys(b.input_pins),...Object.keys(b.include_pins)].map(p=>resolve(p)));
const hash=(s:string)=>createHash('sha256').update(s).digest('hex'),start=performance.now(),done:any[]=[];
assert.equal(p.files.length,308);
for(const f of p.files){
 assert(performance.now()-start<35000);assert(f.path.startsWith(join(D,'raw')+'/')&&f.path.endsWith('/receipt.json')&&!protectedSet.has(resolve(f.path)));
 const original=readFileSync(f.path,'utf8');assert.equal(hash(original),f.before_sha256);
 const r=JSON.parse(original);assert.equal(r.language,'TypeScript/Bun');assert(r.app_terminal_exit!==undefined||r.app_terminal_signal!==undefined);
 for(const pid of[r.app_pid,r.driver_pid])assert(!pid||!existsSync('/proc/'+pid));
 const compact=JSON.stringify(r)+'\n';assert.equal(hash(compact),f.after_sha256);assert.equal(JSON.stringify(JSON.parse(compact),null,2)+'\n',original);
 const temp=f.path+'.compact-tmp';assert(!existsSync(temp));writeFileSync(temp,compact,{flag:'wx'});renameSync(temp,f.path);assert.equal(sha(f.path),f.after_sha256);
 done.push(f);
}
verifyBuild('ui23-build.json');const fs=statfsSync(D),free=Number(fs.bavail)*Number(fs.bsize);assert(free>20*1024**3);
const result={status:'PASS',budget:'storage-receipt-compaction-apply-budget.json',plan:'storage-receipt-compaction-plan.json',seconds:(performance.now()-start)/1000,files:done.length,saved_bytes:done.reduce((n,f)=>n+f.before_bytes-f.after_bytes,0),raw_bytes_after:size(join(D,'raw')),free_bytes:free,current_pool_verified:'ui23-build.json',restoration:p.restoration,original_hashes_and_sizes:'storage-receipt-compaction-plan.json/files',original_receipts_reconstructable_byte_exact:true,mutated_images_sources_inputs_shared_caches:false};
write('storage-receipt-compaction-apply.json',result);console.log(result);
