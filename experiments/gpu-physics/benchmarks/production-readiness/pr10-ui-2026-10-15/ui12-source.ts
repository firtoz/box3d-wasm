export const oldDrawHull='void DrawHull( b3WorldTransform transform, const b3HullData* hull, Vec4 color )\n{';
export const newDrawHull=oldDrawHull+'\n\t// Geometry tools may temporarily produce no solid hull while editing.\n\tif ( hull == NULL )\n\t{\n\t\treturn;\n\t}';
