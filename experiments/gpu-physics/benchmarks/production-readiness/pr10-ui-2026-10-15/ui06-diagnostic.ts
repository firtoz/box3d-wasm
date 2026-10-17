/** Read-only native adapter hooks; optional frozen-color calculation stops before append. */
import assert from 'node:assert/strict';
export function adapterDiagnostic(original:string,probe=false){
 let s=original;const declaration='extern void ui06_adapter_trace(const char*,const void*,b3ShapeId,b3BodyId,int,b3HexColor,float,float,float,bool,bool,bool);\n',fn='static void PopulateCommonFields( DebugShape* us, const b3DebugShape* debugShape )';assert.equal(s.split(fn).length-1,1);s=s.replace(fn,declaration+fn);const body='\tus->bodyId = bodyId;';assert.equal(s.split(body).length-1,1);s=s.replace(body,body+'\n\tui06_adapter_trace("create", us, us->shapeId, us->bodyId, us->bodyType, 0, 0.0f, 0.0f, 1.0f, false, false, false);');
 const draw='\tTransparentShadowCast shadowCast = TRANSPARENT_SHADOW_NONE;';assert.equal(s.split(draw).length-1,1);let hook='\tui06_adapter_trace("draw", us, us->shapeId, us->bodyId, us->bodyType, color, metallic, roughness, c.w, s_adapter.transparentDynamic, s_adapter.transparentKinematic, us->hasMaterial);\n\n';
 if(probe){const definition='static void DrawShape( void* userShape, b3WorldTransform shapeTransform, b3HexColor color, void* context )';assert.equal(s.split(definition).length-1,1);s=s.replace(definition,'static bool ui06_material_probe;\nstatic const void* ui06_probe_owner;\nextern bool ui06_probe_requested(const void*);\n'+definition);hook=`\tui06_adapter_trace(ui06_material_probe ? "default-material-probe" : "draw", ui06_material_probe ? ui06_probe_owner : us, us->shapeId, us->bodyId, us->bodyType, color, metallic, roughness, c.w, s_adapter.transparentDynamic, s_adapter.transparentKinematic, us->hasMaterial);
\t// The probe runs the actual native calculation on copied metadata and frozen
\t// packed color, then returns before any renderer append or geometry mutation.
\tif ( ui06_material_probe ) return;
\tif ( ui06_probe_requested(us) )
\t{
\t\tDebugShape copied = *us;
\t\tui06_probe_owner = us;
\t\tui06_material_probe = true;
\t\tDrawShape(&copied, shapeTransform, (b3HexColor)((uint32_t)color & 0x00FFFFFFu), context);
\t\tui06_material_probe = false;
\t\tui06_probe_owner = NULL;
\t}

`;}
 return s.replace(draw,hook+draw);
}
export const probeHelper=`
extern "C" bool ui06_probe_requested(const void* user) {
 if(!std::getenv("GPU_PR10_DEFAULT_MATERIAL_PROBE"))return false;
 const unsigned request=adapter_request();static unsigned last=~0u;static std::set<std::string> seen;
 if(request!=last){seen.clear();last=request;}char key[96];std::snprintf(key,sizeof(key),"%p:%d",user,adapter_cpu_draw);
 return seen.size()<128&&seen.insert(key).second;
}
`;
