import assert from 'node:assert/strict';
export function heightFieldEncoding(text:string):string{
 assert(!text.includes('height_field_compat.h'));
 text=text.replace('#include "box3d/box3d.h"','#include "box3d/box3d.h"\n#include "height_field_compat.h"');
 const pattern=/([\t ]+)(GpuShapeId (?:id|gpu) = gpu_b3_create_height_field\(|bool replaced = gpu_b3_replace_height_field\()/g;
 let count=0;
 text=text.replace(pattern,(all,indent,call)=>{count++;return indent+'const GpuHeightFieldEncoding encoding = gpu_height_field_encoding(heightField);\n'+indent+call;});
 const original='heightField->minHeight, heightField->heightScale, heightField->scale.x, heightField->scale.y,\n\t\theightField->scale.z, b3GetHeightFieldMaterialIndices(heightField)';
 assert.equal(text.split(original).length-1,count);
 assert(count===1||count===2);
 return text.replaceAll(original,'encoding.minHeight, encoding.heightScale, encoding.scale.x, encoding.scale.y,\n\t\tencoding.scale.z, b3GetHeightFieldMaterialIndices(heightField)');
}
