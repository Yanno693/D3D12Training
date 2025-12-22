#ifdef USE_16BIT_INDEX_BUFFER
#   define Index uint16_t
#define RT_SHADER_SIGNATURE(name) void name##_16bit_hit(inout RTPayload payload, BuiltInTriangleIntersectionAttributes attribs)
#else
#   define Index uint
#define RT_SHADER_SIGNATURE(name) void name##_hit(inout RTPayload payload, BuiltInTriangleIntersectionAttributes attribs)
#endif // 16_BIT_INDEX

