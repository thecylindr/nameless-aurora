#version 440
layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;
layout(std140, binding = 0) uniform buf
{
    mat4 qt_Matrix;
    float qt_Opacity;
    float phase;
    float intensity;
    float variant;
    float energy;
    float bass;
    float pulse;
    float signature;
    float shock;
    float impact;
    float deformationStrength;
    vec4 colorA;
    vec4 colorB;
    vec2 resolution;
    vec2 pointer;
} u;
void main()
{
    float size = max(1.0, min(u.resolution.x, u.resolution.y));
    vec2 p = (qt_TexCoord0 - .5) * u.resolution / size - u.pointer * .012;
    // Corners contain no visible light; skip the trigonometry and lighting there.
    float distanceSquared = dot(p, p);
    if (distanceSquared > .57 * .57) {
        fragColor = vec4(0.);
        return;
    }
    float t = u.phase, aa = 1.2 / size, sig = u.signature;
    float beat = u.pulse * clamp(u.intensity, 0., 1.5), bass = u.bass * clamp(u.intensity, 0., 1.5);
    float angle = atan(p.y, p.x), lobes = sig > .5 ? 4. : 3.;
    float deformation = (u.variant > 1.5 ? .014 : .006) + u.energy * .012 + beat * .008;
    float radius = .312 * (1. + bass * .055 + beat * .045);
    radius += sin(angle * lobes + t * .35 + sig * 6.28) * deformation;
    radius += sin(angle * 2. - t * .23 + sig * 9.) * u.energy * .005;
    float r = length(p);
    vec3 a = u.colorA.rgb, b = u.colorB.rgb;
    float halo = exp(-max(0., r - radius) * 17.) * exp(-max(0., radius - r) * 12.);
    float alpha = halo * (.2 + beat * .14 + bass * .08) * u.intensity;
    vec3 rgb = mix(a, b, .35 + .15 * sin(t * .2)) * alpha;
    float tilt = -.55 + sig * .4 + (.05 + u.energy * .06) * sin(t * .23);
    vec2 q = mat2(cos(tilt), -sin(tilt), sin(tilt), cos(tilt)) * p;
    float ellipse = abs(length(q / (vec2(.432, .137) * (1. + beat * .06))) - 1.);
    float rings = (1. - smoothstep(.003, .018, ellipse)) * (u.variant > .5 && u.variant < 1.5 ? .32 : .14);
    float ring2 = (1. - smoothstep(.004, .018, abs(length(q / vec2(.464, .148)) - 1.))) * .09;
    rgb += mix(a, b, .65) * (rings + ring2);
    alpha = max(alpha, rings + ring2);
    float flash = pow(max(0., 1. - u.shock), 1.4) * u.impact * u.intensity;
    float waveRadius = .32 + u.shock * .17;
    float wave = (1. - smoothstep(aa, aa * 3., abs(r - waveRadius))) * flash * .42;
    float echo = (1. - smoothstep(aa, aa * 2.5, abs(r - waveRadius - .02))) * flash * .15;
    rgb += mix(a, b, u.shock) * (wave + echo);
    alpha = max(alpha, wave + echo);
    float mask = 1. - smoothstep(radius - aa, radius + aa, r);
    if (r < radius + aa) {
        vec3 n = vec3(p / radius, sqrt(max(0., 1. - dot(p, p) / (radius * radius))));
        float flow = sin(n.x * (4. + sig * 2.) + n.y * 3. + n.z * 2.2 - t * .45 + bass * .9);
        flow += .42 * sin(n.y * 7. - n.x * 3.3 + t * .32 + sig * 5.);
        float fold = sin(n.y * (9. + sig * 4. + bass * 2.) + n.x * 2.4 + flow * 2.5 - t * .4);
        float curtain = smoothstep(-.55, .8, fold);
        float mist = .5 + .5 * sin(n.x * 3. - n.y * 4. + n.z * 3. + flow + t * .22);
        float diffuse = .25 + .75 * max(0., dot(n, normalize(vec3(-.45, -.55, .85))));
        float rim = pow(1. - n.z, 3.);
        float specular = pow(max(0., dot(n, normalize(vec3(-.38, -.42, 1.)))), 32.);
        vec3 pigment = mix(a, b, smoothstep(-1., 1., flow));
        vec3 body = mix(vec3(.02, .03, .05), pigment, .28 + .48 * mist) * diffuse;
        body += pigment * curtain * ((u.variant > 1.5 ? .3 : .18) + beat * .12);
        body += mix(a, b, .5) * rim * (.3 + flash * .22) + vec3(.82, .9, 1.) * specular * .38;
        if (u.variant > .5 && u.variant < 1.5)
            body *= .82 + .18 * curtain;
        rgb = mix(rgb, body, mask);
        alpha = mix(alpha, 1., mask);
    }
    float front = (q.y > 0. ? 1. : 0.) * rings * .4;
    rgb += mix(a, b, .3) * front;
    alpha = max(alpha, front);
    // Analytic light trails stay in one shader pass, without noise or blur buffers.
    vec2 s1 = vec2(cos(t * .12 + sig * 5.), sin(t * .12 + sig * 5.)) * .405;
    vec2 s2 = vec2(cos(-t * .09 + 3.1), sin(-t * .09 + 3.1)) * .452;
    vec2 s3 = vec2(cos(t * .07 + 5.2), sin(t * .07 + 5.2)) * .39;
    float mote = exp(-dot(p - s1, p - s1) * 140000.) + exp(-dot(p - s2, p - s2) * 110000.)
        + exp(-dot(p - s3, p - s3) * 130000.);
    float glint = mote * (.4 + flash * .55);
    rgb += mix(a, b, .5) * glint;
    alpha = max(alpha, glint);
    float edgeFade = 1. - smoothstep(.52, .57, r);
    fragColor = vec4(rgb, clamp(alpha, 0., 1.)) * u.qt_Opacity * edgeFade;
}
