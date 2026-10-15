"""Replay direct Step drawing submissions without invoking simulation twice."""

def once(text, old, new):
    if text.count(old) != 1:
        raise RuntimeError(f'split Step hook drift: {old!r}')
    return text.replace(old, new)

def main_hooks(text):
    text = once(text, '\ts_context.sample->Step();', '''#if defined(BOTH_SAMPLES)
    split_step_begin(splitActive);
#endif
\ts_context.sample->Step();
#if defined(BOTH_SAMPLES)
    split_step_end();
#endif''')
    text = once(text, '        split_stream_view(1);\n        both_draw_second();',
                     '        split_stream_view(1);\n        split_step_replay();\n        both_draw_second();')
    text = once(text, '    DrawUI( &s_context );', '''#if defined(BOTH_SAMPLES)
    split_step_ui_begin(both_split_enabled() ? paneWidth : 0);
#endif
    DrawUI( &s_context );
#if defined(BOTH_SAMPLES)
    split_step_ui_end();
#endif''')
    text = once(text, '\tShutdownRenderer();', '''#if defined(BOTH_SAMPLES)
    split_step_shutdown();
#endif
\tShutdownRenderer();''')
    return text

def world_hooks(text):
    return once(text, '\tb3World_Draw( m_worldId, &debugDraw, B3_DEFAULT_MASK_BITS );', '''#if defined(BOTH_SAMPLES)
    split_step_world_begin();
#endif
\tb3World_Draw( m_worldId, &debugDraw, B3_DEFAULT_MASK_BITS );
#if defined(BOTH_SAMPLES)
    split_step_world_end();
#endif''')

# Each wrapper records only an accepted append. Replaying calls the same packing
# operation with copied arguments; transient source strings are owned by the journal.
def wrapper(text, fn, fields, before, after, extra=''):
    params = ', '.join(f'{typ} {name}' for typ, name in fields)
    args = ', '.join(name for _, name in fields)
    record = 'SplitArgs' + fn
    renamed = 'SplitRaw' + fn
    text = once(text, 'void ' + fn + '(', 'static void ' + renamed + '(')
    definitions = '\n'.join(f'    {typ} {name};' for typ, name in fields)
    assignments = '\n'.join(f'    data.{name} = {name};' for _, name in fields)
    copied = ', '.join('data.' + name for _, name in fields)
    return text + f'''
typedef struct {record} {{
{definitions}
}} {record};
static void SplitReplay{fn}(const void* bytes) {{
    {record} data;
    memcpy(&data, bytes, sizeof(data));
    {renamed}({copied});
}}
void {fn}({params}) {{
    size_t previous = {before};
    {renamed}({args});
    if ({after} > previous) {{
        {record} data = {{0}};
{assignments}
        split_step_record(SplitReplay{fn}, &data, sizeof(data));
    }}
{extra}
}}
'''

def gfx_hooks(text, name):
    if name not in ('renderer.c', 'overlay.c', 'text.c'):
        return text
    text = '#include "sokol_split_step.h"\n#include <string.h>\n' + text
    material = [('Vec4','baseColor'),('float','metallic'),('float','roughness'),('TransparentShadowCast','shadowCast')]
    if name == 'renderer.c':
        for fn, fields, shape in [
            ('AppendCube',[('b3Transform','transform'),('b3Vec3','scale')]+material,'cube'),
            ('AppendSphere',[('b3Transform','transform'),('float','radius')]+material,'sphere'),
            ('AppendCapsule',[('b3Transform','transform'),('float','halfLength'),('float','radius')]+material,'capsule')]:
            expression=f's_gfx.{shape}Count + s_gfx.{shape}CountXp'
            text=wrapper(text,fn,fields,expression,expression)
    elif name == 'overlay.c':
        fields=[('b3Vec3','a'),('b3Vec3','b'),('Vec4','linearColor'),('float','thickness'),('OverlayThicknessUnit','thicknessUnit'),('OverlayOcclusionMode','occlusionMode')]
        text=wrapper(text,'OverlayAppendLine',fields,'s_overlay.lineCount','s_overlay.lineCount')
        fields=[('b3Vec3','p'),('Vec4','linearColor'),('float','size'),('OverlayThicknessUnit','sizeUnit'),('OverlayOcclusionMode','occlusionMode')]
        text=wrapper(text,'OverlayAppendPoint',fields,'s_overlay.pointCount','s_overlay.pointCount')
    else:
        # Slot storage already truncates to63characters. Copy those accepted bytes,
        # not a source pointer that can expire or be overwritten before pane two.
        for fn, fields, args in [
            ('DrawString','b3Vec3 worldPos, Vec4 color, const char* text','data.entry.worldPos, data.entry.color, data.text'),
            ('DrawScreenString','int x, int y, Vec4 color, const char* text','data.entry.screenX, data.entry.screenY, data.entry.color, data.text')]:
            raw='SplitRaw'+fn
            text=once(text,'void '+fn+'(','static void '+raw+'(')
            original_args='worldPos, color, text' if fn=='DrawString' else 'x, y, color, text'
            mirror=''
            if fn=='DrawScreenString':
                mirror='''    int width = split_step_ui_width();
    if (width > 0 && x >= 0 && x < width && width <= INT_MAX - x)
        SplitRawDrawScreenString(x + width, y, color, text);'''
            text+=f'''
typedef struct SplitArgs{fn} {{ TextEntry entry; char text[TEXT_BUFFER_SIZE]; }} SplitArgs{fn};
static void SplitReplay{fn}(const void* bytes) {{
    SplitArgs{fn} data;
    memcpy(&data, bytes, sizeof(data));
    {raw}({args});
}}
void {fn}({fields}) {{
    int previous = s_count;
    {raw}({original_args});
    if (s_count > previous) {{
        SplitArgs{fn} data = {{0}};
        data.entry = s_entries[previous];
        data.entry.text = NULL;
        memcpy(data.text, s_entries[previous].text, TEXT_BUFFER_SIZE);
        split_step_record(SplitReplay{fn}, &data, sizeof(data));
    }}
{mirror}
}}
'''
        text='#include <limits.h>\n'+text
    return text
