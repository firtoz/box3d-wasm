export const oldRefresh='\tDebugShape* us = (DebugShape*)userShape;\n\n\tRefreshMaterialFromOverride( us );';
export const newRefresh='\tDebugShape* us = (DebugShape*)userShape;\n\n\t// Type mutations preserve body identity but change transparency and defaults.\n\tus->bodyType = b3Body_GetType( us->bodyId );\n\tRefreshMaterialFromOverride( us );';
export const oldBodyGetter='B3_API b3BodyType b3Body_GetType(b3BodyId bodyId)\n{\n\treturn (b3BodyType)gpu_b3_body_get_type(bodyId);\n}';
export const newBodyGetter='B3_API b3BodyType b3Body_GetType(b3BodyId bodyId)\n{\n\t// CPU debug callbacks carry raw CPU IDs, as in the shape-name lookup.\n\tif (g_cpu_debug_draw)\n\t{\n\t\treturn cpu_b3Body_GetType(bodyId);\n\t}\n\treturn (b3BodyType)gpu_b3_body_get_type(bodyId);\n}';
export const oldShapeGetter='B3_API b3BodyId b3Shape_GetBody(b3ShapeId shape) { return gpu_b3_shape_get_body(shape); }';
export const newShapeGetter='B3_API b3BodyId b3Shape_GetBody(b3ShapeId shape)\n{\n\tif (g_cpu_debug_draw)\n\t{\n\t\treturn cpu_b3Shape_GetBody(shape);\n\t}\n\treturn gpu_b3_shape_get_body(shape);\n}';
