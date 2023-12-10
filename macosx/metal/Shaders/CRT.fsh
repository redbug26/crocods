

#define TEX2D(c) pow(texture2D(mpass_texture, (c)), vec4(inputGamma))
#define FIX(c)   max(abs(c), 1e-6);
#define PI 3.141592653589

// Adjusts the vertical position of scanlines. Useful if the output
// pixel size is large compared to the scanline width (so, scale
// factors less than 4x or so). Ranges from 0.0 to 1.0.
#define phase 0.0

// Assume NTSC 2.2 Gamma for linear blending.
#define inputGamma 2.2

// Simulate a CRT gamma of 2.5.
#define outputGamma 2.5

// Controls the intensity of the barrel distortion used to emulate the curvature of a CRT. 0.0 is perfectly flat, 1.0 is annoyingly distorted, higher values are increasingly ridiculous.
#define distortion 0.0


#define RGB( r, g, b ) vec4( float( r ) / 255.0, float( g ) / 255.0, float( b ) / 255.0, 1)

//vec2 radialDistortion(vec2 coord) {
//    coord *= color_texture_pow2_sz / color_texture_sz;
//    vec2 cc = coord - 0.5;
//    float dist = dot(cc, cc) * distortion;
//    return (coord + cc * (1.0 + dist) * dist) * color_texture_sz / color_texture_pow2_sz;
//}


vec2 crt(vec2 coord, float bend)
{
    // put in symmetrical coords
    coord = (coord - 0.5) * 2.0;
    
//    coord *= 1.1;
    
    // deform coords
    coord.x *= 1.0 + pow((abs(coord.y) / bend), 2.0);
    coord.y *= 1.0 + pow((abs(coord.x) / bend), 2.0);
    
    
    // transform back to 0.0 - 1.0 space
    coord  = (coord / 2.0) + 0.5;
    
    return coord;
}

vec2 CRTCurveUV( vec2 uv )
{
    uv.y = 1 - uv.y;

    uv = uv * 2.0 - 1.0;
    vec2 offset =  abs( uv.yx ) / vec2( 6.0, 4.0 );
    uv = uv + uv * offset * offset;
    uv = uv * 0.5 + 0.5;
    
    return uv;
}

vec4 scale(sampler2D image)
{
    vec2 texCoord = vec2(gl_FragCoord.x, uResolution.y - gl_FragCoord.y) / uResolution;
    
    vec2 uv    = gl_FragCoord.xy / uResolution.xy;
    vec2 crtUV = CRTCurveUV( uv );
    
    crtUV.y = crtUV.y;
    
    return  texture(image, crtUV);
//    return  texture(image, texCoord);

}
