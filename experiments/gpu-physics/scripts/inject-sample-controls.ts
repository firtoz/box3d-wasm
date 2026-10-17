/** Generate native sample UI repairs without modifying upstream Box3D. */
import {readFileSync,writeFileSync} from 'node:fs';
import assert from 'node:assert/strict';

export function manifoldControls(text:string):string{
  const old='\tbool DrawControls() override\n\t{\n\t\tImGui::Checkbox( "Use cache", &m_useCache );\n\t\tif ( m_useCache )';
  const replacement='\tvirtual bool SupportsCacheControls() const { return false; }\n\tvirtual bool SupportsSATControls() const { return false; }\n\n\tbool DrawControls() override\n\t{\n\t\tif ( SupportsCacheControls() )\n\t\t{\n\t\t\tImGui::Checkbox( "Use cache", &m_useCache );\n\t\t}\n\t\telse\n\t\t{\n\t\t\tImGui::TextDisabled( "Cache unavailable for this pair" );\n\t\t}\n\t\tif ( !SupportsSATControls() )\n\t\t{\n\t\t\tImGui::TextDisabled( "SAT controls unavailable for this pair" );\n\t\t}\n\t\telse if ( m_useCache )';
  assert.equal(text.split(old).length-1,2,'Upstream Manifold controls changed');
  text=text.replaceAll(old,replacement);
  for(const [name,base,sat] of [['HullAndSphere','Manifold',false],['CapsuleAndHull','Manifold',false],['TriangleAndCapsule','TriangleManifold',false],['HullAndHull','Manifold',true],['TriangleAndHull','TriangleManifold',true]] as const){
    const anchor=`class ${name} : public ${base}\n{\npublic:\n`;
    assert.equal(text.split(anchor).length-1,1,`Upstream ${name} changed`);
    text=text.replace(anchor,anchor+'\tbool SupportsCacheControls() const override { return true; }\n'+(sat?'\tbool SupportsSATControls() const override { return true; }\n':'')+'\n');
  }
  const auto="\t\t\tImGui::RadioButton( \"auto\", &m_manualFeature, 0 );";
  const reset="\t\t\tif ( ImGui::RadioButton( \"auto\", &m_manualFeature, 0 ) )\n\t\t\t{\n\t\t\t\t// Drop a forced SAT feature once; retain automatic cache on later frames.\n\t\t\t\tm_satCache = {};\n\t\t\t}";
  assert.equal(text.split(auto).length-1,2,'Upstream SAT auto controls changed');
  return text.replaceAll(auto,reset);
}

export function overlayControls(text:string):string{
  for(const [label,flag] of [['Sleep','drawSleep'],['Graph Colors','drawGraphColors'],['Islands','drawIslands']]){
    const old=`\t\t\tImGui::MenuItem( "${label}", nullptr, &gd->${flag} );`;
    assert.equal(text.split(old).length-1,1,'Upstream overlay control changed: '+label);
    text=text.replace(old,[
      '#if defined(BOTH_SAMPLES)',
      `\t\t\tImGui::MenuItem( "${label} (CPU pane only)", nullptr, &gd->${flag} );`,
      '#elif defined(GPU_PHYSICS_SAMPLES)',
      `\t\t\tImGui::MenuItem( "${label} (GPU unavailable)", nullptr, false, false );`,
      '#else', old, '#endif',
    ].join('\n'));
  }
  return text;
}

export function collisionControls(text:string):string{
  text=meshScalePreview(text);
  text=castWorldControls(text);
  const start=text.indexOf('class TimeOfImpact : public Sample\n');
  assert(start>=0,'Upstream TimeOfImpact class changed');
  const old='const char* shapeTypes[] = { "point", "segment", "triangle", "box" };';
  const tail=text.slice(start);
  assert.equal(tail.split(old).length-1,1,'Upstream TOI shape menu changed');
  const distanceStart=text.indexOf('class ShapeDistance : public Sample\n');
  assert(distanceStart>=0&&distanceStart<start,'Upstream ShapeDistance changed');
  let distance=text.slice(distanceStart,start);
  assert.equal(distance.split("\t\t\tm_typeA = ShapeType( shapeType );\n\t\t\tm_proxyA = MakeProxy( m_typeA, m_radiusA );").length-1,1,'Upstream radius A transition changed');
  distance=distance.replace("\t\t\tm_typeA = ShapeType( shapeType );\n\t\t\tm_proxyA = MakeProxy( m_typeA, m_radiusA );","\t\t\tm_typeA = ShapeType( shapeType );\n\t\t\t// Triangle/box previews have no radius control or rounded geometry.\n\t\t\tif ( m_typeA != e_point && m_typeA != e_segment )\n\t\t\t{\n\t\t\t\tm_radiusA = 0.0f;\n\t\t\t}\n\t\t\tm_proxyA = MakeProxy( m_typeA, m_radiusA );");
  assert.equal(distance.split("\t\t\tm_typeB = ShapeType( shapeType );\n\t\t\tm_proxyB = MakeProxy( m_typeB, m_radiusB );").length-1,1,'Upstream radius B transition changed');
  distance=distance.replace("\t\t\tm_typeB = ShapeType( shapeType );\n\t\t\tm_proxyB = MakeProxy( m_typeB, m_radiusB );","\t\t\tm_typeB = ShapeType( shapeType );\n\t\t\t// Triangle/box previews have no radius control or rounded geometry.\n\t\t\tif ( m_typeB != e_point && m_typeB != e_segment )\n\t\t\t{\n\t\t\t\tm_radiusB = 0.0f;\n\t\t\t}\n\t\t\tm_proxyB = MakeProxy( m_typeB, m_radiusB );");
  assert.equal(distance.split("\t\t\tm_proxyA = MakeProxy( m_typeA, m_radiusA );").length-1,1,'Upstream cache A proxy change changed');
  distance=distance.replace("\t\t\tm_proxyA = MakeProxy( m_typeA, m_radiusA );","\t\t\t// Cached support indices belong to the previous vertex set.\n\t\t\tm_cache = {};\n\t\t\tm_proxyA = MakeProxy( m_typeA, m_radiusA );");
  assert.equal(distance.split("\t\t\tm_proxyB = MakeProxy( m_typeB, m_radiusB );").length-1,1,'Upstream cache B proxy change changed');
  distance=distance.replace("\t\t\tm_proxyB = MakeProxy( m_typeB, m_radiusB );","\t\t\t// Cached support indices belong to the previous vertex set.\n\t\t\tm_cache = {};\n\t\t\tm_proxyB = MakeProxy( m_typeB, m_radiusB );");
  return text.slice(0,distanceStart)+distance+tail.replace(old,'const char* shapeTypes[] = { "box", "capsule", "triangle" };');
}

export function meshScalePreview(text:string):string{
  const start=text.indexOf('class MeshScale : public Sample\n');
  const end=text.indexOf('class ShapeCast : public Sample\n',start);
  assert(start>=0&&end>start,'Upstream MeshScale class changed');
  let mesh=text.slice(start,end);
  for(const displacement of['context.fractions[0] * rayTranslation','rayTranslation']){
    const old='b3WorldTransform transform = b3MakeWorldTransform( { '+displacement+', b3Quat_identity } );';
    const replacement='b3WorldTransform transform = { b3OffsetPos( m_start, '+displacement+' ), b3Quat_identity };';
    assert.equal(mesh.split(old).length-1,1,'Upstream MeshScale preview transform changed');
    mesh=mesh.replace(old,replacement);
  }
  return text.slice(0,start)+mesh+text.slice(end);
}

export function castWorldControls(text:string):string{
  const start=text.indexOf('class CastWorld : public Sample\n');
  const end=text.indexOf('class MeshScale : public Sample\n',start);
  assert(start>=0&&end>start,'Upstream CastWorld class changed');
  let cast=text.slice(start,end);
  const old='if ( m_castType != e_rayCast && m_castType != e_boxCast )';
  const slider='ImGui::SliderFloat( "Radius", &m_castRadius, 0.1f, 2.0f, "%.1f" );';
  assert.equal(cast.split(old).length-1,1,'Upstream CastWorld size condition changed');
  assert.equal(cast.split(slider).length-1,1,'Upstream CastWorld size slider changed');
  cast=cast.replace(old,'if ( m_castType != e_rayCast )');
  cast=cast.replace(slider,'ImGui::SliderFloat( m_castType == e_boxCast ? "Half-width" : "Radius", &m_castRadius, 0.1f, 2.0f, "%.1f" );');
  return text.slice(0,start)+cast+text.slice(end);
}

export function humanControls(text:string):string{
  const start=text.indexOf('void DestroyHuman( Human* human )\n');
  const end=text.indexOf('void Human_SetVelocity(',start);
  assert(start>=0&&end>start,'Upstream Human lifetime changed');
  const anchor='\tfor ( int i = 0; i < bone_count; ++i )\n\t{\n\t\tif ( B3_IS_NULL( human->bones[i].bodyId ) )';
  let destroy=text.slice(start,end);
  assert.equal(destroy.split(anchor).length-1,1,'Upstream Human body cleanup changed');
  const cleanup=[
    '\t// Anchor bodies and joints belong to this human too.',
    '\tfor ( int i = 0; i < bone_count; ++i )',
    '\t{',
    '\t\tBone* bone = human->bones + i;',
    '\t\tif ( B3_IS_NON_NULL( bone->anchorJointId ) )',
    '\t\t{',
    '\t\t\tb3DestroyJoint( bone->anchorJointId, false );',
    '\t\t\tbone->anchorJointId = b3_nullJointId;',
    '\t\t}',
    '\t\tif ( B3_IS_NON_NULL( bone->anchorId ) )',
    '\t\t{',
    '\t\t\tb3DestroyBody( bone->anchorId );',
    '\t\t\tbone->anchorId = b3_nullBodyId;',
    '\t\t}',
    '\t}',
    '',
  ].join('\n');
  destroy=destroy.replace(anchor,cleanup+anchor);
  return text.slice(0,start)+destroy+text.slice(end);
}

export function jointControls(text:string):string{
  const start=text.indexOf('class Door : public Sample\n'),end=text.indexOf('static int sampleDoor =',start);
  assert(start>=0&&end>start,'Upstream Door class changed');
  let door=text.slice(start,end);
  const init='\t\tm_twoJoints = true;';
  assert.equal(door.split(init).length-1,1,'Upstream Door constructor changed');
  door=door.replace(init,init+'\n\t\tm_enableLimit = true;\n\t\tm_jointId1 = b3_nullJointId;\n\t\tm_jointId2 = b3_nullJointId;');
  const policy='jointDef.enableLimit = true;';
  assert.equal(door.split(policy).length-1,2,'Upstream Door hinge definitions changed');
  door=door.replaceAll(policy,'jointDef.enableLimit = m_enableLimit;');
  return text.slice(0,start)+door+text.slice(end);
}

export function treeControls(text:string):string{
  const start=text.indexOf('\tvoid LoadTree()\n\t{\n');
  const end=text.indexOf('\n\t\tint maxArea = 0.0f;',start);
  assert(start>=0&&end>start,'Upstream Tree load changed');
  const original=text.slice(start,end);
  assert(original.includes('b3DynamicTree_Destroy( &m_tree );')&&original.includes('m_tree = b3DynamicTree_Load( m_saveFileName, m_loadScale );'),'Upstream Tree load ownership changed');
  const replacement=[
    '\tvoid LoadTree()', '\t{',
    '\t\t// Keep the live tree and its proxy metadata until the load succeeds.',
    '\t\tb3DynamicTree candidate = b3DynamicTree_Load( m_saveFileName, m_loadScale );',
    '\t\tif ( candidate.nodes == nullptr || candidate.proxyCount == 0 )', '\t\t{',
    '\t\t\tif ( candidate.nodes != nullptr )', '\t\t\t{',
    '\t\t\t\tb3DynamicTree_Destroy( &candidate );', '\t\t\t}',
    '\t\t\tm_loadFailed = true;',
    '\t\t\tfprintf( stderr, "Failed to load tree \'%s\'\\n", m_saveFileName );',
    '\t\t\treturn;', '\t\t}', '',
    '\t\tif ( m_tree.nodeCapacity > 0 )', '\t\t{',
    '\t\t\tb3DynamicTree_Destroy( &m_tree );', '\t\t}',
    '\t\tm_tree = candidate;', '\t\tm_proxies.clear();',
    '\t\tm_loadFailed = false;',
  ].join('\n');
  text=text.slice(0,start)+replacement+text.slice(end);
  const create='\tvoid CreateTree()\n\t{';
  assert.equal(text.split(create).length-1,1,'Upstream Tree create changed');
  text=text.replace(create,create+'\n\t\tm_loadFailed = false;');
  const controls='\tbool DrawControls() override\n\t{';
  assert.equal(text.split(controls).length-1,1,'Upstream Tree controls changed');
  text=text.replace(controls,controls+'\n\t\tif ( m_loadFailed )\n\t\t{\n\t\t\tImGui::TextWrapped( "Failed to load tree \'%s\'. The current tree is unchanged. Choose File to rebuild, or Save before retrying Load.", m_saveFileName );\n\t\t}\n');
  const member='\tbool m_doRay;';
  assert.equal(text.split(member).length-1,1,'Upstream Tree members changed');
  return text.replace(member,member+'\n\tbool m_loadFailed = false;');
}

if(import.meta.main){
  const [kind,src,dst]=process.argv.slice(2);
  assert(kind==='manifold'||kind==='collision'||kind==='human'||kind==='joint'||kind==='tree'||kind==='overlays');assert(src&&dst,'Usage: bun inject-sample-controls.ts manifold|collision|human|joint|tree|overlays source destination');
  const result=({manifold:manifoldControls,collision:collisionControls,human:humanControls,joint:jointControls,tree:treeControls,overlays:overlayControls}[kind])(readFileSync(src,'utf8'));
  writeFileSync(dst,result);
}
