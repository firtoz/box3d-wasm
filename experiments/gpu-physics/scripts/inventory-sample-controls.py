#!/usr/bin/env python3
"""Source inventory for PR10. Source presence never qualifies an interaction."""
import argparse, ast, hashlib, json, re
from pathlib import Path

def mask_cpp(text):
    # Preserve offsets/newlines while excluding braces in comments and literals.
    pattern = r'//[^\n]*|/\*[\s\S]*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\''
    return re.sub(pattern, lambda m: ''.join('\n' if c=='\n' else ' ' for c in m[0]), text)

def closing(mask, start, left='{', right='}'):
    depth=0
    for i in range(start,len(mask)):
        if mask[i]==left:depth+=1
        elif mask[i]==right:
            depth-=1
            if depth==0:return i
    raise ValueError('Unclosed source block')

WIDGET=re.compile(r'ImGui::(Button|SmallButton|ArrowButton|InvisibleButton|ImageButton|Checkbox|CheckboxFlags|Combo|BeginCombo|RadioButton|Slider\w*|VSlider\w*|Drag\w*|Input\w*|Selectable|MenuItem|BeginMenu|BeginTabItem|CollapsingHeader|TreeNode\w*|Shortcut|ColorEdit\w*|ColorPicker\w*)\s*\(')

def widgets(text, start=0,end=None):
    end=len(text) if end is None else end;mask=mask_cpp(text);rows=[]
    for m in WIDGET.finditer(text,start,end):
        if mask[m.start():m.start()+6]!='ImGui:':continue
        a=m.end()-1;b=closing(mask,a,'(',')');args=text[a+1:b].strip()
        label=re.match(r'"((?:\\.|[^"\\])*)"',args)
        rows.append({'kind':m[1],'label':label[1] if label else None,'arguments':args[:360],
                     'line':text.count('\n',0,m.start())+1,'status':'UNREVIEWED','ordinary_gpu':'UNREVIEWED','native_gpu':'UNREVIEWED','ordinary_combined':'UNREVIEWED','native_combined':'UNREVIEWED','real_cpu':'UNREVIEWED'})
    return rows

def classes(text):
    mask=mask_cpp(text);out={}
    for m in re.finditer(r'\bclass\s+(\w+)\s*(?:final\s*)?:\s*public\s+(\w+)[^{;]*\{',mask):
        a=m.end()-1;b=closing(mask,a);out[m[1]]=(a,b,m[2])
    return out

def main():
    p=argparse.ArgumentParser();p.add_argument('--root',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args();root=a.root.resolve()
    base=root/'experiments/gpu-physics';old=json.loads((base/'benchmarks/production-readiness/pr09-verified-2026-10-15/data/inventory.json').read_text())
    sources={n['source'] for n in old['native']};texts={s:(root/s).read_text()for s in sources};blocks={s:classes(t)for s,t in texts.items()}
    rows=[];missing=[]
    for index,n in enumerate(old['native'],1):
        cls=n['class'].split('<')[0];s=n['source'];t=texts[s];chain=[];seen=set();owner=cls;controls=[];keys=set();methods=[]
        while owner!='Sample' and owner not in seen:
            seen.add(owner)
            found=[(src,bs[owner])for src,bs in blocks.items()if owner in bs]
            if len(found)!=1:missing.append({'sample':n['cpu_identity'],'class':owner,'matches':len(found)});break
            src,(lo,hi,parent)=found[0];body=texts[src][lo:hi];chain.append({'class':owner,'parent':parent,'source':src,'line':texts[src].count('\n',0,lo)+1})
            controls.extend(dict(c,source=src,declaring_class=owner)for c in widgets(texts[src],lo,hi))
            keys.update(re.findall(r'\b(?:KEY_[A-Z0-9_]+|SAPP_KEYCODE_[A-Z0-9_]+|MOD_[A-Z]+)\b',mask_cpp(body)))
            methods.extend(sorted(set(re.findall(r'\b(DrawControls|Keyboard|MouseDown|MouseUp|MouseMove|HasSolverControls|InfoPanelWidthEm|FocusBounds|FocusBody|FocusHome)\s*\(',mask_cpp(body)))))
            owner=parent
        rows.append({'id':index,'cpu_identity':n['cpu_identity'],'class':n['class'],'source':s,'registration_line':n['registration_line'],'inheritance':chain,'widgets':controls,'declared_input_methods':sorted(set(methods)),'key_tokens':sorted(keys),'shared_controls':'See shared inventory; availability follows actual HasSolverControls/state/layout. No inheritance-based interaction pass.','status':'UNREVIEWED','checked_routes':[],'issues':[]})
    assert len(rows)==165 and not missing,missing
    common=['box3d/samples/sample.cpp','box3d/samples/main.cpp','box3d/samples/host/camera.cpp','experiments/gpu-physics/native-samples/sample_stats.cpp','experiments/gpu-physics/scripts/inject-sokol-bench.py','experiments/gpu-physics/scripts/sokol_split_injection.py']
    shared=[]
    for s in common:
        t=(root/s).read_text()
        if s.endswith('.cpp'):
            shared.extend(dict(c,source=s,declaring_class='shared host; conditional GPU policy requires review')for c in widgets(t))
        elif s.endswith('sokol_split_injection.py'):
            # The combined-only UI is emitted from Python string literals, so
            # scanning only the upstream C++ misses its comparison checkbox.
            seen_generated = set()
            for node in ast.walk(ast.parse(t)):
                if not isinstance(node, ast.Constant) or not isinstance(node.value, str):
                    continue
                for c in widgets(node.value):
                    identity = (c['kind'], c['label'], c['arguments'])
                    if identity in seen_generated:
                        continue
                    seen_generated.add(identity)
                    c.update(source=s, line=node.lineno+c['line']-1,
                             declaring_class='generated combined viewer; BOTH_SAMPLES',
                             generated_gpu_only=True, real_cpu='NOT_APPLICABLE',
                             ordinary_gpu='NOT_APPLICABLE', native_gpu='NOT_APPLICABLE')
                    shared.append(c)
        texts[s]=t
    result={'schema':1,'scope':'PR10 full native catalog source inventory. Lexical widget calls/keys plus inheritance; source conditionals, helpers, generated UI policies and actual effects require review. Counts are source call sites, not completed cases.','native_samples':rows,'shared_widget_calls':shared,'shared_input_sources':common,'source_sha256':{s:hashlib.sha256(t.encode()).hexdigest()for s,t in sorted(texts.items())},'counts':{'samples_mapped':165,'samples_inventory_left':0,'sample_widget_call_sites':sum(len(n['widgets'])for n in rows),'samples_with_widget_calls':sum(bool(n['widgets'])for n in rows),'shared_widget_call_sites':len(shared),'samples_with_input_overrides':sum(any(x in n['declared_input_methods']for x in ['Keyboard','MouseDown','MouseMove','MouseUp'])for n in rows),'interaction_samples_done':0,'interaction_samples_left':165},'limitations':['Labels may be dynamic; conditional paths and repeated runtime widgets are not inferred from lexical call count.','Browser runtime and nonvisual CPU oracles retain PR09 dispositions.','All actual interaction statuses start UNREVIEWED; source inventory never counts as runtime verification.']}
    result['counts'].update(
        cpu_shared_widget_call_sites=sum(not c.get('generated_gpu_only', False) for c in shared),
        gpu_only_shared_widget_call_sites=sum(bool(c.get('generated_gpu_only')) for c in shared))
    a.output.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(result['counts']))
if __name__=='__main__':main()
