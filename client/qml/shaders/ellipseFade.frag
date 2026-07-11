#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;

    // QML property: vector2d itemSize
    vec2 itemSize;

    // QML property: real fadeWidth
    float fadeWidth;

    // QML property: color tintColor
    vec4 tintColor;
};

layout(binding = 1) uniform sampler2D source;

void main()
{
    vec4 color = texture(source, qt_TexCoord0);

    vec2 halfSize = itemSize * 0.5;

    if (halfSize.x <= 0.0 || halfSize.y <= 0.0) {
        fragColor = vec4(0.0);
        return;
    }

    // Pixelkoordináta az item középpontjához képest
    vec2 p = (qt_TexCoord0 - vec2(0.5, 0.5)) * itemSize;

    float lenP = length(p);

    float ellipseAlpha = 1.0;

    if (lenP > 0.0001) {
        vec2 dir = p / lenP;

        // Az ellipszis peremének távolsága ebben az irányban
        float denom =
            (dir.x * dir.x) / (halfSize.x * halfSize.x) +
            (dir.y * dir.y) / (halfSize.y * halfSize.y);

        float edgeDistance = 1.0 / sqrt(denom);

        // Pozitív: az ellipszisen belül vagyunk.
        // 0: pontosan a peremen.
        // Negatív: az ellipszisen kívül.
        float insideDistance = edgeDistance - lenP;

        // A peremtől befelé fadeWidth pixel alatt 0 -> 1
        ellipseAlpha = smoothstep(0.0, fadeWidth, insideDistance);
    }

    // Eredeti alpha megtartása + ellipszis alpha + színező alpha + Qt opacity
    float finalAlpha = color.a * ellipseAlpha * tintColor.a * qt_Opacity;

    // Premultiplied alpha kimenet: rgb is szorozva van alpha-val
    fragColor = vec4(tintColor.rgb * finalAlpha, finalAlpha);
}
