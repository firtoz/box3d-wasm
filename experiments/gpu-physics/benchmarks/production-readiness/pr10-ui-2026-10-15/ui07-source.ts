export const oldAspect='\t\t\t\tfloat aspect = cam.m_height > 0 ? (float)cam.m_width / (float)cam.m_height : 1.0f;';
export const newAspect=`#if defined(BOTH_SAMPLES)
                // DrawUI uses window width for layout; frame the actual viewport.
                const int frameWidth = both_split_enabled() && sapp_width() >= 2 ? sapp_width() / 2 : cam.m_width;
                float aspect = cam.m_height > 0 ? (float)frameWidth / (float)cam.m_height : 1.0f;
#else
${oldAspect}
#endif`;
